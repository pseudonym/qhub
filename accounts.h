// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_ACCOUNTS_H_
#define _INCLUDED_PLUGIN_ACCOUNTS_H_

#include "Plugin.h"
#include "UserData.h"

using namespace std;

namespace qhub {

class ADCClient;

class Accounts : public Plugin {
public:
	static UserData::Key idUserLevel;	// int
	static UserData::Key idVirtualPath;	// string
	
	Accounts() throw() {};
	virtual ~Accounts() throw() {};

	virtual void on(Message m, int& a, ADCClient* client, string const& msg) throw() {
		switch(m) {
		case STARTED: load(); fprintf(stderr, "Plugin: accounts: started\n"); break;
		case STOPPED: save(); fprintf(stderr, "Plugin: accounts: stopped\n"); break;
		case LOGIN: onLogin(a, client); break;
		case INFO: onInfo(a, client); break;
		case AUTHENTICATED: onAuth(a, client); break;
		case COMMAND: onCommand(a, client, msg); break;
		default: break;
		}
	}

private:
	bool load() throw();
	bool save() const throw();
	
	void onLogin(int& a, ADCClient* client) throw();
	void onInfo(int& a, ADCClient* client) throw();
	void onAuth(int& a, ADCClient* client) throw();
	void onCommand(int& a, ADCClient* client, string const& msg) throw();

	typedef hash_map<string, string> Users; // too simple.. works for now
	Users users;
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_ACCOUNTS_H_
