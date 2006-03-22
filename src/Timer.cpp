// vim:ts=4:sw=4:noet
#include <cassert>
#include <cstring>

#include "Timer.h"

using namespace std;
using namespace qhub;

static void callback(int, short event, void* arg) throw()
{
	assert(event == EV_TIMEOUT);
	Timer* t = static_cast<Timer*>(arg);
	TimerListener::Timeout foo;
	t->fire(foo);
	delete t;
}

// factory to hide ugly fact that we're just calling operator new
Timer* Timer::makeTimer(unsigned seconds, unsigned usec) throw()
{
	return new Timer(seconds, usec);
}

Timer::Timer(unsigned seconds, unsigned usec) throw()
{
	memset(&ev, sizeof(struct event), 0);
	evtimer_set(&ev, callback, this);
	tv.tv_sec = seconds;
	tv.tv_usec = usec;
	evtimer_add(&ev, &tv);
}

Timer::~Timer() throw()
{
	evtimer_del(&ev);
}
