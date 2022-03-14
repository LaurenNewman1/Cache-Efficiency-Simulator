#include "file.h"
#include <queue>
#include <random>
#include <vector>
#include <time.h>
#include "json/json.h"
using namespace std;

class OldestFirst {
    File* files;
    Json::Value* params;
    vector<int> origin;     // list of i at the origin
    vector<int> cache;      // list of i in the cache
    queue<int> queue;       // list of i waiting at access queue

    vector<float> responseTimes;

    void newRequestEvent(int index, lognormal_distribution<float>*);
    void fileReceivedEvent(int index);
    void arriveAtQueueEvent(int index);
    void departQueueEvent(int index);

    public:
        OldestFirst();
        OldestFirst(File* files, Json::Value* params);
        void simulate();
};