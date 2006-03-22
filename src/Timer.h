// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_TIMER_H_
#define _INCLUDED_TIMER_H_

#include "EventHandler.h"
#include "Speaker.h"

using namespace std;

namespace qhub {

struct TimerListener {
	template<int I> struct X { enum { TYPE = I }; };

	typedef X<0> Timeout;

	virtual void on(Timeout) throw() = 0;
	virtual ~TimerListener() throw() {}
};

class Timer : public Speaker<TimerListener> {
public:
	static Timer* makeTimer(unsigned seconds, unsigned usec = 0) throw();

	Timer(unsigned seconds, unsigned usec = 0) throw();
	virtual ~Timer() throw();

private:
	struct event ev;
	struct timeval tv;
};

} //namespace qhub

#endif //_INCLUDED_TIMER_H_
