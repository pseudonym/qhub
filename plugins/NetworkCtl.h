// vim:ts=4:sw=4:noet
#ifndef QHUB_PLUGIN_NETWORK_CTL_H
#define QHUB_PLUGIN_NETWORK_CTL_H

#include <set>

#include "Plugin.h"

#include "VirtualFs.h"

namespace qhub {

class NetworkCtl : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// void* (Plugin*)

	NetworkCtl() throw() : Plugin("networkctl"), virtualfs(NULL) {};
	virtual ~NetworkCtl() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(InterConnected&, InterHub*) throw();
	virtual void on(InterDisconnected&, InterHub*) throw();

	virtual void on(ChDir, const std::string&, Client*) throw();
	virtual void on(Help, const std::string&, Client*) throw();
	virtual void on(Exec, const std::string&, Client*, const StringList&) throw();

private:
	void initVFS() throw();
	void deinitVFS() throw();

	VirtualFs* virtualfs;

	typedef std::set<InterHub*> Interhubs;
	Interhubs interhubs;
};

} //namespace qhub

#endif // QHUB_PLUGIN_NETWORK_CTL_H
