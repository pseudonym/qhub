#ifndef __Time_h_
#define __Time_h_

#include "config.h"
#include "types.h"

#include <cassert>

#include "time.h"
#if defined(HAVE_SYS_TIME_H) && defined(TIME_WITH_SYS_TIME)
#include "sys/time.h"
#endif

namespace qhub {

//all time is based on nanoseconds
//gettimeofday, RDTSC, time

//gives nanoseconds, not since any particular time
//supposed to be used for _differences_ and nothing else, so the wrapping at
//2^64 (<1year) is probably survivable
static uint64_t currentTimeNanos()
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
	if(gettimeofday(&tv, NULL) == -1){
		assert(0 && "gettimeofday errored on us\n");
		return 0;
	}

	return uint64_t(tv.tv_sec)*1000000000 + uint64_t(tv.tv_usec)*1000;
#endif //HAVE_GETTIMEOFDAY
}

class difftimer
{
public:
	difftimer() { oldTime = currentTimeNanos(); };

	uint64_t getDiff() {
		uint64_t temp = oldTime;
		oldTime = currentTimeNanos();
		if(oldTime < temp){
			//try to figure where it wrapped?
			//remember this could alse be NTPD doing dirty things to us
			return 0;
		}
		return oldTime-temp;
	};
private:
	uint64_t	oldTime;

};


} //namespace qhub


#if 0
timespec s, r;
s.tv_sec=0;
s.tv_nsec=100000;
for(int i=0; i<100; i++){
	nanosleep(&s, &r);
	printf("%d\n", r.tv_nsec);
	//printf("bah\n");
}
#endif

#endif //_Time_h_
