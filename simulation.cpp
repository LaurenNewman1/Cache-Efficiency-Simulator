#include "file.h"
#include "simulation.h"
#include <random>
#include <time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <chrono>
#include <array>
#include <numeric>
#include <string>
#include "json/json.h"
#include "logger/Logger.h"
#include "event.h"
#include "event.cpp"
#include "request.h"
using namespace std;

float simulate(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string algorithm) {

    initialize(files, params, logger, algorithm);

    // Generate distributions
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine gen(seed);
    poisson_distribution<int> poisson((*params)["lambda"].asInt()); 

    // Use weighted probabilities from Pareto
    vector<float> probs;
    for (int i = 0; i < (*params)["N"].asInt(); i++) {
        probs.push_back(files[i].getPopularity());
    }
    discrete_distribution<int> fileSelect(probs.begin(), probs.end());

    // Generate request events
    int idGen = 0;
    int reqPerSec = poisson(gen);
    for (int sec = 0; sec < (*params)["totalTime"].asFloat(); sec++) {
        for (int req = 0; req < reqPerSec; req++) {
            Request* request = new Request();
            request->id = idGen++;
            request->index = fileSelect(gen);
            request->startTime = float(sec) + req / float(reqPerSec);;
            Event* ev = new Event();
            ev->req = request;
            ev->key = float(sec) + req / float(reqPerSec);;
            ev->func = newRequestEvent;
            event_enqueue(ev, &eventTree);
        }
    }

    // Main event loop
    while (currTime < (*params)["totalTime"].asFloat()) {
        Event* ev = event_dequeue(&eventTree);
        if (ev == NULL) {
            logger->info("Queue empty! Simulation ended.");
            break;
        }
        currTime = ev->key;
        (*(ev->func))(ev->req);
    }

    logger->info("Reached the end of totalTime parameter. Simulation ended.");

    stringstream log;
    log << "results..." << endl;
    for (auto const &pair: responses) {
        log << "{" << pair.first << ": " << pair.second << "}\n";
    }
    logger->info(log.str());

    return getAvgResponseTime();
};

void initialize(File* f, Json::Value* p, CPlusPlusLogging::Logger* l, string a) {
    files = f;
    params = p;
    logger = l;
    currTime = 0.0;
    if (a == "oldestfirst") {
        algorithm = oldestfirst;
    }
    else if (a == "largestfirst") {
        algorithm = largestfirst;
    }
    else if (a == "leastrecent") {
        algorithm = leastrecent;
    }
    event_queue_init(&eventTree);
}

static void newRequestEvent(Request* r) {
    // if in cach, retrieve file
    vector<int>::iterator found = find(cach.begin(), cach.end(), r->index);
    if (found != cach.end()) {
        logger->info(getLogMessage(r, 0));
        // if least recent, move recent request to the back
        if (algorithm == leastrecent) {
            cach.erase(cach.begin() + distance(cach.begin(), found));
            cach.push_back(r->index);
        }
        Event* newEv = new Event();
        newEv->req = r;
        newEv->key = currTime + files[r->index].getSize() / (*params)["inBand"].asFloat();
        newEv->func = fileReceivedEvent;
        event_enqueue(newEv, &eventTree);
    }
    // if not in cach, retrieve from origin to queue
    else {
        logger->info(getLogMessage(r, 1));
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        default_random_engine gen(seed);
        Json::Value prop = (*params)["prop"];
        lognormal_distribution<float> propTime(prop["mean"].asFloat(), prop["sd"].asFloat()); // mean, SD  in nanoseconds
        Event* newEv = new Event();
        newEv->req = r;
        newEv->key = currTime + propTime(gen);
        newEv->func = arriveAtQueueEvent;
        event_enqueue(newEv, &eventTree);

        // exponential_distribution<float> X((*params)["lambda"].asFloat());
        // Event* newEv2 = new Event();
        // newEv2->index = ev->index;
        // newEv2->key = ev->key + X(gen);
        // newEv2->startTime = ev->startTime;
        // newEv2->func = newRequestEvent;
        // event_enqueue(newEv2, &eventTree);
    }
};

