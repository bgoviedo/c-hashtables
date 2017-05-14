#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include <sys/time.h>

typedef struct {
	struct timeval startTime;
	struct timeval stopTime;
}StopWatch_t;

void startTimer(StopWatch_t *);

void stopTimer(StopWatch_t *);

double getElapsedTime(StopWatch_t *);

#endif /* STOPWATCH_H_ */
