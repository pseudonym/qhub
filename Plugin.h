// vim:ts=4:sw=4:noet
#ifndef __PLUGIN_H_
#define __PLUGIN_H_

#include <string>
#include <ltdl.h>

#include <compat_hash_map.h>
#include <list>

#include "Util.h"

using namespace std;

namespace qhub {

class ADC;

class Plugin {
public:
	enum Message {
		STARTED,		// Plugin started (called after construction)
		STOPPED,		// Plugin stopped (called before destruction)
		CONNECTED,		// User connected (param ADC*)
		DISCONNECTED,	// User disconnected (param ADC*)
		LOGIN,			// User tries to log in (param ADC*)
		AUTHENTICATED,	// User has sent a correct password (param ADC*)
		INFO,			// User sends a new INF line (param ADC*)
		COMMAND			// User sends direct message to hub cid
	};
	
	/*
	 * Static methods
	 */
	static void init() throw();
	static void deinit() throw();
	static void openModule(const char* filename) throw();
	static void removeModule(const char* filename) throw();
	static void fire(Message m, ADC* client = NULL, string const& msg = Util::emptyString) throw();

	Plugin() throw() {};
	virtual ~Plugin() throw();
	virtual void on(Message m, ADC* client, string const& msg) throw() = 0;

private:
	typedef list<Plugin*> Plugins;
	static Plugins modules;
	typedef void* (*get_plugin_t)();

	string name;
	lt_dlhandle handle;
};

}

#endif
