#include "file.h"
#include "oldestfirst.h"
#include <random>
#include <time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <chrono>
#include <array>
#include <string>
#include "request.h"
#include "json/json.h"
#include "logger/Logger.h"
using namespace std;

OldestFirst::OldestFirst(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger) {
    this->currTime = 0;
    this->params = params;
    this->files = files;
    this->cacheContents = 0;
    this->logger = logger;
}

void OldestFirst::simulate() {

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
    int numReqs = 0;
    for (int iter = 0; iter < (*params)["testIterations"].asInt(); iter++) {
        for (int req = 0; req < poisson(gen); req++) {
            Request* newReq = new Request(numReqs++, fileSelect(gen));
            requests.push_back(newReq);
            newRequestEvent(newReq);
        }
    }

    for (int i = 0; i < requests.size(); i++) {
        cout << requests.at(i)->id << ": " << requests.at(i)->index << ", " << requests.at(i)->responseTime << endl;
    }
};

void OldestFirst::newRequestEvent(Request* req) {
    // if in cache, retrieve file
    if (find(cache.begin(), cache.end(), req->index) != cache.end()) {
        logger->info(getLogMessage(req, 0));
        currTime += files[req->index].getSize() / (*params)["inBand"].asFloat();
        fileReceivedEvent(req);
    }
    // if not in cache, retrieve from origin to queue
    else {
        logger->info(getLogMessage(req, 1));
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        default_random_engine gen(seed);
        Json::Value prop = (*params)["prop"];
        lognormal_distribution<float> propTime(prop["mean"].asFloat(), prop["sd"].asFloat()); // mean, SD  in nanoseconds
        currTime += propTime(gen);
        arriveAtQueueEvent(req);
        exponential_distribution<float> X((*params)["lambda"].asFloat());
        currTime += X(gen);
    }
};

void OldestFirst::fileReceivedEvent(Request* req) {
    logger->info(getLogMessage(req, 2));
    req->endTime = currTime;
    req->responseTime = currTime - req->startTime;
    return; 
};

void OldestFirst::arriveAtQueueEvent(Request* req) {
    // if nothing in queue, depart
    if (queue.empty()) {
        logger->info(getLogMessage(req, 3));
        currTime += files[req->index].getSize() / (*params)["accBand"].asFloat();
        departQueueEvent(req); 
    }
    // else, add to queue
    else {
        queue.push(req);
        logger->info(getLogMessage(req, 4));
    }
};

void OldestFirst::departQueueEvent(Request* req) {
    // make room for new file in cache
    while (cacheContents + files[req->index].getSize() > (*params)["C"].asFloat()) {
        cacheContents -= files[cache.front()].getSize();
        cache.erase(cache.begin());
        logger->info(getLogMessage(req, 5));
    }
    // add to cache
    cache.push_back(req->index);
    cacheContents += files[req->index].getSize();
    logger->info(getLogMessage(req, 6));
    // send to user
    currTime += files[req->index].getSize() / (*params)["inBand"].asFloat();
    fileReceivedEvent(req);
    // depart the next item from queue
    if (!queue.empty()) {
        Request* front = queue.front();
        Request* next = *find_if(requests.begin(), requests.end(), [front](Request* r){ return r->id == front->id; });
        currTime += files[next->index].getSize() / (*params)["accBand"].asFloat();
        departQueueEvent(next);
    }
};

string OldestFirst::getLogMessage(Request* req, int type) {
    stringstream log;
    switch (type) {
        case 0:
            log << "New Request Event: " << "\n\tid: " << req->id << "\n\tindex: " << req->index
                << "\n\tresult: " << "found in cache" << "\n\ttime: " << currTime << endl;
            break;
        case 1:
            log << "New Request Event: " << "\n\tid: " << req->id << "\n\tindex: " << req->index
                << "\n\tresult: " << "not found in cache" << "\n\ttime: " << currTime << endl;
            break;
        case 2:
            log << "File Received Event: " << "\n\tid: " << req->id << "\n\tindex: " << req->index
                << "\n\tresult: " << "file received" << "\n\ttime: " << currTime << endl;
            break;
        case 3:
            log << "Arrived at Queue Event: " << "\n\tid: " << req->id << "\n\tindex: " << req->index
                << "\n\tresult: " << "nothing in queue" << "\n\ttime: " << currTime << endl;
            break;
        case 4:
            log << "Arrived at Queue Event: " << "\n\tid: " << req->id << "\n\tindex: " << req->index
                << "\n\tresult: " << "added to queue" << "\n\ttime: " << currTime << endl;
            break;
        case 5:
            log << "Depart Queue Event: " << "\n\tid: " << req->id << "\n\tindex: " << req->index
                << "\n\tresult: " << "making room in cache" << "\n\ttime: " << currTime << endl;
            break;
        case 6:
            log << "Depart Queue Event: " << "\n\tid: " << req->id << "\n\tindex: " << req->index
                << "\n\tresult: " << "space available; adding file to cache" << "\n\ttime: " << currTime << endl;
            break;
    }
    return log.str();
}