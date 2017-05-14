#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../Utils/packetsource.h"
#include "../hashtable_1.h"
#include "probetable.h"

#define DISTANCE_THRESHOLD 4

int pt_lockless_add(bucket_element* elements, int capacity, int key,
                    Packet_t* p) {
  const int i_start = key & (capacity - 1);
  bucket_element* e_start = elements + i_start;
  for (int num_steps = 0; num_steps <= DISTANCE_THRESHOLD; ++num_steps) {
    const int i_next = (i_start + num_steps) & (capacity - 1);
    bucket_element* e_current = elements + i_next;
    if (!e_current->packet) {
      elements->packet = p;
      elements->key = key;
      if (num_steps > e_start->max_dist) e_start->max_dist = num_steps;
      return 0;
    }
    if (e_current->key == key) return 2;
  }
  return 1;
}
void pt_increase_size(probetable* h) {
  const int old_size = h->capacity;
  pthread_rwlock_wrlock(&h->whole_lock);

  if (h->capacity != old_size) {
    pthread_rwlock_unlock(&h->whole_lock);
    return;
  }

  int new_capacity = h->capacity * 2;
  bucket_element* new_elements;
  bool is_successful = false;
  while (!is_successful) {
    is_successful = true;
    new_elements = calloc(new_capacity, sizeof(bucket_element));
    const int capacity = h->capacity;
    for (int i = 0; i < capacity; ++i) {
      bucket_element* e_current = h->elements + i;
      if (!e_current->packet) continue;

      const int result = pt_lockless_add(new_elements, new_capacity,
                                         e_current->key, e_current->packet);
      if (result == 2) {
        printf("Error (pt_increase_size): Duplicate encountered\n");
        exit(-1);
      }

      if (result == 1) {
        is_successful = false;
        new_capacity *= 2;
        free(new_elements);
        break;
      }
    }
  }

  for (int i = 0; i < new_capacity; ++i) {
    pthread_mutex_init(&new_elements[i].datalock, NULL);
    pthread_mutex_init(&new_elements[i].distlock, NULL);
  }
  for (int i = 0; i < h->capacity; ++i) {
    pthread_mutex_destroy(&h->elements[i].datalock);
    pthread_mutex_destroy(&h->elements[i].distlock);
  }
  free(h->elements);
  h->elements = new_elements;
  h->capacity = new_capacity;
  pthread_rwlock_unlock(&h->whole_lock);
  return;
}

bool probetable_add(hashtable_1* _h, int key, Packet_t* p) {
  probetable* h = (probetable*)_h;
  pthread_rwlock_rdlock(&h->whole_lock);
  const int i_start = key & (h->capacity - 1);
  bucket_element* e_start = h->elements + i_start;
  pthread_mutex_lock(&e_start->distlock);

  for (int num_steps = 0; num_steps <= DISTANCE_THRESHOLD; ++num_steps) {
    const int i_next = (i_start + num_steps) & (h->capacity - 1);
    bucket_element* e_current = h->elements + i_next;

    pthread_mutex_lock(&e_current->datalock);
    if (!e_current->packet) {
      e_current->packet = p;
      e_current->key = key;
      pthread_mutex_unlock(&e_current->datalock);
      if (e_start->max_dist < num_steps) e_start->max_dist = num_steps;
      pthread_mutex_unlock(&e_start->distlock);
      pthread_rwlock_unlock(&h->whole_lock);
      return true;
    }
    if (e_current->key == key) {
      pthread_mutex_unlock(&e_current->datalock);
      pthread_mutex_unlock(&e_start->distlock);
      pthread_rwlock_unlock(&h->whole_lock);
      return false;
    }
    pthread_mutex_unlock(&e_current->datalock);
  }

  pthread_mutex_unlock(&e_start->distlock);
  pthread_rwlock_unlock(&h->whole_lock);
  pt_increase_size(h);
  return probetable_add(_h, key, p);
}

bool probetable_remove(hashtable_1* _h, int key) {
  probetable* h = (probetable*)_h;
  pthread_rwlock_rdlock(&h->whole_lock);
  const int i_start = key & (h->capacity - 1);
  bucket_element* e_start = h->elements + i_start;
  for (int num_steps = 0; num_steps <= e_start->max_dist; ++num_steps) {
    const int i_next = (i_start + num_steps) & (h->capacity - 1);
    bucket_element* e_current = h->elements + i_next;
    pthread_mutex_lock(&e_current->datalock);
    if (e_current->key == key) {
      e_current->packet = NULL;
      e_current->key = -1;
      pthread_mutex_unlock(&e_current->datalock);
      pthread_rwlock_unlock(&h->whole_lock);
      return true;
    }
    pthread_mutex_unlock(&e_current->datalock);
  }
  pthread_rwlock_unlock(&h->whole_lock);
  return false;
}

bool probetable_contains(hashtable_1* _h, int key) {
  probetable* h = (probetable*)_h;
  pthread_rwlock_rdlock(&h->whole_lock);
  const int i_start = key & (h->capacity - 1);
  bucket_element* e_start = h->elements + i_start;
  for (int num_steps = 0; num_steps <= e_start->max_dist; ++num_steps) {
    const int i_next = (i_start + num_steps) & (h->capacity - 1);
    bucket_element* e_current = h->elements + i_next;
    if (e_current->key == key) {
      pthread_rwlock_unlock(&h->whole_lock);
      return true;
    }
  }
  pthread_rwlock_unlock(&h->whole_lock);
  return false;
}
probetable* new_probetable(int capacity) {
  probetable* newtable = (probetable*)malloc(sizeof(probetable));
  newtable->capacity = capacity;
  newtable->elements = calloc(capacity, sizeof(bucket_element));
  for (int i = 0; i < capacity; ++i) {
    pthread_mutex_init(&newtable->elements[i].datalock, NULL);
    pthread_mutex_init(&newtable->elements[i].distlock, NULL);
  }
  pthread_rwlock_init(&newtable->whole_lock, NULL);
  newtable->add = probetable_add;
  newtable->remove = probetable_remove;
  newtable->contains = probetable_contains;
  return newtable;
}
