// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_BANS_H_
#define _INCLUDED_PLUGIN_BANS_H_

#include "Plugin.h"
#include "compat_hashtable.h"
#include "UserData.h"

#include "VirtualFs.h"

using namespace std;

namespace qhub {

class Bans : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// void* (Plugin*)
	
	Bans() throw() : Plugin("bans"), virtualfs(NULL) {};
	virtual ~Bans() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(ClientLogin&, Client*) throw();

	virtual void on(ChDir, const string&, Client*) throw();
	virtual void on(Help, const string&, Client*) throw();
	virtual void on(Exec, const string&, Client*, const StringList&) throw();

private:
	bool load() throw();
	bool save() throw();
	void initVFS() throw();
	void deinitVFS() throw();
	static time_t parseTime(const string&);
	struct BanInfo;	// silly forward declarations...
	static void killUser(Client*, const BanInfo&) throw();

	VirtualFs* virtualfs;

	struct BanInfo {
		BanInfo(time_t t, const string& b, const string& r) throw() : timeout(t), banner(b), reason(r) {}
		time_t timeout;
		string banner;
		string reason;
	};


	typedef hash_map<string,BanInfo> BanList;
	BanList ipBans;
	BanList nickBans;
	BanList cidBans;
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_BANS_H_
