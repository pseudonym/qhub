// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_ACCOUNTS_H_
#define _INCLUDED_PLUGIN_ACCOUNTS_H_

#include "Plugin.h"
#include "compat_hash_map.h"
#include "UserData.h"

#include "VirtualFs.h"

using namespace std;

namespace qhub {

class Accounts : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idUserLevel;	// int
	static UserData::key_type idVirtualFs;	// void* (Plugin*)
	
	Accounts() throw() : virtualfs(NULL) {};
	virtual ~Accounts() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(ClientLogin&, Client*) throw();
	virtual void on(ClientInfo&, Client*, UserInfo&) throw();
	virtual void on(UserConnected&, Client*) throw();

	virtual void on(ChDir, const string&, Client*) throw();
	virtual void on(Help, const string&, Client*) throw();
	virtual void on(Exec, const string&, Client*, const StringList&) throw();

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
