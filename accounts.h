// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_ACCOUNTS_H_
#define _INCLUDED_PLUGIN_ACCOUNTS_H_

#include "Plugin.h"

using namespace std;

namespace qhub {

class ADCClient;

class Accounts : public Plugin {
public:
	static u_int32_t idUserLevel;
	
	Accounts() throw() {};
	virtual ~Accounts() throw() {};

	virtual void on(Message m, ADCClient* client, string const& msg) throw() {
		switch(m) {
		case STARTED: fprintf(stderr, "Plugin: accounts: started\n"); break;
		case STOPPED: fprintf(stderr, "Plugin: accounts: stopped\n"); break;
		case LOGIN: onLogin(client); break;
		case INFO: onInfo(client); break;
		case AUTHENTICATED: onAuth(client); break;
		case COMMAND: onCommand(client, msg); break;
		default: break;
		}
	}

private:
	void load() throw();
	void save() throw();
	
	void onLogin(ADCClient* client) throw();
	void onInfo(ADCClient* client) throw();
	void onAuth(ADCClient* client) throw();
	void onCommand(ADCClient* client, string const& msg) throw();
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_ACCOUNTS_H_
