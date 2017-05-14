
#ifndef HASHPACKETTEST_H_
#define HASHPACKETTEST_H_

//static void millToTimeSpec(struct timespec *ts, unsigned long ms);
void serialHashPacketTest(int numMilliseconds,
							float fractionAdd,
							float fractionRemove,
							float hitRate,
							int maxBucketSize,
							long mean,
							int initSize);

#endif /* HASHPACKETTEST_H_ */
