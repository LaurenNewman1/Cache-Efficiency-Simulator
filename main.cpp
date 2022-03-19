#include <iostream>
#include <vector>
#include <queue>
#include <math.h>
#include <random>
#include <time.h>
#include <fstream>
#include <string>
#include "file.h"
#include "simulation.h"
#include "json/json.h"
#include "logger/Logger.h"
using namespace std;

void initializeFiles(File* files, int N);
string printFiles(File* files, int N);
float normalize(int x);
Json::Value* readParameters(string filename);

int main (int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Error: must provide json file with parameters and simulation type";
    }

    CPlusPlusLogging::Logger* logger = NULL; // Create the object pointer for Logger Class
    logger = CPlusPlusLogging::Logger::getInstance();

    ifstream params_file(argv[1]);
    stringstream buffer;
    buffer << params_file.rdbuf();

    Json::Reader reader;
    Json::Value params;
    bool parseSuccess = reader.parse(buffer.str(), params, false);
    if (!parseSuccess)
    {
        logger->error("Failed to read parameter file");
    }
    else {
        logger->info(buffer.str());
    }

    File* files = new File[params["N"].asInt()];            // list of files

    initializeFiles(files, params["N"].asInt());

    logger->info("Files initialized according to Pareto distribution");
    logger->info(printFiles(files, params["N"].asInt()));

    if (string(argv[2]) == "oldestfirst") {
        logger->info("Starting simulation: oldest first...");
        simulate(files, &params, logger, "oldestfirst");
    }
    else if (string(argv[2]) == "largestfirst") {
        logger->info("Starting simulation: largest first...");
        simulate(files, &params, logger, "largestfirst");
    }
    else if (string(argv[2]) == "leastrecent") {
        logger->info("Starting simulation: least recent...");
        simulate(files, &params, logger, "leastrecent");
    }
    else {
        cout << "Error: Invalid replacement algorith. Try \"oldestfirst\"" << endl;
    }

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

string printFiles(File* files, int N) {
    stringstream log;
    for (int i = 0; i < N; i++) {
        log << "File " << i << ": size = " << files[i].getSize() 
            << ", popularity = " << files[i].getPopularity() << endl;
    }
    return log.str();
}

float normalize(int x) {
    return (x + 10) * 0.1;
};