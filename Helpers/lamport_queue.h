#ifndef FIREWALL_LAMPORTQUEUE_
#define FIREWALL_LAMPORTQUEUE_

#include <stdbool.h>
#include "../Utils/hashgenerator.h"

typedef struct lamport_queue {
  volatile int head, tail;
  int enqueue_count;
  int capacity;
  int queue_id;
  void** items;
} lamport_queue;

lamport_queue* new_queues(int num_queues, int capacity);
bool enqueue(lamport_queue* queue, void*);
bool dequeue(lamport_queue* queue, void**);
bool queue_is_full(lamport_queue* queue);
void free_queues(int num_queues, lamport_queue* queues);

#endif /* FIREWALL_LAMPORTQUEUE_ */