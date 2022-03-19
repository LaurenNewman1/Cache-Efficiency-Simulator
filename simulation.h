#include "file.h"
#include <queue>
#include <random>
#include <vector>
#include <time.h>
#include <map>
#include "json/json.h"
#include "logger/Logger.h"
#include "event.h"
using namespace std;

enum Algorithm { oldestfirst, largestfirst, leastrecent };

static float currTime;
static int algorithm;
static File* files;
static Json::Value* params;
static vector<int> cach;      // list of i in the cache
static queue<Request*> q;       // list of i waiting at access queue
static CPlusPlusLogging::Logger* logger;

static Event_Struct *eventTree;

static std::multimap<int, float> responses;

void initialize(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string a);

static void newRequestEvent(Request* r);
static void fileReceivedEvent(Request* r);
static void arriveAtQueueEvent(Request* r);
static void departQueueEvent(Request* r);

string getLogMessage(Request* ev, int type);

void simulate(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string algorithm);

void oldestFirst();
void largestFirst();
void leastRecent();

static float getCacheSize();
static float getAvgResponseTime();