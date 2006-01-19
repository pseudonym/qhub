// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_PLUGIN_VIRTUALFS_H_
#define _INCLUDED_PLUGIN_VIRTUALFS_H_

#include "Plugin.h"
#include "UserData.h"
#include "Speaker.h"
#include "ADCClient.h"

using namespace std;

namespace qhub {

class Dir;

struct VirtualFsListener {
	template<int I> struct X { enum { TYPE = I }; };

	typedef X<0> ChDir;
	typedef X<1> Help;
	typedef X<2> Exec;

	virtual void on(ChDir, const string&, Client*) throw() {}
	virtual void on(Help, const string&, Client*) throw() {}
	virtual void on(Exec, const string&, Client*, const StringList&) throw() {}
};

class VirtualFs : public Plugin, public VirtualFsListener {
public:
	static UserData::key_type idVirtualFs;	// voidPtr
	static UserData::key_type idVirtualPath;	// string

	/*
	 * IPC structure
	 *
	struct Message {
		enum Type {
			CHDIR,
			HELP,
			EXEC
		};
		Message(Type t, string const& i, ADCClient* c, StringList const& a = Util::emptyStringList) throw()
		: type(t), cwd(i), client(c), arg(a) {};
		void reply(string const& msg) throw();
		Type const type;
		string const cwd;
		ADCClient* const client;
		StringList const& arg;
	};*/
	
	VirtualFs() throw() {};
	virtual ~VirtualFs() throw() {};

	virtual void on(PluginStarted&, Plugin*) throw();
	virtual void on(PluginStopped&, Plugin*) throw();
//	virtual void on(PluginMessage&, Plugin*, void*) throw();
	virtual void on(UserCommand&, ADCClient*, string&) throw();

	virtual void on(Help, const string&, Client*) throw();

	bool mkdir(string const& dir, VirtualFsListener* plugin) throw();
	bool mknod(string const& node, VirtualFsListener* plugin) throw();
	bool rmdir(string const& dir) throw();
	bool rmnod(string const& dir) throw();

private:
	Dir* root;

	void init() throw();
	void deinit() throw();
};

} //namespace qhub

#endif //_INCLUDED_PLUGIN_VIRTUALFS_H_
