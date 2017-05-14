
#include <time.h>
#include "hashpackettest.h"
#include "stopwatch.h"
#include "hashgenerator.h"
#include "paddedprim.h"
#include "statistics.h"
#include <stdbool.h>
#include <pthread.h>
#include "hashpacketworker.h"

static void millToTimeSpec(struct timespec *ts, unsigned long ms)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000;
}


void serialHashPacketTest(int numMilliseconds,
							float fractionAdd,
							float fractionRemove,
							float hitRate,
							int maxBucketSize,
							long mean,
							int initSize)
{

	StopWatch_t timer;

	PaddedPrimBool_NonVolatile_t done;
	done.value = false;

//	PaddedPrimBool_t memFence;
//	memFence.value = false;

	HashPacketGenerator_t * source = createHashPacketGenerator(fractionAdd,fractionRemove,hitRate,mean);
	SerialHashTable_t * table = createSerialHashTable(1, maxBucketSize);

	for( int i = 0; i < initSize; i++ ) {
	  HashPacket_t * pkt = getAddPacket(source);
	  add_ht(table,mangleKey(pkt), pkt->body);
	}

	pthread_t workerThread;
	int rc;
	pthread_attr_t attr;
	void *status;

	struct timespec tim;

	millToTimeSpec(&tim,numMilliseconds);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	SerialPacketWorker_t  workerData = {&done, source, table,0,0,0};

	rc = pthread_create(&workerThread, &attr, (void *) &serialWorker, (void *) &workerData);

	if (rc){
		fprintf(stderr,"ERROR; return code from pthread_create() for solo thread is %d\n", rc);
		exit(-1);
	}

	printf("Sleeping!!");

	startTimer(&timer);

	nanosleep(&tim, NULL);

	done.value = true;

//	memFence.value = true;

	 rc = pthread_join(workerThread, &status);
	 if (rc){
		 fprintf(stderr,"firewall error: return code for the threads using pthread_join() for solo thread  is %d\n", rc);
		 exit(-1);
	 }

	 stopTimer(&timer);

	 pthread_attr_destroy(&attr);

	long totalCount = workerData.totalPackets;
	printf("count: %ld \n", totalCount);
	printf("time: %f\n",getElapsedTime(&timer));
	printf("%f inc / ms", totalCount/getElapsedTime(&timer));
}
