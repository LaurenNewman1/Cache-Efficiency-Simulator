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
    int reqPerSec = poisson(gen);
    for (int sec = 0; sec < (*params)["totalTime"].asFloat(); sec++) {
        for (int req = 0; req < reqPerSec; req++) {
            Event* ev = new Event();
            ev->index = fileSelect(gen);
            ev->key = sec;
            ev->startTime = sec;
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
        (*(ev->func))(ev);
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

static void newRequestEvent(Event* ev) {
    // if in cache, retrieve file
    if (find(cache.begin(), cache.end(), ev->index) != cache.end()) {
        logger->info(getLogMessage(ev, 0));
        Event* newEv = new Event();
        newEv->index = ev->index;
        newEv->key = ev->key + files[ev->index].getSize() / (*params)["inBand"].asFloat();
        newEv->startTime = ev->startTime;
        newEv->func = fileReceivedEvent;
        event_enqueue(newEv, &eventTree);
    }
    // if not in cache, retrieve from origin to queue
    else {
        logger->info(getLogMessage(ev, 1));
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        default_random_engine gen(seed);
        Json::Value prop = (*params)["prop"];
        lognormal_distribution<float> propTime(prop["mean"].asFloat(), prop["sd"].asFloat()); // mean, SD  in nanoseconds
        Event* newEv = new Event();
        newEv->index = ev->index;
        newEv->key = ev->key + propTime(gen);
        newEv->startTime = ev->startTime;
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

static void fileReceivedEvent(Event* ev) {
    logger->info(getLogMessage(ev, 2));
    responses.insert(std::pair<int, float>(ev->index, currTime - ev->startTime));
};

static void arriveAtQueueEvent(Event* ev) {
    // if nothing in queue, depart
    if (q.empty()) {
        q.push(ev);
        logger->info(getLogMessage(ev, 3));
        Event* newEv = new Event();
        newEv->index = ev->index;
        newEv->key = ev->key + files[ev->index].getSize() / (*params)["accBand"].asFloat();
        newEv->startTime = ev->startTime;
        newEv->func = departQueueEvent;
        event_enqueue(newEv, &eventTree);
    }
    // else, add to queue
    else {
        q.push(ev);
        logger->info(getLogMessage(ev, 4));
    }
};

static void departQueueEvent(Event* ev) {
    q.pop();
    // make room for new file in cache
    while (cacheContents + files[ev->index].getSize() > (*params)["C"].asFloat()) {
        switch (algorithm) {
            case oldestfirst: {
                oldestFirst();
            }
        }
        logger->info(getLogMessage(ev, 5));
    }
    // add to cache
    cache.push_back(ev->index);
    cacheContents += files[ev->index].getSize();
    logger->info(getLogMessage(ev, 6));
    // send to user
    Event* newEv = new Event();
    newEv->index = ev->index;
    newEv->key = ev->key + files[ev->index].getSize() / (*params)["inBand"].asFloat();
    newEv->startTime = ev->startTime;
    newEv->func = fileReceivedEvent;
    event_enqueue(newEv, &eventTree);
    // depart the next item from queue
    if (!q.empty()) {
        Event* front = q.front();
        Event* newEv = new Event();
        newEv->index = front->index;
        newEv->key = front->key + files[front->index].getSize() / (*params)["accBand"].asFloat();
        newEv->startTime = ev->startTime;
        newEv->func = departQueueEvent;
        event_enqueue(newEv, &eventTree);
    }
};

static void oldestFirst() {
    cacheContents -= files[cache.front()].getSize();
    cache.erase(cache.begin());
}

static string getLogMessage(Event* ev, int type) {
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