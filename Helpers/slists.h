#ifndef SLISTS_H
#define SLISTS_H

#include "../Utils/packetsource.h"


typedef struct slist_element slist_element;
struct slist_element {
  Packet_t *packet;
  unsigned int key;
  unsigned int reversed_key;
  slist_element *next;
};

typedef struct slist {
  slist_element *head;
  volatile int size;
} slist;

slist *new_serial_lists(int numlists);
bool slist_add(slist *lists, int key, Packet_t *p);
bool slist_remove(slist *lists, int key);
bool slist_contains(slist *lists, int key);
void destroy_slists(slist *lists, int size);

#endif /* SLISTS_H */