// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_ACCOUNTS_H_
#define _INCLUDED_PLUGIN_ACCOUNTS_H_

#include "Plugin.h"

namespace qhub {

using namespace std;

class ADC;

class Accounts : public Plugin {
public:
	Accounts() throw() {};
	virtual ~Accounts() throw() {};

	virtual void on(Message m, ADC* client) throw() {
		switch(m) {
		case STARTED: fprintf(stderr, "Plugin: accounts: started\n"); break;
		case STOPPED: fprintf(stderr, "Plugin: accounts: stopped\n"); break;
		case LOGIN: onLogin(client); break;
		case INFO: onInfo(client); break;
		case AUTHENTICATED: onAuth(client); break;
		default: break;
		}
	}

	void onLogin(ADC* client) throw();
	void onInfo(ADC* client) throw();
	void onAuth(ADC* client) throw();
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_ACCOUNTS_H_
