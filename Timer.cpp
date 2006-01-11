// vim:ts=4:sw=4:noet
#include <cassert>
#include <cstring>

#include "Timer.h"

using namespace qhub;

static void callback(int, short event, void* arg)
{
	assert(event == EV_TIMEOUT);
	Timer* t = static_cast<Timer*>(arg);
	t->onAlarm();
}

Timer::Timer() throw()
{
	memset(&ev, sizeof(struct event), 0);
	evtimer_set(&ev, callback, this);
}

Timer::~Timer() throw()
{
	evtimer_del(&ev);
}

void Timer::alarm(unsigned seconds) throw() {
	evtimer_del(&ev);
	if(!seconds)
		return; // don't re-add, 0 means deactivate
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	evtimer_add(&ev, &tv);
}
