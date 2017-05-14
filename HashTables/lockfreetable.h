#ifndef LOCKFREE_H
#define LOCKFREE_H

#include <pthread.h>
#include <stdbool.h>
#include "../Utils/packetsource.h"
#include "../hashtable_1.h"
#include "../Helpers/lf_lists.h"

typedef struct lockfreetable {
  bool (*add)(hashtable_1*, int, Packet_t*);
  bool (*remove)(hashtable_1*, int);
  bool (*contains)(hashtable_1*, int);
  volatile int capacity;

  pthread_mutex_t* locks;
  lf_list* buckets;
  volatile int* num_contain_accesses;
  volatile bool is_resize_needed;
  int initial_capacity;
} lftable;

lftable* new_lockfreetable(int capacity);

#endif /* LOCKFREE_H */