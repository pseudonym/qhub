// vim:ts=4:sw=4:noet
#include "FsUtil.h"

#include "UserInfo.h"
#include "UserData.h"
#include "XmlTok.h"
#include "Logs.h"
#include "Settings.h"
#include "PluginManager.h"

using namespace std;
using namespace qhub;

/*
 * Plugin loader
 */
QHUB_GET_PLUGIN(FsUtil)


/*
 * Plugin details
 */

UserData::key_type FsUtil::idVirtualFs = "virtualfs";

bool FsUtil::load() throw()
{
	aliases.clear(); // clean old data
	XmlTok* p = Settings::instance()->getConfig("fsutil");
	if(p->findChild("aliases")) {
		p = p->getNextChild();
		aliasPrefix = p->getAttr("prefix");
		if(aliasPrefix.empty())
			aliasPrefix = "+";
		if(p->findChild("alias")) {
			XmlTok* tmp;
			while((tmp = p->getNextChild())) {
				aliases[tmp->getAttr("in")] = tmp->getAttr("out");
			}
		}
		p = p->getParent();
	}
	return true;
}

bool FsUtil::save() const throw()
{
	XmlTok* p = Settings::instance()->getConfig("fsutil");
	p->clear();
	p = p->addChild("aliases");
	p->setAttr("prefix", aliasPrefix);
	for(Aliases::const_iterator i = aliases.begin(); i != aliases.end(); ++i) {
		XmlTok* tmp = p->addChild("alias");
		tmp->setAttr("in", i->first);
		tmp->setAttr("out", i->second);
	}
	return true;
}

void FsUtil::initVFS() throw()
{
	assert(virtualfs->mkdir("/fsutil", this));
	assert(virtualfs->mknod("/fsutil/alias", this));
	assert(virtualfs->mknod("/fsutil/unalias", this));
}

void FsUtil::deinitVFS() throw()
{
	assert(virtualfs->rmdir("/fsutil"));
}

void FsUtil::on(PluginStarted&, Plugin* p) throw()
{
	if(p == this) {
		load();
		virtualfs = (VirtualFs*)Util::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin FsUtil: VirtualFs interface found.\n";
			initVFS();
		} else {
			Logs::err << "warning: Plugin FsUtil: VirtualFs interface not found.\n";
		}
		Logs::stat << "success: Plugin FsUtil: Started.\n";
	} else if(!virtualfs) {
		virtualfs = (VirtualFs*)Util::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin FsUtil: VirtualFs interface found.\n";
			initVFS();
		}
	}
}

void FsUtil::on(PluginStopped&, Plugin* p) throw()
{
	if(p == this) {
		if(virtualfs)
			deinitVFS();
		save();
		Logs::stat << "success: Plugin FsUtil: Stopped.\n";
	} else if(virtualfs && p == virtualfs) {
		Logs::err << "warning: Plugin FsUtil: VirtualFs interface disabled.\n";
		virtualfs = NULL;
	}
}

void FsUtil::on(ChDir, const string&, Client* c) throw()
{
	c->doPrivateMessage("This is the file system utilities section.");
}

void FsUtil::on(Help, const string&, Client* c) throw()
{
	c->doPrivateMessage(
			"The following commands are available to you:\n"
			"load\t\t\tloads settings\n"
			"save\t\t\tsaves settings\n"
			"alias [alias] [command]\tlist/add aliases\n"
			"unalias <alias>\t\tremoves an alias"
	);
}

void FsUtil::on(Exec, const string& cwd, Client* c, const StringList& arg) throw()
{
	assert(arg.size() >= 1);
	if(arg[0] == "load") {
		if(load()) {
			c->doPrivateMessage("Success: FsUtil settings reloaded.");
		} else {
			c->doPrivateMessage("Failure: Failed to reload FsUtil settings file.");
		}
	} else if(arg[0] == "save") {
		if(save()) {
			c->doPrivateMessage("Success: FsUtil settings file saved.");
		} else {
			c->doPrivateMessage("Failure: Failed to save FsUtil settings file.");
		}
	} else if(arg[0] == "alias") {
		if(arg.size() == 1) {
			string tmp = "Success: aliases, prefix = \"" + aliasPrefix + "\":\r\n";
			for(Aliases::const_iterator i = aliases.begin(); i != aliases.end(); ++i) {
				tmp += i->first + " = " + i->second + "\r\n";
			}
			c->doPrivateMessage(tmp);
		} else if(arg.size() == 2) {
			Aliases::const_iterator i = aliases.find(arg[1]);
			if(i != aliases.end())
				c->doPrivateMessage("Success: " + i->first + " = " + i->second);
			else
				c->doPrivateMessage("Failed: " + arg[1] + " undefined");
		} else if(arg.size() == 3) {
			aliases[arg[1]] = arg[2];
			c->doPrivateMessage("Success: alias added/modified");
		} else {
			c->doPrivateMessage("Syntax: alias [alias] [command]");
		}
	} else if(arg[0] == "unalias") {
		if(arg.size() == 2) {
			Aliases::iterator i = aliases.find(arg[1]);
			if(i != aliases.end()) {
				aliases.erase(i);
				c->doPrivateMessage("Success: alias removed");
			} else {
				c->doPrivateMessage("Failed: no such alias defined");
			}
		} else {
			c->doPrivateMessage("Syntax: unalias <nick>");
		}
	}
}

void FsUtil::on(UserCommand& a, Client* client, string& msg) throw()
{
	if(msg.compare(0, aliasPrefix.length(), aliasPrefix) == 0) {
		string::size_type i = msg.find(' ');
		if(i == string::npos)
			i = msg.length();
		Aliases::const_iterator j = aliases.find(msg.substr(aliasPrefix.length(), i - aliasPrefix.length()));
		if(j != aliases.end()) {
			msg.replace(0, i, j->second);
			a.setState(Plugin::MODIFIED);
		}
	}
}

void FsUtil::on(UserMessage& a, Client* c, Command&, string& msg) throw()
{
	if(msg.compare(0, aliasPrefix.length(), aliasPrefix) == 0) {
		string::size_type i = msg.find(' ');
		if(i == string::npos)
			i = msg.length();
		Aliases::const_iterator j = aliases.find(msg.substr(aliasPrefix.length(), i - aliasPrefix.length()));
		if(j != aliases.end()) {
			msg.replace(0, i, j->second);
			Plugin::UserCommand action;
			PluginManager::instance()->fire(action, c, msg);
			a.setState(Plugin::STOP);
		}
	}
}

void FsUtil::on(UserPrivateMessage& a, Client* c, Command&, string& msg, sid_type) throw()
{
	if(msg.compare(0, aliasPrefix.length(), aliasPrefix) == 0) {
		string::size_type i = msg.find(' ');
		if(i == string::npos)
			i = msg.length();
		Aliases::const_iterator j = aliases.find(msg.substr(aliasPrefix.length(), i - aliasPrefix.length()));
		if(j != aliases.end()) {
			msg.replace(0, i, j->second);
			Plugin::UserCommand action;
			PluginManager::instance()->fire(action, c, msg);
			a.setState(Plugin::STOP);
		}
	}
}
