// vim:ts=4:sw=4:noet
#include "Timer.h"

using namespace qhub;

unsigned Timer::second = 0;
bool Timer::modified = false;
Timer::Alarms Timer::alarms;
Timer::Alarms Timer::alarms2;

void Timer::tick() throw() {
	second++;
	if(modified) {
		for(Alarms::iterator i = alarms2.begin(); i != alarms2.end(); ++i)
			alarms[i->first] = i->second;
		for(bool done = true; !done; done = true) {
			for(Alarms::iterator i = alarms.begin(); i != alarms.end(); ++i) {
				if(i->second == 0) {
					alarms.erase(i);
					done = false;
					break;
				}
			}
		}
		modified = false;
		alarms2.clear();
	}
	for(Alarms::iterator i = alarms.begin(); i != alarms.end(); ++i) {
		i->second--;
		if(i->second == 0)
			(i->first)->onAlarm();
	}
}

void Timer::alarm(unsigned seconds) throw() {
	alarms2[this] = seconds;
	modified = true;
}

/*
#include <stdio.h>

class A : public Timer {
public:
	A() {};
	void wakeMe(unsigned s) {
		alarm(s);
	}
	virtual void onAlarm() throw() {
		fprintf(stderr, "alarm at %u\n", Timer::getSecond());
		alarm(1);
	}
};
	
int main() {
	A a, b, c;
	a.wakeMe(3);
	b.wakeMe(5);
	c.wakeMe(7);
	for(int i = 0; i < 50; ++i) {
		Timer::tick();
	}
	return 0;
}
*/
