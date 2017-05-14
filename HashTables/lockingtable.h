#ifndef LOCKINGTABLE_H
#define LOCKINGTABLE_H

#include <pthread.h>

#include "../Utils/packetsource.h"
#include "../hashtable_1.h"
#include "../Helpers/slists.h"

typedef struct lockingtable {
  bool (*add)(hashtable_1*, int, Packet_t*);
  bool (*remove)(hashtable_1*, int);
  bool (*contains)(hashtable_1*, int);
  volatile int capacity;

  int initial_capacity;
  pthread_rwlock_t* locks;
  slist* buckets;
} lockingtable;

lockingtable* new_lockingtable(int capacity);

#endif /* LOCKINGTABLE_H */
