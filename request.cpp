#include "request.h"

Request::Request(int id, int i) {
    this->id = id;
    this->index = i;
    this->responseTime = 0.0;
}