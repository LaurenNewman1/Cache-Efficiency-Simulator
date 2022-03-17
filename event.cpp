#include "event.h"
#include "splay/splay.h"

static void event_queue_init(Event **tr_ptr) {
    init((SplayTree *) tr_ptr);
}

static void event_enqueue(Event *ev, Event **tr_ptr) {
  insert((SplayNode *)ev, (SplayTree *) tr_ptr);
}

static Event *event_dequeue(Event **tr_ptr) {
  Event *event;
  rm((SplayNode **) &event, (SplayTree *) tr_ptr);
  return(event);
}