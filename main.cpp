#include <iostream>
#include <vector>
#include <queue>
#include <math.h>
#include <random>
#include <time.h>
#include "file.h"
using namespace std;

void initializeFiles(File* files, int N);
void printFiles(File* files, int N);
float normalize(int x);

int main (int argc, char *argv[]) {
    const int N = 1000;   // num files originally in origin servers
    int lambda;     // num requests made per second
    float accBand = 15.0f;   // Mbps
    float inBand = 100.0f;    // Mbps
    
    mt19937 rand(time(NULL));
    lognormal_distribution<float> propTime(0.0, 1.0); // mean, SD  in nanoseconds
    float transRate = 20.0f;    // Mbps

    vector<int> origin;     // list of i at the origin
    vector<int> cache;      // list of i in the cache
    queue<int> queue;       // list of i waiting at access queue

    File* files = new File[N];            // list of files

    initializeFiles(files, N);

    printFiles(files, N);

    return 0;
}

void initializeFiles(File* files, int N) {
    float alphaS = 2.0;
    float alphaP = 1.1;
    float modeS = 10.0;
    float modeP = 10.0;
    float sumq = 0.0f;
    // Initialize sizes
    for (int i = 0; i < N; i++) {
        float s = alphaS * pow(modeS, alphaS) / pow(normalize(i), alphaS + 1);
        files[i].setSize(s);
    }
    // Initialize popularities
    for (int i = 0; i < N; i++) {
        float p = alphaP * pow(modeP, alphaP) / pow(normalize(i), alphaP + 1);
        sumq += p;
        files[i].setPopularity(p);
    }
    for (int i = 0; i < N; i++) {
        files[i].setPopularity(files[i].getPopularity() / sumq);
    }
}

void printFiles(File* files, int N) {
    for (int i = 0; i < N; i++) {
        cout << "File " << i << ": " << files[i].getSize() << ", " << files[i].getPopularity() << endl;
    }
}

float normalize(int x) {
    return (x + 1) * 0.1;
};