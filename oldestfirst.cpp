#include "file.h"
#include "oldestfirst.h"
#include "parameters.h"
#include <time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <chrono>
#include <array>
using namespace std;

OldestFirst::OldestFirst(File* files) {
    this->params = new Parameters();
    this->files = files;
}

void OldestFirst::simulate() {

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine gen(seed);
    lognormal_distribution<float> propTime(0.0, 1.0); // mean, SD  in nanoseconds
    poisson_distribution<int> poisson(params->lambda); 

    array<float, 1000> probs;
    for (int i = 0; i < params->N; i++) {
        probs.at(i) = files[i].getPopularity();
    }
    discrete_distribution<int> fileSelect(probs.begin(), probs.end());

    for (int iter = 0; iter < params->testIterations; iter++) {
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