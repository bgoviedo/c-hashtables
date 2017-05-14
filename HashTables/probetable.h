#ifndef LINEARPROBE_H
#define LINEARPROBE_H

#include <pthread.h>

#include "../Utils/packetsource.h"
#include "../hashtable_1.h"

typedef struct bucket_element {
    Packet_t* packet;
    int key;
    volatile int max_dist;
  // Protects key and packet
    pthread_mutex_t datalock;
  // protects max_dist
    pthread_mutex_t distlock;
} bucket_element;

typedef struct probetable {
  bool (*add)(hashtable_1*, int, Packet_t*);
  bool (*remove)(hashtable_1*, int);
  bool (*contains)(hashtable_1*, int);
  volatile int capacity;

  pthread_rwlock_t whole_lock;
  bucket_element* elements;
} probetable;

probetable* new_probetable(int capacity);

#endif /* LINEARPROBE_H */
