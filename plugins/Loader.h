// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_LOADER_H_
#define _INCLUDED_PLUGIN_LOADER_H_

#include "Plugin.h"
#include "UserData.h"
#include "ADCClient.h"

#include "VirtualFs.h"

using namespace std;

namespace qhub {

class Loader : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// voidPtr (Plugin*)
	
	Loader() throw() {};
	virtual ~Loader() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
//	virtual void on(PluginMessage&, Plugin*, void*) throw();

	virtual void on(ChDir, const string&, Client*) throw();
	virtual void on(Help, const string&, Client*) throw();
	virtual void on(Exec, const string&, Client*, const StringList&) throw();

private:
	VirtualFs* virtualfs;
	
	int load() throw();
	bool save() const throw();
	void initVFS() throw();
	void deinitVFS() throw();
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_LOADER_H_
