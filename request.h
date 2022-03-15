#pragma once
using namespace std;

class Request {

    public:
        int id;
        int index;
        float responseTime;
        Request(int id, int i);
};