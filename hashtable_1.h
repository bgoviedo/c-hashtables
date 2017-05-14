#ifndef HASHTABLE_H_1
#define HASHTABLE_H_1

#include "Utils/packetsource.h"
#include <stdbool.h>
typedef struct hashtable_1 hashtable_1;

struct hashtable_1 {
  bool (*add)(hashtable_1*, int, Packet_t*);
  bool (*remove)(hashtable_1*, int);
  bool (*contains)(hashtable_1*, int);
  volatile int capacity;
};

typedef enum ht_type { LOCKING, LINEARPROBE, LOCKFREE } ht_type;

hashtable_1* new_hashtable_1(ht_type type, int capacity);

#endif /* HASHTABLE_H_1 */
