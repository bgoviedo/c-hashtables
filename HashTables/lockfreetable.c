
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Utils/packetsource.h"

#include "../hashtable_1.h"
#include "../Helpers/lf_lists.h"
#include "lockfreetable.h"

#define RESIZE_THRESHOLD 4
#define PREALLOCATE_COUNT 1 << 22

#define MARKOF(x) ((unsigned long)(x) & (0x1l))
#define REFOF(x) (__typeof__(x))((unsigned long)(x) & (~0x1l))

/*----------------------------------------------------------------------------*/
/*------------------------------HELPER FUNCTIONS------------------------------*/
/*----------------------------------------------------------------------------*/

void lft_increase_size(lftable *h) {
  const int old_size = h->capacity;
  if (old_size >= PREALLOCATE_COUNT) {
    printf("Exceeded max size\n");
    return;
  }

  for (int i = 0; i < h->initial_capacity; ++i)
    pthread_mutex_lock(h->locks + i);

  if (h->capacity != old_size) goto unlock;

  lf_element **elements = malloc(h->capacity * sizeof(lf_element *));
  for (int i = 0; i < h->capacity; ++i) {
    lf_list *current_bucket = h->buckets + i;
    lf_element *e_current = current_bucket->head;
    elements[i] = NULL;
    int idx = 0;
    while (e_current) {
      int i_new = e_current->key & (h->capacity * 2 - 1);
      if (i_new != i) {
        if (!MARKOF(e_current->next))
          lf_list_add(h->buckets + i_new, e_current->key, e_current->packet,
                      NULL);
        idx++;
      } else {
        elements[i] = e_current;
      }
      e_current = REFOF(e_current->next);
    }
    current_bucket->size = idx;
  }
  int *idx_new = malloc(sizeof(int));
  *idx_new = 0;
  volatile int *idx_old = h->num_contain_accesses;
  h->capacity *= 2;
  __sync_synchronize();
  h->num_contain_accesses = idx_new;

  // spin
  while (*idx_old)
    ;
  for (int i = 0; i < h->capacity / 2; ++i) {
    lf_element *e_current = elements[i];
    if (!e_current) {
      h->buckets[i].head = NULL;
      continue;
    }
    unsigned long m = MARKOF(e_current->next);
    e_current->next = (lf_element *)m;
  }
  free(elements);

unlock:
  for (int i = 0; i < h->initial_capacity; ++i)
    pthread_mutex_unlock(h->locks + i);
  return;
}
/*----------------------------------------------------------------------------*/
/*----------------------------TABLE IMPLEMENTATION----------------------------*/
/*----------------------------------------------------------------------------*/

bool lftable_add(hashtable_1 *_h, int key, Packet_t *p) {
  lftable *h = (lftable *)_h;
  const int i_lock = key & (h->initial_capacity - 1);
  bool should_resize = false;

  pthread_mutex_lock(h->locks + i_lock);
  const int i_bucket = key & (h->capacity - 1);
  const bool is_successful_add =
      lf_list_add(h->buckets + i_bucket, key, p, NULL);
  if (h->buckets[i_bucket].size > RESIZE_THRESHOLD) should_resize = true;
  pthread_mutex_unlock(h->locks + i_lock);

  if (should_resize) lft_increase_size(h);

  return is_successful_add;
}

bool lftable_remove(hashtable_1 *_h, int key) {
  lftable *h = (lftable *)_h;
  const int i_lock = key & (h->initial_capacity - 1);

  pthread_mutex_lock(h->locks + i_lock);
  const int i_bucket = key & (h->capacity - 1);
  const bool is_successful_remove = lf_list_remove(h->buckets + i_bucket, key);
  pthread_mutex_unlock(h->locks + i_lock);

  return is_successful_remove;
}

bool lftable_contains(hashtable_1 *_h, int key) {
  lftable *h = (lftable *)_h;
  volatile int *count = h->num_contain_accesses;
  __sync_fetch_and_add(count, 1);
  const int i_bucket = key & (h->capacity - 1);
  const bool does_contain_key =
      lf_list_contains(h->buckets + i_bucket, key, NULL);
  __sync_fetch_and_add(count, -1);
  return does_contain_key;
}

lftable *new_lockfreetable(int capacity) {
  lftable *newtable = (lftable *)malloc(sizeof(lftable));
  newtable->initial_capacity = newtable->capacity = capacity;
  newtable->buckets = new_lockfree_lists(PREALLOCATE_COUNT);
  newtable->locks = malloc(capacity * sizeof(pthread_mutex_t));
  for (int i = 0; i < capacity; ++i)
    pthread_mutex_init(newtable->locks + i, NULL);
  newtable->num_contain_accesses = malloc(sizeof(int));
  *(newtable->num_contain_accesses) = 0;
  newtable->is_resize_needed = false;
  newtable->add = lftable_add;
  newtable->remove = lftable_remove;
  newtable->contains = lftable_contains;
  return newtable;
}