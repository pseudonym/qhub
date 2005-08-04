// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_BANS_H_
#define _INCLUDED_PLUGIN_BANS_H_

#include <boost/shared_ptr.hpp>

#include "../Plugin.h"
#include "../compat_hash_map.h"
#include "../UserData.h"

using namespace std;

namespace qhub {

class ADCClient;
class VirtualFs;

class Bans : public Plugin {
public:
	static UserData::Key idVirtualFs;	// void* (Plugin*)
	
	Bans() throw() : virtualfs(NULL) {};
	virtual ~Bans() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(PluginMessage&, Plugin*, void*) throw();
	virtual void on(ClientLogin&, ADCClient*) throw();

private:
	bool load() throw();
	bool save() throw();
	void initVFS() throw();
	void deinitVFS() throw();
	static time_t parseTime(const string&);
	struct BanInfo;	// silly forward declarations...
	static void killUser(ADCClient*, const BanInfo&) throw();

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
