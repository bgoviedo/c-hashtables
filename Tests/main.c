#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../Utils/hashpackettest.h"
#include "performance_tests.h"

int main(int argc, char** argv) {
    if (!strcmp(argv[1], "serial")) {
        serialHashPacketTest(atoi(argv[2]), atof(argv[3]), atof(argv[4]), 
                atof(argv[5]), atoi(argv[6]), atol(argv[7]), atoi(argv[8]));
    }

    if (!strcmp(argv[1], "parallel")) {
        parallel_dispatcher(atoi(argv[2]), atof(argv[3]), atof(argv[4]), 
                atof(argv[5]), atol(argv[6]), atoi(argv[7]), atoi(argv[8]), atoi(argv[9]), atoi(argv[10]));
    }
    return 0;
}