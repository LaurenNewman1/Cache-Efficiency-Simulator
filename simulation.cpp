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
#include <string>
#include "json/json.h"
#include "logger/Logger.h"
#include "event.h"
#include "event.cpp"
#include "request.h"
using namespace std;

void simulate(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string algorithm) {

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
            request->startTime = float(sec);
            Event* ev = new Event();
            ev->req = request;
            ev->key = float(sec);
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
};

static void initialize(File* f, Json::Value* p, CPlusPlusLogging::Logger* l, string a) {
    files = f;
    params = p;
    logger = l;
    currTime = 0.0;
    cacheContents = 0.0;
    if (a == "oldestfirst") {
        algorithm = oldestfirst;
    }
    event_queue_init(&eventTree);
}

static void newRequestEvent(Request* r) {
    // if in cache, retrieve file
    if (find(cache.begin(), cache.end(), r->index) != cache.end()) {
        logger->info(getLogMessage(r, 0));
        Event* newEv = new Event();
        newEv->req = r;
        newEv->key = currTime + files[r->index].getSize() / (*params)["inBand"].asFloat();
        newEv->func = fileReceivedEvent;
        event_enqueue(newEv, &eventTree);
    }
    // if not in cache, retrieve from origin to queue
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
    // make room for new file in cache
    while (cacheContents + files[front->index].getSize() > (*params)["C"].asFloat()) {
        switch (algorithm) {
            case oldestfirst: {
                oldestFirst();
            }
        }
        logger->info(getLogMessage(front, 5));
    }
    // add to cache
    cache.push_back(front->index);
    cacheContents += files[front->index].getSize();
    logger->info(getLogMessage(front, 6));
    // send to user
    Event* newEv = new Event();
    newEv->req = r;
    newEv->key = currTime + files[front->index].getSize() / (*params)["inBand"].asFloat();
    newEv->func = fileReceivedEvent;
    event_enqueue(newEv, &eventTree);
    // depart the next item from queue
    // error here
    if (!q.empty()) {
        Request* front = q.front();
        Event* newEv = new Event();
        newEv->req = front;
        newEv->key = currTime;
        newEv->func = departQueueEvent;
        event_enqueue(newEv, &eventTree);
    }
};

static void oldestFirst() {
    cacheContents -= files[cache.front()].getSize();
    cache.erase(cache.begin());
}

static string getLogMessage(Request* ev, int type) {
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