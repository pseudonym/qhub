#ifndef QHUB_PLUGINMANAGER_H
#define QHUB_PLUGINMANAGER_H

#include "Singleton.h"
#include "Plugin.h"
#include "Util.h"

#include <vector>
#include <string>

namespace qhub {

class PluginManager : public Singleton<PluginManager> {
public:
	static const std::string PLUGIN_EXTENSION;

	bool open(std::string const& name, std::string const& insertBefore = Util::emptyString) throw();
	bool remove(std::string const& name) throw();
	void removeAll() throw();
	bool has(std::string const& name) throw();

	/*
	 * Iterators
	 */
	typedef std::vector<Plugin*>::iterator iterator;
	iterator begin() throw() { return plugins.begin(); };
	iterator end() throw() { return plugins.end(); };

	template<typename T0>
	void fire(T0& type) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type);
		}
	}
	template<typename T0, class T1>
	void fire(T0& type, T1& c1) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1);
		}
	}
	template<typename T0, class T1, class T2>
	void fire(T0& type, T1& c1, T2& c2) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2);
		}
	}
	template<typename T0, class T1, class T2, class T3>
	void fire(T0& type, T1& c1, T2& c2, T3& c3) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2, c3);
		}
	}
	template<typename T0, class T1, class T2, class T3, class T4>
	void fire(T0& type, T1& c1, T2& c2, T3& c3, T4& c4) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2, c3, c4);
		}
	}
	template<typename T0, class T1, class T2, class T3, class T4, class T5>
	void fire(T0& type, T1& c1, T2& c2, T3& c3, T4& c4, T5& c5) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2, c3, c4, c5);
		}
	}

private:
	typedef std::vector<Plugin*> Plugins;
	Plugins plugins;
	typedef void* (*get_plugin_t)();
};

} // namespace qhub

#endif // QHUB_PLUGINMANAGER_H

