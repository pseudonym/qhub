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
	
	enum {
		// Plugin messages
		PLUGIN_STARTED,
		PLUGIN_STOPPED,
		PLUGIN_MESSAGE,
		// Client messages
		CLIENT_CONNECTED,		// client connected (not everything may be initialized??)
		CLIENT_DISCONNECTED,	// client disconnected
		CLIENT_LINE,			// client send data
		CLIENT_LOGIN,			// client tries to authenticate with first BINF
		CLIENT_INFO,			// client has sent a BINF (not called on first BINF)
		// Logged in client messages
		USER_CONNECTED,			// user connected / logged in properly
		USER_DISCONNECTED,		// user disconnected
		USER_COMMAND,			// user sends private message to hub bot
		USER_MESSAGE,			// user sends message
		USER_PRIVATEMESSAGE,	// user sends message with PM flag
		// Last
		LAST
	};
	// add AUTH_FAILED
	//
	// FIXME add someway to disable stringlist editing when not in CLIENT_LINE

	enum Action {
		NOTHING = 0x0,
		MODIFY = 0x1,
		MODIFIED = MODIFY, // plugin has modified the input
		REPLY = 0x2,
		REPLIED = REPLY, // plugin has made a reply
		HANDLE = 0x4,
		HANDLED = HANDLE, // plugin has done something (other plugins may choose to do nothing now)
		STOP = 0x8,
		STOPPED = STOP, // plugin requests that hub does not process the command/message
		DISCONNECT = 0x16,
		DISCONNECTED = DISCONNECT // client has been removed, do as little as possible
	};

	template<int I, int C> struct ActionType {
		enum { actionType = I };
		ActionType() throw() : reply(NULL), can(C), does(0) { };
		~ActionType() throw() { if(reply) delete reply; };
		void setState(Action d) throw() { assert((can & d) == d); does |= d; };
		bool isSet(Action d) const throw() { return (does & d) == d; }
		StringList* reply;
	private:
		int can;
		int does;
	};
	
	typedef ActionType<PLUGIN_STARTED, NOTHING>
			PluginStarted;
	typedef ActionType<PLUGIN_STOPPED, NOTHING>
			PluginStopped;
	typedef ActionType<PLUGIN_MESSAGE, HANDLE>
			PluginMessage;
	
	typedef ActionType<CLIENT_CONNECTED, HANDLE | DISCONNECT>
			ClientConnected;
	typedef ActionType<CLIENT_DISCONNECTED, HANDLE>
			ClientDisconnected;
	typedef ActionType<CLIENT_LINE, MODIFY | HANDLE | STOP | DISCONNECT>
			ClientLine;
	typedef ActionType<CLIENT_LOGIN, HANDLE | DISCONNECT>
			ClientLogin;
	typedef ActionType<CLIENT_INFO, HANDLE | DISCONNECT>
			ClientInfo;
	
	typedef ActionType<USER_CONNECTED, HANDLE | DISCONNECT>
			UserConnected;
	typedef ActionType<USER_DISCONNECTED, HANDLE>
			UserDisconnected;
	typedef ActionType<USER_COMMAND, REPLY | HANDLE | STOP | DISCONNECT>
			UserCommand;
	typedef ActionType<USER_MESSAGE, REPLY | HANDLE | STOP | DISCONNECT>
			UserMessage;
	typedef ActionType<USER_PRIVATEMESSAGE, REPLY | HANDLE | STOP | DISCONNECT>
			UserPrivateMessage;

	
	template<typename T0>
	static void fire(T0& type) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type);
		}
	}
	template<typename T0, class T1>
	static void fire(T0& type, T1& c1) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1);
		}
	}
	template<typename T0, class T1, class T2>
	static void fire(T0& type, T1& c1, T2& c2) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2);
		}
	}
	template<typename T0, class T1, class T2, class T3>
	static void fire(T0& type, T1& c1, T2& c2, T3& c3) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2, c3);
		}
	}
	template<typename T0, class T1, class T2, class T3, class T4>
	static void fire(T0& type, T1& c1, T2& c2, T3& c3, T4& c4) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2, c3, c4);
		}
	}
	template<typename T0, class T1, class T2, class T3, class T4, class T5>
	static void fire(T0& type, T1& c1, T2& c2, T3& c3, T4& c4, T5& c5) throw() {
		for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->on(type, c1, c2, c3, c4, c5);
		}
	}

	// Called after construction
	// parm: Plugin* = the plugin
	virtual void on(PluginStarted&, Plugin*) throw() {};
	// Called before destruction
	// parm: Plugin* = the plugin
	virtual void on(PluginStopped&, Plugin*) throw() {};
	// Called by a plugin
	// parm: Plugin* = the plugin
	// parm: void* = the custom data
	virtual void on(PluginMessage&, Plugin*, void*) throw() {};
	// Called when a client connects
	// note: not everything may be initialized
	// parm: ADCClient* = the client
	virtual void on(ClientConnected&, ADCClient*) throw() {};
	// Called when a client disconnects
	// note: the client may not have been added to the userlist
	// parm: ADCClient* = the client
	virtual void on(ClientDisconnected&, ADCClient*) throw() {};
	// Called on every client input
	// parm: ADCClient* = the client
	virtual void on(ClientLine&, ADCClient*, u_int32_t const, StringList) throw() {};
	// Called when a client sends first INF (modify INF's through attributes)
	// parm: ADCClient* = the client
	virtual void on(ClientLogin&, ADCClient*) throw() {};
	// Called when a client sends INFs
	virtual void on(ClientInfo&, ADCClient*) throw() {};
	virtual void on(UserConnected&, ADCClient*) throw() {};
	virtual void on(UserDisconnected&, ADCClient*) throw() {};
	// Called when a client sends a direct PM to the hub CID
	// parm: ADCClient* = the client
	// parm: string = the message
	virtual void on(UserCommand&, ADCClient*, string&) throw() {};
	// Called when a client sends a normal broadcast chat message
	// parm: ADCClient* = the client
	// parm: string = the message
	virtual void on(UserMessage&, ADCClient*, u_int32_t const, string&) throw() {};
	virtual void on(UserPrivateMessage&, ADCClient*, u_int32_t const, string&, string&) throw() {};

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
