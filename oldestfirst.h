#include "file.h"
#include <queue>
#include <vector>
#include "json/json.h"
using namespace std;

class OldestFirst {
    File* files;
    Json::Value* params;
    vector<int> origin;     // list of i at the origin
    vector<int> cache;      // list of i in the cache
    queue<int> queue;       // list of i waiting at access queue

    vector<float> responseTimes;

    public:
        OldestFirst(File* files, Json::Value* params);
        void simulate();
        void newRequestEvent(int index);
};