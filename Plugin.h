// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_H_
#define _INCLUDED_PLUGIN_H_

#include <string>
#include <ltdl.h>

#include <compat_hash_map.h>
#include <list>

#include "Util.h"
#include "UserData.h"

using namespace std;

namespace qhub {

class ADCClient;

class Plugin {
public:
	static UserData data;
	
	/*
	 * Some stuff
	 */
	Plugin() throw() {};
	virtual ~Plugin() throw() {};
	string const& getId() const throw() { return name; };

	/*
	 * Plugin calls
	 */
	template<int I>	struct X { enum { TYPE = I };  };

	// These messages are always sent
	typedef X<0> PluginStarted;
	typedef X<1> PluginStopped;
	typedef X<2> PluginMessage;
	// These messages must be requested (FIXME.. always sent for now)
	typedef X<3> ClientConnected;
	typedef X<4> ClientDisconnected;
	typedef X<5> ClientLine;
	typedef X<6> ClientLogin;
	typedef X<7> ClientAuthenticated;
	typedef X<8> ClientAuthFailed;
	typedef X<9> ClientInfo;
	typedef X<10> ClientCommand;
	typedef X<11> ClientMessage;
	//typedef X<13> ClientUserDisconnect;
	//typedef X<14> ClientUserKick;
	//typedef X<15> ClientUserBan;
	//typedef X<16> ClientUserRedirect;

	template<typename T0>
	static void fire(T0 type) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type);
		}
	}
	template<typename T0, class T1>
	static void fire(T0 type, T1 const& c1) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1);
		}
	}
	template<typename T0, class T1, class T2>
	static void fire(T0 type, T1 const& c1, T2& c2) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2);
		}
	}
	template<typename T0, class T1, class T2>
	static void fire(T0 type, T1 const& c1, T2 const& c2) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2);
		}
	}
	template<typename T0, class T1, class T2, class T3>
	static void fire(T0 type, T1 const& c1, T2& c2, T3& c3) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2, c3);
		}
	}
	
	// Called after construction
	// parm: Plugin* = the plugin
	virtual void on(PluginStarted, Plugin*) throw() {};
	// Called before destruction
	// parm: Plugin* = the plugin
	virtual void on(PluginStopped, Plugin*) throw() {};
	// Called by a plugin
	// parm: Plugin* = the plugin
	// parm: void* = the custom data
	virtual void on(PluginMessage, Plugin*, void*) throw() {};
	// Called when a client connects
	// note: not everything may be initialized
	// parm: ADCClient* = the client
	virtual void on(ClientConnected, ADCClient*) throw() {};
	// Called when a client disconnects
	// note: the client may not have been added to the userlist
	// parm: ADCClient* = the client
	virtual void on(ClientDisconnected, ADCClient*) throw() {};
	// Called on every client input
	// parm: ADCClient* = the client
	virtual void on(ClientLine, ADCClient*) throw() {};
	// Called when a client sends first INF (modify INF's through attributes)
	// parm: ADCClient* = the client
	virtual void on(ClientLogin, ADCClient*) throw() {};
	// Called when a client sends a correct authentication token (extra parm: password)
	// parm: ADCClient* = the client
	// parm: string = the correctly used password
	virtual void on(ClientAuthenticated, ADCClient*, string const&) throw() {};
	// Called when a client sends an incorrect authentication token
	// parm: ADCClient* = the client
	// parm: string = the correct yet unused password
	virtual void on(ClientAuthFailed, ADCClient*, string const&) throw() {};
	// Called when a client sends INFs after logon
	virtual void on(ClientInfo, ADCClient*) throw() {};
	// Called when a client sends a direct PM to the hub CID
	// parm: ADCClient* = the client
	// parm: string = the message
	virtual void on(ClientCommand, ADCClient*, string&) throw() {};
	// Called when a client sends a normal broadcast chat message
	// parm: ADCClient* = the client
	// parm: string = the message
	virtual void on(ClientMessage, ADCClient*, string&) throw() {};

	/*
	 * Static methods
	 */
	static void init() throw();
	static void deinit() throw();
	static bool openModule(string const& filename, string const& insertBefore = Util::emptyString) throw();
	static bool removeModule(string const& filename) throw();
	static void removeAllModules() throw();
	static bool hasModule(string const& filename) throw();

	/*
	 * Iterators
	 */
	typedef list<Plugin*>::iterator iterator;
	static iterator begin() throw() { return plugins.begin(); };
	static iterator end() throw() { return plugins.end(); };

private:
	typedef list<Plugin*> Plugins;
	static Plugins plugins;
	typedef void* (*get_plugin_t)();
	string name;
	lt_dlhandle handle;
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_H_
