// vim:ts=4:sw=4:noet
#ifndef QHUB_PLUGIN_VIRTUALFS_H
#define QHUB_PLUGIN_VIRTUALFS_H

#include "Plugin.h"
#include "UserData.h"
#include "Util.h"

namespace qhub {

class Dir;
class Client;

struct VirtualFsListener {
	template<int I> struct X { enum { TYPE = I }; };

	typedef X<0> ChDir;
	typedef X<1> Help;
	typedef X<2> Exec;

	virtual void on(ChDir, const std::string&, Client*) throw() {}
	virtual void on(Help, const std::string&, Client*) throw() {}
	virtual void on(Exec, const std::string&, Client*, const StringList&) throw() {}

	virtual ~VirtualFsListener() {} // to make the compiler shut up
};

class VirtualFs : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// voidPtr
	static UserData::key_type idVirtualPath;	// string

	VirtualFs() throw() : Plugin("virtualfs") {};
	virtual ~VirtualFs() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
	virtual void on(UserCommand&, Client*, const std::string&) throw();

	virtual void on(Help, const std::string&, Client*) throw();

	bool mkdir(std::string const& dir, VirtualFsListener* plugin) throw();
	bool mknod(std::string const& node, VirtualFsListener* plugin) throw();
	bool rmdir(std::string const& dir) throw();
	bool rmnod(std::string const& dir) throw();

private:
	Dir* root;

	void init() throw();
	void deinit() throw();
};

} //namespace qhub

#endif // QHUB_PLUGIN_VIRTUALFS_H
