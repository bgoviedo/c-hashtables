#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "../Utils/packetsource.h"
#include "lockingtable.h"
#include "../Helpers/slists.h"
#include "../Helpers/lf_lists.h"

#define RESIZE_THRESHOLD 4
#define PREALLOCATE_COUNT 1 << 22

void lt_increase_size(lockingtable* h) {
  const int old_size = h->capacity;
  if (old_size == PREALLOCATE_COUNT) goto unlock;

  for (int i = 0; i < h->initial_capacity; ++i)
    pthread_rwlock_wrlock(h->locks + i);

  if (h->capacity != old_size) goto unlock;

  for (int i = 0; i < h->capacity; ++i) {
    slist* bucket = h->buckets + i;
    slist_element* e_current = bucket->head;
    while (e_current) {
      const int i_new = e_current->key & (h->capacity * 2 - 1);
      if (i_new != i) {
        slist_add(h->buckets + i_new, e_current->key, e_current->packet);
        const int temp_key = e_current->key;
        e_current = e_current->next;
        slist_remove(bucket, temp_key);
      } else {
        e_current = e_current->next;
      }
    }
  }
  h->capacity *= 2;

unlock:
  for (int i = 0; i < h->initial_capacity; ++i)
    pthread_rwlock_unlock(h->locks + i);
  return;
}

bool lockingtable_add(hashtable_1* _h, int key, Packet_t* p) {
  lockingtable* h = (lockingtable*)_h;
  const int lock_index = key & (h->initial_capacity - 1);

  pthread_rwlock_wrlock(h->locks + lock_index);
  const int bucket_index = key & (h->capacity - 1);
  const bool is_successful_add = slist_add(h->buckets + bucket_index, key, p);
  const bool should_resize =
      (h->buckets[bucket_index].size > RESIZE_THRESHOLD) ? true : false;
  pthread_rwlock_unlock(h->locks + lock_index);

  if (should_resize) lt_increase_size(h);

  return is_successful_add;
}

bool lockingtable_remove(hashtable_1* _h, int key) {
  lockingtable* h = (lockingtable*)_h;
  const int lock_index = key & (h->initial_capacity - 1);

  pthread_rwlock_rdlock(h->locks + lock_index);
  const int bucket_index = key & (h->capacity - 1);
  const bool is_successful_remove =
      slist_remove(h->buckets + bucket_index, key);
  pthread_rwlock_unlock(h->locks + lock_index);

  return is_successful_remove;
}

bool lockingtable_contains(hashtable_1* _h, int key) {
  lockingtable* h = (lockingtable*)_h;
  const int lock_index = key & (h->initial_capacity - 1);

  pthread_rwlock_rdlock(h->locks + lock_index);
  const int bucket_index = key & (h->capacity - 1);
  const bool contains_key = slist_contains(h->buckets + bucket_index, key);
  pthread_rwlock_unlock(h->locks + lock_index);

  return contains_key;
}

lockingtable* new_lockingtable(int capacity) {
  lockingtable* newtable = (lockingtable*)malloc(sizeof(lockingtable));
  newtable->add = lockingtable_add;
  newtable->remove = lockingtable_remove;
  newtable->contains = lockingtable_contains;
  newtable->capacity = newtable->initial_capacity = capacity;
  newtable->locks =
      (pthread_rwlock_t*)malloc(capacity * sizeof(pthread_rwlock_t));
  newtable->buckets = new_serial_lists(PREALLOCATE_COUNT);

  for (int i = 0; i < capacity; ++i)
    pthread_rwlock_init(newtable->locks + i, NULL);

  return newtable;
}