static void fileReceivedEvent(Request* r) {
    logger->info(getLogMessage(r, 2));
    responses.insert(std::pair<int, float>(r->index, currTime - r->startTime));
};

static void arriveAtQueueEvent(Request* r) {
    // if nothing in queue, depart
    if (q.empty()) {
        q.push(r);
        logger->info(getLogMessage(r, 3));
        Event* newEv = new Event();
        newEv->req = r;
        newEv->key = currTime + files[r->index].getSize() / (*params)["accBand"].asFloat();
        newEv->func = departQueueEvent;
        event_enqueue(newEv, &eventTree);
    }
    // else, add to queue
    else {
        q.push(r);
        logger->info(getLogMessage(r, 4));
    }
};

static void departQueueEvent(Request* r) {
    Request* front = q.front();
    q.pop();
    // make room for new file in cach
    while (getCacheSize() + files[front->index].getSize() > (*params)["C"].asFloat()) {
        logger->info(getLogMessage(front, 5));
        if (algorithm == oldestfirst) {
            oldestFirst();
        }
        else if (algorithm == largestfirst) {
            largestFirst();
        }
        else if (algorithm == leastrecent) {
            leastRecent();
        }
    }
    // add to cache
    cach.push_back(front->index);
    logger->info(getLogMessage(front, 6));
    // send to user
    Event* newEv = new Event();
    newEv->req = r;
    newEv->key = currTime + files[front->index].getSize() / (*params)["inBand"].asFloat();
    newEv->func = fileReceivedEvent;
    event_enqueue(newEv, &eventTree);
    // depart the next item from queue
    if (!q.empty()) {
        Request* front = q.front();
        Event* newEv = new Event();
        newEv->req = front;
        newEv->key = currTime;
        newEv->func = departQueueEvent;
        event_enqueue(newEv, &eventTree);
    }
};

void oldestFirst() {
    cach.erase(cach.begin());
};

void largestFirst() {
    int min = *min_element(cach.begin(), cach.end()); // smaller the index, larger the file
    vector<int>::iterator it = std::find(cach.begin(), cach.end(), min);
    cach.erase(cach.begin() + distance(cach.begin(), it));
};

void leastRecent() {
    cach.erase(cach.begin());
};

float getCacheSize() {
    float cacheContents = 0.0;
    for (int i = 0; i < cach.size(); i++) {
        cacheContents += files[cach.at(i)].getSize();
    }
    return cacheContents;
}

string getLogMessage(Request* ev, int type) {
    stringstream log;
    switch (type) {
        case 0:
            log << "New Request Event: " << "\n\tindex: " << ev->index
                << "\n\tresult: " << "found in cache" << "\n\ttime: " << currTime << endl;
            break;
        case 1:
            log << "New Request Event: " << "\n\tindex: " << ev->index
                << "\n\tresult: " << "not found in cache" << "\n\ttime: " << currTime << endl;
            break;
        case 2:
            log << "File Received Event: " << "\n\tindex: " << ev->index
                << "\n\tresult: " << "file received" << "\n\ttime: " << currTime << endl;
            break;
        case 3:
            log << "Arrived at Queue Event: " << "\n\tindex: " << ev->index
                << "\n\tresult: " << "nothing in queue" << "\n\ttime: " << currTime << endl;
            break;
        case 4:
            log << "Arrived at Queue Event: " << "\n\tindex: " << ev->index
                << "\n\tresult: " << "added to queue" << "\n\ttime: " << currTime << endl;
            break;
        case 5:
            log << "Depart Queue Event: " << "\n\tindex: " << ev->index
                << "\n\tresult: " << "making room in cache" << "\n\ttime: " << currTime << endl;
            break;
        case 6:
            log << "Depart Queue Event: " << "\n\tindex: " << ev->index
                << "\n\tresult: " << "space available; adding file to cache" << "\n\ttime: " << currTime << endl;
            break;
    }
    return log.str();
}

float getAvgResponseTime() {
    vector<float> times;
    for (auto const &pair: responses) {
        times.push_back(pair.second);
    }
    return accumulate(times.begin(), times.end(), 0.0) / times.size();
}