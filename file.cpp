#include "file.h"

File::File() {};

void File::setSize(float s) {
    this->size = s;
};

void File::setPopularity(float p) {
    this->popularity = p;
};

float File::getSize() {
    return this->size;
};

float File::getPopularity() {
    return this->popularity;
};