// vim:ts=4:sw=4:noet

#ifndef QHUB_SPEAKER_H
#define QHUB_SPEAKER_H

#include <set>

namespace qhub {

template<typename Listener>
class Speaker {
	typedef std::set<Listener*> Listeners;
	Listeners listeners;
	Listeners tmp; // needed if one of the on() calls adds/removes
public:
	void addListener(Listener* l)
	{
		listeners.insert(l);
	}

	void removeListener(Listener* l)
	{
		listeners.erase(l);
	}

	void removeAllListeners()
	{
		listeners.clear();
	}

	template<typename T0>
	void fire(T0& type) throw() {
		tmp = listeners;
		for(typename Listeners::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->on(type);
		}
	}
	template<typename T0, class T1>
	void fire(T0& type, T1& c1) throw() {
		tmp = listeners;
		for(typename Listeners::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->on(type, c1);
		}
	}
	template<typename T0, class T1, class T2>
	void fire(T0& type, T1& c1, T2& c2) throw() {
		tmp = listeners;
		for(typename Listeners::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->on(type, c1, c2);
		}
	}
	template<typename T0, class T1, class T2, class T3>
	void fire(T0& type, T1& c1, T2& c2, T3& c3) throw() {
		tmp = listeners;
		for(typename Listeners::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->on(type, c1, c2, c3);
		}
	}
	template<typename T0, class T1, class T2, class T3, class T4>
	void fire(T0& type, T1& c1, T2& c2, T3& c3, T4& c4) throw() {
		tmp = listeners;
		for(typename Listeners::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->on(type, c1, c2, c3, c4);
		}
	}
	template<typename T0, class T1, class T2, class T3, class T4, class T5>
	void fire(T0& type, T1& c1, T2& c2, T3& c3, T4& c4, T5& c5) throw() {
		tmp = listeners;
		for(typename Listeners::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->on(type, c1, c2, c3, c4, c5);
		}
	}
};

} // namespace qhub

#endif // QHUB_SPEAKER_H
