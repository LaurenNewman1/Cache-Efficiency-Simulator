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
#include "request.h"
#include "json/json.h"
using namespace std;

OldestFirst::OldestFirst(File* files, Json::Value* params) {
    this->params = params;
    this->files = files;
    this->cacheContents = 0;
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
    bool fileReceived = false;
    while (!fileReceived) {
        // if in cache, retrieve file
        if (find(cache.begin(), cache.end(), req->index) != cache.end()) {
            req->responseTime += files[req->index].getSize() / (*params)["inBand"].asFloat();
            fileReceivedEvent(req);
            fileReceived = true;
        }
        // if not in cache, retrieve from origin to queue
        else {
            unsigned seed = chrono::system_clock::now().time_since_epoch().count();
            default_random_engine gen(seed);
            Json::Value prop = (*params)["prop"];
            lognormal_distribution<float> propTime(prop["mean"].asFloat(), prop["sd"].asFloat()); // mean, SD  in nanoseconds
            req->responseTime += propTime(gen);
            arriveAtQueueEvent(req);
            exponential_distribution<float> X((*params)["lambda"].asFloat());
            req->responseTime += X(gen);
        }
    }
};

void OldestFirst::fileReceivedEvent(Request* req) {
    // response times have already been recorded
    return; 
};

void OldestFirst::arriveAtQueueEvent(Request* req) {
    // if nothing in queue, depart
    if (queue.empty()) {
        req->responseTime += files[req->index].getSize() / (*params)["accBand"].asFloat();
        departQueueEvent(req); 
    }
    // else, add to queue
    else {
        queue.push(req);
    }
};

void OldestFirst::departQueueEvent(Request* req) {
    // make room for new file in cache
    while (cacheContents + files[req->index].getSize() > (*params)["C"].asFloat()) {
        cacheContents -= files[cache.front()].getSize();
        cache.erase(cache.begin());
    }
    // add to cache
    cache.push_back(req->index);
    cacheContents += files[req->index].getSize();
    // send to user
    req->responseTime += files[req->index].getSize() / (*params)["inBand"].asFloat();
    fileReceivedEvent(req);
    // depart the next item from queue
    if (!queue.empty()) {
        Request* front = queue.front();
        Request* next = *find_if(requests.begin(), requests.end(), [front](Request* r){ return r->id == front->id; });
        next->responseTime += files[next->index].getSize() / (*params)["accBand"].asFloat();
        departQueueEvent(next);
    }
};