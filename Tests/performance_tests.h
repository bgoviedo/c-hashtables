#ifndef TESTS_PERFORMANCE_H
#define TESTS_PERFORMANCE_H
#include "../hashtable_1.h"
void parallel_dispatcher 
        (int numMilliseconds,
        double fractionAdd,
        double fractionRemove,
        double hitRate,
        long mean,
        int initSize,
        int seed,
        int numWorkers,
        ht_type type);


#endif /* TESTS_PERFORMANCE_H */
