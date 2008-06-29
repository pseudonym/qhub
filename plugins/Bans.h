// vim:ts=4:sw=4:noet
#ifndef QHUB_PLUGIN_BANS_H
#define QHUB_PLUGIN_BANS_H

#include "VirtualFs.h"

#include "compat_hashtable.h"
#include "Plugin.h"
#include "UserData.h"

namespace qhub {

class Bans : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// void* (Plugin*)
	
	Bans() throw() : Plugin("bans"), virtualfs(NULL) {};
	virtual ~Bans() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(ClientLogin&, Client*) throw();

	virtual void on(ChDir, const std::string&, Client*) throw();
	virtual void on(Help, const std::string&, Client*) throw();
	virtual void on(Exec, const std::string&, Client*, const StringList&) throw();

private:
	bool load() throw();
	bool save() throw();
	void initVFS() throw();
	void deinitVFS() throw();
	static time_t parseTime(const std::string&);
	struct BanInfo;	// silly forward declarations...
	static void killUser(Client*, const BanInfo&) throw();

	VirtualFs* virtualfs;

	struct BanInfo {
		BanInfo(time_t t, const std::string& b, const std::string& r) throw() : timeout(t), banner(b), reason(r) {}
		time_t timeout;
		std::string banner;
		std::string reason;
	};


	typedef std::hash_map<std::string,BanInfo> BanList;
	BanList ipBans;
	BanList nickBans;
	BanList cidBans;
};

} //namespace qhub

#endif // QHUB_PLUGIN_BANS_H
