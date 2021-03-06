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

void initializeFiles(File* files, Json::Value* params);
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

    initializeFiles(files, &params);

    logger->info("Files initialized according to Pareto distribution");
    logger->info(printFiles(files, params["N"].asInt()));

    cout << "Starting simulation. See out/log.log for a complete output." << endl;

    if (string(argv[2]) == "oldestfirst") {
        logger->info("Starting simulation: oldest first...");
        float avg = simulate(files, &params, logger, "oldestfirst");
        stringstream log;
        log << "Average response time: " << avg << " seconds" << endl;
        logger->info(log.str());
    }
    else if (string(argv[2]) == "largestfirst") {
        logger->info("Starting simulation: largest first...");
        float avg = simulate(files, &params, logger, "largestfirst");
        stringstream log;
        log << "Average response time: " << avg << " seconds" << endl;
        logger->info(log.str());
    }
    else if (string(argv[2]) == "leastrecent") {
        logger->info("Starting simulation: least recent...");
        float avg = simulate(files, &params, logger, "leastrecent");
        stringstream log;
        log << "Average response time: " << avg << " seconds" << endl;
        logger->info(log.str());
    }
    else if (string(argv[2]) == "all") {
        logger->info("Starting simulation (1/3): oldest first...");
        float avgOld = simulate(files, &params, logger, "oldestfirst");
        logger->info("Starting simulation (2/3): largest first...");
        float avgLarge = simulate(files, &params, logger, "largestfirst");
        logger->info("Starting simulation (3/3): least recent...");
        float avgLeast = simulate(files, &params, logger, "leastrecent");
        stringstream log;
        log << "Statistics\n"
            << "Oldest First: " << "\n\tAverage response time: " << avgOld << " seconds\n"
            << "Largest First: " << "\n\tAverage response time: " << avgLarge << " seconds\n"
            << "Least Recent: " << "\n\tAverage response time: " << avgLeast << " seconds\n";
        logger->info(log.str());
    }
    else {
        cout << "Error: Invalid replacement algorith. Try \"oldestfirst\"" << endl;
    }

    cout << "Simulation completed successfully." << endl;

    return 0;
}

void initializeFiles(File* files, Json::Value* params) {
    Json::Value pareto = (*params)["pareto"];
    int N = (*params)["N"].asInt();
    float alphaS = pareto["meanS"].asFloat();
    float alphaP = pareto["meanP"].asFloat();
    float modeS = pareto["modeS"].asFloat();
    float modeP = pareto["modeP"].asFloat();
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