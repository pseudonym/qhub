// vim:ts=4:sw=4:noet
#include "Loader.h"

#include "Client.h"
#include "Logs.h"
#include "PluginManager.h"
#include "Settings.h"
#include "Util.h"
#include "XmlTok.h"

#include <fstream>

using namespace std;
using namespace qhub;

/*
 * Plugin loader/unloader
 */
QHUB_PLUGIN(Loader)

/*
 * Plugin details
 */
UserData::key_type Loader::idVirtualFs = "virtualfs";

int Loader::load() throw()
{
	try {
		string fn = Settings::instance()->getConfigDir() + "/plugins.xml";
		ifstream is(fn.c_str());
		XmlTok root(is);
		if(root.getName() != "plugins")
			throw runtime_error("invalid root element");

		root.findChild("plugin");
		int success = 0;
		int failure = 0;
		while(XmlTok* tmp = root.getNextChild()) {
			string const& name = tmp->getData();
			// check if we are already added; don't want inf-recurse
			if(!PluginManager::instance()->has(name) && name != "loader") {
				Logs::stat << "\nloading plugin \"" << name << "\"\n";
				if(PluginManager::instance()->open(name)) {
					success++;
				} else {
					failure++;
				}
			}
		}
		return failure == 0 ? success : -failure;
	} catch(const exception& e) {
		Logs::err << "Loader: could not load plugins.xml: " << e.what() << endl;
		return 0;
	}
}

bool Loader::save() const throw()
{
	try {
		XmlTok root("plugins");
		for(PluginManager::iterator i = PluginManager::instance()->begin(); i != PluginManager::instance()->end(); ++i) {
			XmlTok* tmp = root.addChild("plugin");
			tmp->setData((*i)->getId());
		}
		string fn = Settings::instance()->getConfigDir() + "/plugins.xml";
		ofstream os(fn.c_str());
		root.save(os);

		return true;
	} catch(const exception& e) {
		Logs::err << "Loader: could not save plugins.xml: " << e.what() << endl;
		return false;
	}
}

void Loader::initVFS() throw()
{
	assert(virtualfs->mkdir("/plugins", this));
	assert(virtualfs->mknod("/plugins/load", this));
	assert(virtualfs->mknod("/plugins/save", this));
	assert(virtualfs->mknod("/plugins/list", this));
	assert(virtualfs->mknod("/plugins/insert", this));
	assert(virtualfs->mknod("/plugins/remove", this));
	assert(virtualfs->mknod("/plugins/removeall", this));
	assert(virtualfs->mknod("/plugins/restart", this));
}

void Loader::deinitVFS() throw()
{
	assert(virtualfs->rmdir("/plugins"));
}

void Loader::on(PluginStarted&, Plugin* p) throw()
{
	if(p == this) {
		load();
		virtualfs = (VirtualFs*)Util::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin Loader: VirtualFs interface found.\n";
			initVFS();
		} else {
			Logs::err << "warning: Plugin Loader: VirtualFs interface not found.\n";
		}
		Logs::stat << "success: Plugin Loader: Started.\n";
	} else if(!virtualfs) {
		virtualfs = (VirtualFs*)Util::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin Loader: VirtualFs interface found.\n";
			initVFS();
		}
	}
}

void Loader::on(PluginStopped&, Plugin* p) throw()
{
	if(p == this) {
		if(virtualfs)
			deinitVFS();
		Logs::stat << "success: Plugin Loader: Stopped.\n";
	} else if(virtualfs && p == virtualfs) {
		Logs::err << "warning: Plugin Loader: VirtualFs interface disabled.\n";
		virtualfs = NULL;
	}
}

void Loader::on(ChDir, const string&, Client* c) throw()
{
	c->doPrivateMessage("This is the plugins section. Load and unload plugins here.");
}

void Loader::on(Help, const string& cwd, Client* c) throw()
{
	assert(cwd == "/plugins/");
	c->doPrivateMessage(
			"The following commands are available to you:\r\n"
			"load\t\t\tloads the plugins in the settings file (done automatically at startup)\r\n"
			"save\t\t\tsaves the plugin load order to disk (must be done manually)\r\n"
			"list\t\t\tshows the list of loaded plugins\r\n"
			"insert <plugin> [insertbefore]\tloads the plugin in the specified position\r\n"
			"remove <plugin>\t\tunloads the plugin\r\n"
			"removeall\t\t\tunloads every plugin except this one and virtualfs\r\n"
			"restart <plugin>\t\tunloads and loads the plugin (e.g. to reload settings)\r\n"
	);
}

