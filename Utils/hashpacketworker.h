
#ifndef HASHPACKETWORKER_H_
#define HASHPACKETWORKER_H_

#include "paddedprim.h"
#include "hashgenerator.h"
#include "hashtable.h"

typedef struct {
	PaddedPrimBool_NonVolatile_t * done;
	HashPacketGenerator_t * source;
	SerialHashTable_t * table;
	long totalPackets;
	long residue;
	long fingerprint;
}SerialPacketWorker_t;


void serialWorker(SerialPacketWorker_t * data);


#endif /* HASHPACKETWORKER_H_ */
