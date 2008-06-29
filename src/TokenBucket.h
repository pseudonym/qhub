#ifndef QHUB_TOKENBUCKET_H
#define QHUB_TOKENBUCKET_H

#include "qhub.h"

#include <sys/time.h>
#include <time.h>

namespace qhub {

class TokenBucket
{
public:
	// interval defaults to one second
	TokenBucket(int m, int inc, int in = 1000*1000);

	/*
	 * Updates the amount in the bucket based on how much time since last
	 * call, uses up the specified amount, and returns true if it's ok to
	 * send, false otherwise.
	 */
	bool use(int amt);

private:
	int count;
	timeval last;

	int max; // max elements in bucket
	int increment; // added each interval
	int interval; // in microseconds
};

} // namespace qhub

#endif // QHUB_TOKENBUCKET_H
