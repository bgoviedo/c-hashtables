#ifndef PACKETSOURCE_H_
#define PACKETSOURCE_H_

#include "generators.h"

typedef struct {
	long iterations;
	long seed;
}Packet_t;

typedef struct {
	UniformGenerator_t  * uniformGen;
	UniformGenerator_t  * uniformSeed;
	long  * uniformCounts;

	ExponentialGenerator_t  * exponentialGen;
	UniformGenerator_t  * exponentialSeed;
	long  * exponentialCounts;
}PacketSource_t;

PacketSource_t * createPacketSource(long mean, int numSources, short seed);

volatile Packet_t * getUniformPacket(PacketSource_t * packetSource, int soruceNum);
long getUniformCount(PacketSource_t * packetSource, int soruceNum);

volatile Packet_t * getExponentialPacket(PacketSource_t * packetSource, int sourceNum);
long getExponentialCount(PacketSource_t * packetSource, int sourceNum);

void deletePacketSource(PacketSource_t * packetSource);


#endif /* PACKETSOURCE_H_ */
