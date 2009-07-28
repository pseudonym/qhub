// vim:ts=4:sw=4:noet
#ifndef QHUB_PLUGIN_FSUTIL_H
#define QHUB_PLUGIN_FSUTIL_H

#include "VirtualFs.h"

#include "fast_map.h"
#include "Client.h"
#include "Plugin.h"
#include "UserData.h"

namespace qhub {

class FsUtil : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// void* (Plugin*)

	FsUtil() throw() : Plugin("fsutil"), virtualfs(NULL) {};
	virtual ~FsUtil() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(UserCommand&, Client*, std::string&) throw();
	virtual void on(UserMessage&, Client*, Command&, std::string&) throw();
	virtual void on(UserPrivateMessage&, Client*, Command&, std::string&, sid_type) throw();

	virtual void on(ChDir, const std::string&, Client*) throw();
	virtual void on(Help, const std::string&, Client*) throw();
	virtual void on(Exec, const std::string&, Client*, const StringList&) throw();

private:
	bool load() throw();
	bool save() const throw();
	void initVFS() throw();
	void deinitVFS() throw();

	VirtualFs* virtualfs;
	typedef QHUB_FAST_MAP<std::string, std::string> Aliases;
	Aliases aliases;
	std::string aliasPrefix;
};

} //namespace qhub

#endif // QHUB_PLUGIN_FSUTIL_H
