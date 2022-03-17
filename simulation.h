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
static queue<Request*> q;       // list of i waiting at access queue
static CPlusPlusLogging::Logger* logger;

static Event_Struct *eventTree;

static std::multimap<int, float> responses;

static void initialize(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string a);

static void newRequestEvent(Request* r);
static void fileReceivedEvent(Request* r);
static void arriveAtQueueEvent(Request* r);
static void departQueueEvent(Request* r);

static string getLogMessage(Request* ev, int type);

void simulate(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger, string algorithm);

static void oldestFirst();