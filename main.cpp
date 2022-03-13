#include <iostream>
#include <vector>
#include <queue>
#include <math.h>
#include <random>
#include <time.h>
#include "file.h"
#include "oldestfirst.h"
#include "parameters.h"
using namespace std;

void initializeFiles(File* files, int N);
void printFiles(File* files, int N);
float normalize(int x);

int main (int argc, char *argv[]) {
    const int N = 1000;   // num files originally in origin servers
    File* files = new File[N];            // list of files

    initializeFiles(files, N);

    OldestFirst* sim1 = new OldestFirst(files);
    sim1->simulate();

    //printFiles(files, N);

    return 0;
}

void initializeFiles(File* files, int N) {
    float alphaS = 2.0;
    float alphaP = 1.1;
    float modeS = 5.0;
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
    return (x + 10) * 0.1;
};