#include <iostream>
#include <vector>
#include <math.h>
#include "file.h"
using namespace std;

void initializeFiles(File* files, int N);
void printFiles(File* files, int N);
float normalize(int x);

int main (int argc, char *argv[]) {
    const int N = 1000;   // num files originally in origin servers
    int lambda;     // num requests made per second

    vector<int> origin;     // list of i at the origin
    vector<int> cache;      // list of i in the cache

    File* files = new File[N];            // list of files

    initializeFiles(files, N);

    printFiles(files, N);

    return 0;
}

void initializeFiles(File* files, int N) {
    int mean = 1; //MB
    int alphaS = - (1 - mean) / mean; // 0
    int alphaP = 0.1;
    float sumq = 0.0f;
    // Initialize sizes
    for (int i = 0; i < N; i++) {
        float s = 1.0 / pow(normalize(i), alphaS + 1);
        files[i].setSize(s);
    }
    // Initialize popularities
    for (int i = 0; i < N; i++) {
        float p = (1.0 / pow(normalize(i), alphaP + 1));
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