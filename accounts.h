// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_ACCOUNTS_H_
#define _INCLUDED_PLUGIN_ACCOUNTS_H_

#include "Plugin.h"

using namespace std;

namespace qhub {

class ADC;

class Accounts : public Plugin {
public:
	Accounts() throw() {};
	virtual ~Accounts() throw() {};

	virtual void on(Message m, ADC* client, string const& msg) throw() {
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
	
	void onLogin(ADC* client) throw();
	void onInfo(ADC* client) throw();
	void onAuth(ADC* client) throw();
	void onCommand(ADC* client, string const& msg) throw();
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_ACCOUNTS_H_
