// vim:ts=4:sw=4:noet
#ifndef __PLUGIN_H_
#define __PLUGIN_H_

#include <string>
#include <ltdl.h>

#include <compat_hash_map.h>
#include <list>

using namespace std;

namespace qhub {

class ADC;

class Plugin {
public:
	enum Message {
		STARTED, STOPPED,
		CONNECTED, DISCONNECTED,
		LOGIN, AUTHENTICATED, INFO
	};
	
	/*
	 * Static methods
	 */
	static void init() throw();
	static void deinit() throw();
	static void openModule(const char* filename) throw();
	static void removeModule(const char* filename) throw();
	static void fire(Message m, ADC* client) throw();

	Plugin() throw() {};
	virtual ~Plugin() throw();
	virtual void on(Message m, ADC* client) throw() = 0;

private:
	typedef list<Plugin*> Plugins;
	static Plugins modules;
	typedef void* (*PLUGIN_START)();
	PLUGIN_START start;
	typedef void (*PLUGIN_STOP)();
	PLUGIN_STOP stop;
	
	//loads methods from our module
//	void loadFromModule();
//	void load(const char* name);


	/* Per-class stuff
	 * 
	 */

	//our handle, name
	string name;
	lt_dlhandle handle;

	//all our functions
	//this should be replaces by function pointers (performance)
//	hash_map<string, lt_ptr> functions;
};

}

#endif
