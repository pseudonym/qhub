// vim:ts=4:sw=4:noet
#ifndef QHUB_PLUGIN_ACCOUNTS_H
#define QHUB_PLUGIN_ACCOUNTS_H

#include "VirtualFs.h"

#include "fast_map.h"
#include "Plugin.h"
#include "UserData.h"

#include <string>
#include <utility>

namespace qhub {

class Accounts : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idUserLevel;	// int
	static UserData::key_type idVirtualFs;	// void* (Plugin*)
	
	Accounts() throw() : Plugin("accounts"), virtualfs(NULL) {};
	virtual ~Accounts() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(ClientLogin&, Client*) throw();
	virtual void on(ClientInfo&, Client*, UserInfo&) throw();
	virtual void on(UserConnected&, Client*) throw();

	virtual void on(ChDir, const std::string&, Client*) throw();
	virtual void on(Help, const std::string&, Client*) throw();
	virtual void on(Exec, const std::string&, Client*, const StringList&) throw();

private:
	bool load() throw();
	bool save() const throw();
	void initVFS() throw();
	void deinitVFS() throw();
	
	VirtualFs* virtualfs;
	// too simple.. works for now
	typedef QHUB_FAST_MAP<std::string, std::pair<std::string,int> > Users;
	Users users;
};

} //namespace qhub

#endif // QHUB_PLUGIN_ACCOUNTS_H
