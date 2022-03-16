#include "file.h"
#include <queue>
#include <random>
#include <vector>
#include <time.h>
#include "json/json.h"
#include "request.h"
#include "logger/Logger.h"
using namespace std;

class OldestFirst {
    float currTime;
    File* files;
    Json::Value* params;
    vector<int> origin;     // list of i at the origin
    vector<int> cache;      // list of i in the cache
    float cacheContents;
    queue<Request*> queue;       // list of i waiting at access queue
    CPlusPlusLogging::Logger* logger;

    vector<Request*> requests;

    void newRequestEvent(Request* req);
    void fileReceivedEvent(Request* req);
    void arriveAtQueueEvent(Request* req);
    void departQueueEvent(Request* req);
    
    string getLogMessage(Request* req, int type);

    public:
        OldestFirst();
        OldestFirst(File* files, Json::Value* params, CPlusPlusLogging::Logger* logger);
        void simulate();
};