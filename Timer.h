// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_TIMER_H_
#define _INCLUDED_TIMER_H_

#include <map>

using namespace std;

namespace qhub {

class Timer {
private:
	typedef map<Timer*, unsigned> Alarms;
	static Alarms alarms, alarms2;
	static unsigned second;
	static bool modified;
	
public:
	static unsigned getSecond() throw() { return second; };
	static void tick() throw();

	Timer() {};
	virtual ~Timer() { alarm(0); };
	void alarm(unsigned seconds = 0) throw();
	virtual void onAlarm() throw() {};
};

} //namespace qhub

#endif //_INCLUDED_TIMER_H_
