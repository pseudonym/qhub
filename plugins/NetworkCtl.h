// vim:ts=4:sw=4:noet
#ifndef NETWORK_CTL_H
#define NETWORK_CTL_H

#include <set>

#include "Plugin.h"

#include "VirtualFs.h"

using namespace std;

namespace qhub {

class NetworkCtl : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// void* (Plugin*)
	
	NetworkCtl() throw() : virtualfs(NULL) {};
	virtual ~NetworkCtl() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(InterConnected&, InterHub*) throw();
	virtual void on(InterDisconnected&, InterHub*) throw();

	virtual void on(ChDir, const string&, Client*) throw();
	virtual void on(Help, const string&, Client*) throw();
	virtual void on(Exec, const string&, Client*, const StringList&) throw();

private:
	void initVFS() throw();
	void deinitVFS() throw();

	VirtualFs* virtualfs;

	typedef set<InterHub*> Interhubs;
	Interhubs interhubs;
};

} //namespace qhub

#endif //NETWORK_CTL_H
