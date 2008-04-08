// vim:ts=4:sw=4:noet
#ifndef QHUB_PLUGIN_LOADER_H
#define QHUB_PLUGIN_LOADER_H

#include "Plugin.h"
#include "UserData.h"
#include "Client.h"

#include "VirtualFs.h"

namespace qhub {

class Loader : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// voidPtr (Plugin*)
	
	Loader() throw() : Plugin("loader") {}
	virtual ~Loader() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();

	virtual void on(ChDir, const std::string&, Client*) throw();
	virtual void on(Help, const std::string&, Client*) throw();
	virtual void on(Exec, const std::string&, Client*, const StringList&) throw();

private:
	VirtualFs* virtualfs;
	
	int load() throw();
	bool save() const throw();
	void initVFS() throw();
	void deinitVFS() throw();
};

} //namespace qhub

#endif // QHUB_PLUGIN_LOADER_H
