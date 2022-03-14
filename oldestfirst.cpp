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
#include "json/json.h"
using namespace std;

OldestFirst::OldestFirst(File* files, Json::Value* params) {
    this->params = params;
    this->files = files;
}

void OldestFirst::simulate() {

    // Generate distributions
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine gen(seed);
    Json::Value prop = (*params)["prop"];
    lognormal_distribution<float> propTime(prop["mean"].asFloat(), prop["sd"].asFloat()); // mean, SD  in nanoseconds
    poisson_distribution<int> poisson((*params)["lambda"].asInt()); 

    // Use weighted probabilities from Pareto
    vector<float> probs;
    for (int i = 0; i < (*params)["N"].asInt(); i++) {
        probs.push_back(files[i].getPopularity());
    }
    discrete_distribution<int> fileSelect(probs.begin(), probs.end());

    // Generate request events
    for (int iter = 0; iter < (*params)["testIterations"].asInt(); iter++) {
        for (int req = 0; req < poisson(gen); req++) {
            newRequestEvent(fileSelect(gen));
        }
    }


}

void OldestFirst::newRequestEvent(int index) {
    if (find(cache.begin(), cache.end(), index) != cache.end()) {
        cout << "Element found in cache!";
    }
    else {
        cout << "Element not found";
    }
}