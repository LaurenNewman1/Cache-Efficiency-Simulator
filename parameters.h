#pragma once
#include <vector>
#include <random>
#include <time.h>
using namespace std;

class Parameters {
    public:
        const int N = 1000;   // num files originally in origin servers
        int C;     // Mb capacity of cache
        int lambda;     // num requests made per second
        float accBand;   // Mbps
        float inBand;    // Mbps
        
        float transRate;    // Mbps

        int testIterations;

        Parameters();
};