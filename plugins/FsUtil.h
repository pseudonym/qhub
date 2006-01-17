// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_FSUTIL_H_
#define _INCLUDED_PLUGIN_FSUTIL_H_

#include "../Plugin.h"
#include "../UserData.h"
#include "../ADCClient.h"
#include "VirtualFs.h"

using namespace std;

namespace qhub {

class FsUtil : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// void* (Plugin*)

	FsUtil() throw() : virtualfs(NULL) {};
	virtual ~FsUtil() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
//	virtual void on(PluginMessage&, Plugin*, void*) throw();
	virtual void on(UserCommand&, Client*, string&) throw();
	virtual void on(UserMessage&, Client*, u_int32_t const, string&) throw();
	virtual void on(UserPrivateMessage&, Client*, u_int32_t const, string&, string&) throw();

	virtual void on(ChDir, const string&, Client*) throw();
	virtual void on(Help, const string&, Client*) throw();
	virtual void on(Exec, const string&, Client*, const StringList&) throw();

private:
	bool load() throw();
	bool save() const throw();
	void initVFS() throw();
	void deinitVFS() throw();
	
	VirtualFs* virtualfs;
	typedef hash_map<string, string> Aliases;
	Aliases aliases;
	string aliasPrefix;
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_FSUTIL_H_
