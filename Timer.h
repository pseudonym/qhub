// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_TIMER_H_
#define _INCLUDED_TIMER_H_

#include "EventHandler.h"

using namespace std;

namespace qhub {

class Timer {
public:
	Timer() throw();
	virtual ~Timer() throw();
	void alarm(unsigned seconds = 0) throw();
	virtual void onAlarm() throw() {}

private:
	struct event ev;
	struct timeval tv;
};

} //namespace qhub

#endif //_INCLUDED_TIMER_H_
