// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_H_
#define _INCLUDED_PLUGIN_H_

#include <string>
#include <ltdl.h>

#include <compat_hash_map.h>
#include <list>

#include "Util.h"

using namespace std;

namespace qhub {

class ADCClient;

class Plugin {
public:
	enum Message {
		STARTED,		// Plugin started (called after construction)
		STOPPED,		// Plugin stopped (called before destruction)
		CONNECTED,		// User connected (param ADCClient*)
		DISCONNECTED,	// User disconnected (param ADCClient*)
		LOGIN,			// User tries to log in (param ADCClient*)
		AUTHENTICATED,	// User has sent a correct password (param ADCClient*)
		INFO,			// User sends a new INF line (param ADCClient*)
		COMMAND			// User sends direct message to hub cid
	};

	enum Action {
		NONE = 0,
		HANDLED = 1,	// I handled the command
		STOP = 2,		// Don't handle this command
	};
	
	/*
	 * Static methods
	 */
	static void init() throw();
	static void deinit() throw();
	static void openModule(const char* filename) throw();
	static void removeModule(const char* filename) throw();
	static void removeAllModules() throw();
	static void fire(Message m, int& a, ADCClient* client = NULL, string const& msg = Util::emptyString) throw();

	Plugin() throw() {};
	virtual ~Plugin() throw() {};
	virtual void on(Message m, int& a, ADCClient* client, string const& msg) throw() = 0;

private:
	typedef list<Plugin*> Plugins;
	static Plugins modules;
	typedef void* (*get_plugin_t)();
	string name;
	lt_dlhandle handle;
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_H_
