#pragma once
#include "splay/splay.h"

typedef struct Event_Struct {
    struct Event_Struct *next;          /* link events into list */
    struct Event_Struct *left;          /* Splay tree internals */
    struct Event_Struct *right;         /* Splay tree internals */
    struct Event_Struct *parent;        /* Splay tree internals */
    double              key;            /* timer value    */

    int index; // file
    void (*func)(struct Event_Struct *); 
    float startTime;
    float responseTime;
} Event;