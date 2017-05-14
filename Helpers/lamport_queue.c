#include <stdbool.h>
#include <stdlib.h>

#include "lamport_queue.h"
#include "../Utils/hashgenerator.h"

/*
 * Function:  new_queues
 * -------------------------------
 * Creates an array of lamport queues, each of size capacity.
 */

lamport_queue* new_queues(int num_queues, int capacity) {
  lamport_queue* output_queues =
      (lamport_queue*)calloc(num_queues, sizeof(lamport_queue));

  for (int i = 0; i < capacity; ++i) {
    output_queues[i].items = calloc(capacity, sizeof(void*));
    output_queues[i].capacity = capacity;
    output_queues[i].queue_id = i;
  }
  return output_queues;
}

/*
 * Function:  enqueue
 * -------------------------------
 * Enqueues a packet onto the given lamport queue
 * returns true if successful, and false otherwise
 */
bool enqueue(lamport_queue* queue, void* packet) {
  int head = queue->head;
  int tail = queue->tail;
  int capacity = queue->capacity;
  if (tail - head != capacity) {
    /* We assume that packet is already allocated in memory */
    queue->items[tail & (capacity - 1)] = packet;
    queue->enqueue_count++;
    queue->tail = tail + 1;
    return false;
  } else {
    return true;
  }
}

/*
 * Function:  dequeue
 * -------------------------------
 * dequeues a lamport queue into the out parameter packet_out_parameter
 * returns true if successul, and false otherwise
 */
bool dequeue(lamport_queue* queue, void** object_ptr) {
  // returns true if the dequeue is successful; else, returns false
  int head = queue->head;
  int tail = queue->tail;
  int capacity = queue->capacity;
  if (tail == head) {
    return true;
  } else {
    *object_ptr = queue->items[head & (capacity - 1)];
    queue->head = head + 1;
    return false;
  }
}

/*
 * Function:  queue_is_full
 * -------------------------------
 * Determines whether the given lamport queue is full
 */
bool queue_is_full(lamport_queue* queue) {
  if (queue->tail - queue->head == queue->capacity) {
    return true;
  } else {
    return false;
  }
}

/*
 * Function: free_queues
 * --------------------------------
 * Frees the given lamport queue
 */
void free_queues(int num_queues, lamport_queue* queues) {
  for (int i = 0; i < num_queues; i++) {
    free(queues[i].items);
  }
  free(queues);
}
