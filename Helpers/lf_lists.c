#include <stdbool.h>
#include <stdlib.h>

#include "../Utils/packetsource.h"

#include "lf_lists.h"

#define _UNUSED_ __attribute__((unused))
#define MARKOF(x) ((unsigned long)(x) & (0x1l))
#define REFOF(x) (__typeof__(x))((unsigned long)(x) & (~0x1l))
#define MARKED(x) (__typeof__(x))((unsigned long)(x) | 0x1l)

lf_list *new_lockfree_lists(int n) {
  lf_list *lists = malloc(n * sizeof(lf_list));
  for (int i = 0; i < n; ++i) {
    lists[i].head = NULL;
    lists[i].size = 0;
  }
  return lists;
}

int lf_list_find(volatile lf_list *l, unsigned reversed_key, lf_element **_prev,
                 lf_element **_curr) {
  // TODO: Potentially add volatile to the next line
  lf_element *prev = NULL, *curr = NULL, *next = NULL;
  while (true) {
  restart_find:

    prev = l->head;
    if (prev == NULL) return 0;
    curr = prev->next;
    while (MARKOF(curr)) {
      bool successful_phy_rm =
          __sync_bool_compare_and_swap(&l->head, prev, REFOF(curr));
      if (!successful_phy_rm) goto restart_find;
      prev = REFOF(curr);
      curr = prev->next;
    }

    if (prev == NULL) return 0;

    if (reversed_key <= prev->reversed_key) {
      *_curr = (lf_element *)REFOF(prev);
      return 0;
    }
    int num_steps = 0;

    while (1) {
      if (curr == NULL) {
        *_prev = (lf_element *)REFOF(prev);
        return num_steps;
      }
      next = curr->next;
      while (MARKOF(next)) {
        bool successful_phy_rm =
            __sync_bool_compare_and_swap(&prev->next, curr, REFOF(next));
        if (!successful_phy_rm) goto restart_find;
        curr = REFOF(next);
        next = curr->next;
      }

      if (reversed_key <= curr->reversed_key) {
        *_prev = (lf_element *)REFOF(prev);
        *_curr = (lf_element *)REFOF(curr);
        return num_steps;
      }
      prev = REFOF(curr);
      curr = REFOF(curr->next);
      num_steps++;
    }
  }
}

bool lf_list_add(lf_list *lists, int key, Packet_t *p, int *num_steps) {
  unsigned reversed_key = reverse_bits(key);
  lf_element *prev = NULL;
  lf_element *curr = NULL;

  lf_element *new_element = malloc(sizeof(lf_element));
  new_element->packet = p;
  new_element->key = key;
  new_element->reversed_key = reversed_key;

  while (1) {
    if (num_steps)
      *num_steps = lf_list_find(lists, reversed_key, &prev, &curr);
    else
      lf_list_find(lists, reversed_key, &prev, &curr);

    __sync_add_and_fetch(&lists->size, 1);
    if (prev == NULL) {
      new_element->next = curr;
      if (curr == NULL) {
        if (__sync_bool_compare_and_swap(&lists->head, curr, new_element)) {
          return true;
        }
        __sync_add_and_fetch(&lists->size, -1);
        continue;
      }
      if (curr->reversed_key == reversed_key) {
        free(new_element);
        __sync_add_and_fetch(&lists->size, -1);
        return false;
      }
      if (__sync_bool_compare_and_swap(&lists->head, curr, new_element)) {
        return true;
      }
      __sync_add_and_fetch(&lists->size, -1);
      continue;
    }

    if (curr && curr->reversed_key == reversed_key) {
      __sync_add_and_fetch(&lists->size, -1);
      free(new_element);
      return false;
    }

    new_element->next = curr;
    if (__sync_bool_compare_and_swap(&prev->next, curr, new_element)) {
      return true;
    }
    __sync_add_and_fetch(&lists->size, -1);
  }
}

bool lf_list_remove(lf_list *l, int key) {
  unsigned reversed_key = reverse_bits(key);
  lf_element *prev = NULL, *curr = NULL, *next = NULL;
  while (1) {
    lf_list_find(l, reversed_key, &prev, &curr);
    if (curr) {
      if (curr->reversed_key != reversed_key) {
        return false;
      }

      next = curr->next;
      bool successful_log_rm =
          __sync_bool_compare_and_swap(&curr->next, next, MARKED(next));
      if (!successful_log_rm) continue;
      if (prev) {
        __sync_bool_compare_and_swap(&prev->next, curr, REFOF(next));
      } else {
        __sync_bool_compare_and_swap(&l->head, curr, REFOF(next));
      }
      __sync_add_and_fetch(&l->size, -1);
      return true;
    }
    return false;
  }
}

bool lf_list_contains(lf_list *l, int key, int *num_steps) {
  unsigned reversed_key = reverse_bits(key);
  lf_element *curr = l->head;
  int n_steps = 0;

  if (!curr) {
    return false;
  }

  while (curr && reversed_key > curr->reversed_key) {
    n_steps++;
    curr = REFOF(curr->next);
  }
  if (num_steps) *num_steps = n_steps;

  return (curr && curr->reversed_key == reversed_key && !MARKOF(curr->next));
}

void destroy_lockfree_lists(lf_list *l, int n) {
  for (int i = 0; i < n; i++) {
    lf_element *curr = l[i].head, *next = NULL;
    while (curr) {
      next = REFOF(curr->next);
      free(REFOF(curr));
      curr = next;
    }
  }
  free(l);
}