void Loader::on(Exec, const string& cwd, Client* c, const StringList& arg) throw()
{
	assert(arg.size() >= 1);
	if(arg[0] == "load") {
		int n = 0;
		if((n = load()) >= 0) {
			c->doPrivateMessage("Success: " + Util::toString(n) + " plugin(s) loaded from disk.");
		} else {
			c->doPrivateMessage("Failure: " + Util::toString(-n) + " plugin(s) failed to load.");
		}
	} else if(arg[0] == "save") {
		if(save()) {
			c->doPrivateMessage("Success: Plugin load order saved to disk.");
		} else {
			c->doPrivateMessage("Failure: Saving plugin load order to disk failed.");
		}
	} else if(arg[0] == "list") {
		string ret = "Success: Plugins loader:\r\n";
		for(PluginManager::iterator i = PluginManager::instance()->begin();
				i != PluginManager::instance()->end(); ++i) {
			ret += (*i)->getId() + ((*i)->getId() == "virtualfs" ? " (cannot be unloaded)\r\n" : "\r\n");
		}
		c->doPrivateMessage(ret);
	} else if(arg[0] == "insert") {
		if(arg.size() == 3 && !PluginManager::instance()->has(arg[1]) && PluginManager::instance()->open(arg[1], arg[2])) {
			c->doPrivateMessage("Success: Plugin loaded.");
		} else if(arg.size() == 2 && !PluginManager::instance()->has(arg[1]) && PluginManager::instance()->open(arg[1])) {
			c->doPrivateMessage("Success: Plugin loaded.");
		} else if(arg.size() == 2 || arg.size() == 3) {
			c->doPrivateMessage("Failure: Plugin failed to load.");
		} else {
			c->doPrivateMessage("Syntax: insert <plugin> [insertbefore]");
		}
	} else if(arg[0] == "restart") {
		if(arg.size() == 2) {
			PluginManager::iterator j = PluginManager::instance()->end();
			string next;
			for(PluginManager::iterator i = PluginManager::instance()->begin();
					i != PluginManager::instance()->end(); ++i) {
				if((*i)->getId() == arg[1]) {
					j = i++;
					if(i != PluginManager::instance()->end())
						next = (*i)->getId();
					break;
				}
			}
			if(j != PluginManager::instance()->end()) {
				PluginManager::instance()->remove(arg[1]);
				if(PluginManager::instance()->open(arg[1], next)) {
					c->doPrivateMessage("Success: Plugin restarted.");
				} else {
					c->doPrivateMessage("Failure: Plugin failed to restart properly.");
				}
			} else {
				c->doPrivateMessage("Failure: Plugin not loaded.");
			}
		} else {
			c->doPrivateMessage("Syntax: restart <plugin>");
		}
	} else if(arg[0] == "removeall") {
		string ret = "Success: Unloaded the following plugins:\r\n";
		while(true) {
			PluginManager::iterator i; // Iterator is invalidated on removeModule
			for(i = PluginManager::instance()->begin(); i != PluginManager::instance()->end(); ++i) {
				// Don't unload ourself and dependencies
				if(*i == this || (virtualfs && *i == virtualfs))
					continue;
				ret += (*i)->getId() + "\r\n";
				PluginManager::instance()->remove((*i)->getId());
				break;
			}
			if(i == PluginManager::instance()->end())
				break;
		}
		c->doPrivateMessage(ret);
	} else if(arg[0] == "remove") {
		string ret = "Success: Unloaded the following plugins:\r\n";
		if(arg.size() != 1) {
			for(unsigned i = 1; i < arg.size(); ++i) {
				// can't remove virtualfs, we're being called by it
				if(arg[i] != "virtualfs" && PluginManager::instance()->remove(arg[i])) {
					ret += arg[i] + "\r\n";
				}
			}
			c->doPrivateMessage(ret);
		} else {
			c->doPrivateMessage("Syntax: remove <plugin1> ... <pluginN>");
		}
	}
}
