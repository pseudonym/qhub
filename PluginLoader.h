// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_LOADER_H_
#define _INCLUDED_PLUGIN_LOADER_H_

#include "Plugin.h"
#include "UserData.h"

using namespace std;

namespace qhub {

class VirtualFs;

class Loader : public Plugin {
public:
	static UserData::Key idVirtualFs;	// voidPtr (Plugin*)
	
	Loader() throw() {};
	virtual ~Loader() throw() {};

	virtual void on(PluginStarted, Plugin*) throw();
	virtual void on(PluginStopped, Plugin*) throw();
	virtual void on(PluginMessage, Plugin*, void*) throw();

private:
	VirtualFs* virtualfs;
	
	int load() throw();
	bool save() const throw();
	void initVFS() throw();
	void deinitVFS() throw();
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_LOADER_H_
