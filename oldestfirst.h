#include "file.h"
#include "parameters.h"
#include <queue>
#include <vector>
using namespace std;

class OldestFirst {
    File* files;
    Parameters* params;
    vector<int> origin;     // list of i at the origin
    vector<int> cache;      // list of i in the cache
    queue<int> queue;       // list of i waiting at access queue

    vector<float> responseTimes;

    public:
        OldestFirst(File* files);
        void simulate();
        void newRequestEvent(int index);
};