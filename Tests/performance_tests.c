#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "../Utils/stopwatch.h"
#include "../Utils/hashgenerator.h"
#include "../Utils/hashpackettest.h"
#include "../Utils/fingerprint.h"

#include "../Helpers/lamport_queue.h"
#include "../hashtable_1.h"

#define QUEUEDEPTH 16

static inline int log2val(int val) {
    int result = 0;
    while (val >>= 1, val)
        result++;
    return result;
}

void *alarm1(void *numMilliseconds) {
    int *num_ms = (int *) numMilliseconds;
    usleep(*num_ms * 1000);
    *num_ms = 0;
    return NULL;
}

typedef struct worker_data {
    hashtable_1* table;
    lamport_queue *queue;
    long sum;
} worker_data;

void *load_worker(void *_data) {
    worker_data *data = (worker_data *) _data;
    lamport_queue *queue = data->queue;
    hashtable_1 *table = data->table;
    volatile HashPacket_t *packet;
    while (true) {
        if (dequeue(queue, (void**)&packet)) {
            continue;
        }
        if (!packet) {
            break;
        }
        data->sum += getFingerprint(packet->body->iterations, packet->body->seed);
        switch (packet->type) {
            case Add:
                table->add(table, mangleKey(packet), (Packet_t *) packet->body);
                break;
            case Remove:
                table->remove(table, mangleKey(packet));
                break;
            case Contains:
                table->contains(table, mangleKey(packet));
                break;
        }
    }
    return NULL;
}

void *no_load_worker(void *_data) {
    worker_data *data = (worker_data *) _data;
    lamport_queue *queue = data->queue;
    volatile HashPacket_t *packet = NULL;
    while (true) {
        if (dequeue(queue, (void**)&packet)) {
            continue;
        }
        if (!packet) {
            break;
        }
    }
    return NULL;
}

void parallel_dispatcher 
        (int numMilliseconds,
        double fractionAdd,
        double fractionRemove,
        double hitRate,
        long mean,
        int initSize,
        int seed,
        int numWorkers,
        ht_type type)
        {

    bool noLoad = false;
    if (mean == 0) {
        mean = 1;
        noLoad = true;
    }
    
    HashPacketGenerator_t *packet_src = createHashPacketGenerator(fractionAdd, fractionRemove, hitRate, mean);
    hashtable_1 *table = new_hashtable_1(type, 1 << (log2val(numWorkers) + 1));

    for (int i = 0; i < initSize; i++) {
        HashPacket_t *packet = getAddPacket(packet_src);
        table->add(table, mangleKey(packet), (Packet_t *) packet->body);
    }
    lamport_queue* queues = new_queues(numWorkers, QUEUEDEPTH);
    worker_data *data = calloc(numWorkers, sizeof(worker_data));
    for (int i = 0; i < numWorkers; i++) {
        data[i].table = table;
        data[i].queue = queues + i;
    }
    pthread_t *workers = calloc(numWorkers, sizeof(pthread_t));
    for (int i = 0; i < numWorkers; ++i) {
        if (noLoad) {
            pthread_create(workers + i, NULL, no_load_worker, (void *) (data + i));
        } else {
            pthread_create(workers + i, NULL, load_worker, (void *) (data + i));
        }
    }

    long counter = 0;
    volatile int not_done = numMilliseconds;
    pthread_t alarm_thread;
    pthread_create(&alarm_thread, NULL, alarm1, (void *) &not_done);

    while (not_done) {
        for (int i = 0; i < numWorkers; ++i) {
            if (queue_is_full(queues + i))
                continue;
            HashPacket_t *packet = getRandomPacket(packet_src);
            enqueue(queues + i,(void*)packet);
            counter++;
        }
    }
    printf("%ld\n", counter);
    exit(0);
}
