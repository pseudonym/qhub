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
		alarms.insert(alarms2.begin(), alarms2.end());
		alarms2.clear();
		modified = false;
		for(Alarms::iterator i = alarms.begin(); i != alarms.end(); ) {
			if(i->second == 0)
				alarms.erase(i++);
			else
				++i;
		}
	}
	for(Alarms::iterator i = alarms.begin(); i != alarms.end(); ++i) {
		i->second--;
		if(i->second == 0) {
			Timer* t = i->first;
			alarms.erase(i);
			t->onAlarm();
		}
	}
}

void Timer::alarm(unsigned seconds) throw() {
	alarms2[this] = seconds;
	modified = true;
}
