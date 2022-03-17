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

enum Algorithm { oldestfirst };

static float currTime;
static int algorithm;
static File* files;
static Json::Value* params;
static vector<int> cache;      // list of i in the cache
static float cacheContents;
static queue<Event*> q;       // list of i waiting at access queue
static CPlusPlusLogging::Logger* logger;

static Event_Struct *eventTree;

static std::multimap<int, float> responses;

static void initialize(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string a);

static void newRequestEvent(Event* ev);
static void fileReceivedEvent(Event* ev);
static void arriveAtQueueEvent(Event* ev);
static void departQueueEvent(Event* ev);

static string getLogMessage(Event* ev, int type);

void simulate(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string algorithm);

static void oldestFirst();