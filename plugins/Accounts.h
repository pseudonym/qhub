// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_ACCOUNTS_H_
#define _INCLUDED_PLUGIN_ACCOUNTS_H_

#include "../Plugin.h"
#include "../compat_hash_map.h"
#include "../UserData.h"

using namespace std;

namespace qhub {

class ADCClient;
class VirtualFs;

class Accounts : public Plugin {
public:
	static UserData::Key idUserLevel;	// int
	static UserData::Key idVirtualFs;	// void* (Plugin*)
	
	Accounts() throw() : virtualfs(NULL) {};
	virtual ~Accounts() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(PluginMessage&, Plugin*, void*) throw();
	virtual void on(ClientLogin&, ADCClient*) throw();
	virtual void on(ClientInfo&, ADCClient*, UserInfo&) throw();
	virtual void on(UserConnected&, ADCClient*) throw();

private:
	bool load() throw();
	bool save() const throw();
	void initVFS() throw();
	void deinitVFS() throw();
	
	VirtualFs* virtualfs;
	typedef hash_map<string, pair<string,int> > Users; // too simple.. works for now
	Users users;
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_ACCOUNTS_H_
