
#include "hashpacketworker.h"
#include "fingerprint.h"
#include "hashtable.h"
#include "paddedprim.h"

void serialWorker(SerialPacketWorker_t * data){
	HashPacket_t * pkt;

	while( !data->done->value ) {
		data->totalPackets++;
		pkt = getRandomPacket(data->source);
		data->residue += getFingerprint(pkt->body->iterations, pkt->body->seed);

		switch(pkt->type) {
	        case Add:
	          add_ht(data->table,mangleKey(pkt),pkt->body);
	          break;
	        case Remove:
	          remove_ht(data->table,mangleKey(pkt));
	          break;
	        case Contains:
	          contains_ht(data->table,mangleKey(pkt));
	          break;
	      }
	    }
}
