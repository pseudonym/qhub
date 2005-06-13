// vim:ts=4:sw=4:noet
#include "Accounts.h"

#include "../ADCClient.h"
#include "../UserInfo.h"
#include "../UserData.h"
#include "../XmlTok.h"
#include "../Logs.h"
#include "VirtualFs.h"

using namespace qhub;

/*
 * Plugin loader
 */

extern "C" {
	void* getPlugin() { return new Accounts(); }
} //extern "C"



/*
 * Plugin details
 */

UserData::Key Accounts::idUserLevel = UserData::toKey("userlevel");
UserData::Key Accounts::idVirtualFs = UserData::toKey("virtualfs");

bool Accounts::load() throw()
{
	bool success = false;
	XmlTok root;
	if(root.load(CONFIGDIR "/accounts.xml")) {
		XmlTok* p = &root;
		if(p->findChild("accounts")) {
			users.clear(); // clean old users
			success = true;
			p = p->getNextChild();
			if(p->findChild("user")) {
				XmlTok* tmp;
				while((tmp = p->getNextChild())) {
					int lvl = Util::toInt(tmp->getAttr("level"));
					users[tmp->getAttr("nick")] =
							make_pair(tmp->getAttr("password"),lvl==0?1:lvl);
				}
			}
			p = p->getParent();
		}
	}
	return success;
}

bool Accounts::save() const throw()
{
	XmlTok root;
	XmlTok* p = &root;
	p = p->addChild("accounts");
	for(Users::const_iterator i = users.begin(); i != users.end(); ++i) {
		XmlTok* tmp = p->addChild("user");
		tmp->setAttr("nick", i->first);
		tmp->setAttr("password", i->second.first);
		tmp->setAttr("level", Util::toString(i->second.second));
	}
	p = p->getParent();
	return root.save(CONFIGDIR "/accounts.xml");
}

void Accounts::initVFS() throw()
{
	virtualfs->mkdir("/accounts", this);
	virtualfs->mknod("/accounts/load", this);
	virtualfs->mknod("/accounts/save", this);
	virtualfs->mknod("/accounts/add", this);
	virtualfs->mknod("/accounts/del", this);
	virtualfs->mknod("/accounts/list", this);
	//assert(virtualfs->mknod("/accounts/show", this));
}

void Accounts::deinitVFS() throw()
{
	virtualfs->rmdir("/accounts");
}

void Accounts::on(PluginStarted&, Plugin* p) throw()
{
	if(p == this) {
		load();
		virtualfs = (VirtualFs*)Plugin::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin Accounts: VirtualFs interface found.\n";
			initVFS();
		} else {
			Logs::err << "warning: Plugin Accounts: VirtualFs interface not found.\n";
		}
		Logs::stat << "success: Plugin Accounts: Started.\n";
	} else if(!virtualfs) {
		virtualfs = (VirtualFs*)Plugin::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin Accounts: VirtualFs interface found.\n";
			initVFS();
		}
	}
}

void Accounts::on(PluginStopped&, Plugin* p) throw()
{
	if(p == this) {
		if(virtualfs)
			deinitVFS();
		save();
		Logs::stat << "success: Plugin Accounts: Stopped.\n";
	} else if(virtualfs && p == virtualfs) {
		Logs::err << "warning: Plugin Accounts: VirtualFs interface disabled.\n";
		virtualfs = NULL;
	}
}

void Accounts::on(ClientLogin&, ADCClient* client) throw()
{
	UserInfo* inf = client->getUserInfo();
	Users::const_iterator i = users.find(inf->getNick());
	if(i != users.end()) {
		UserData* data = client->getUserData();
		data->setInt(idUserLevel, i->second.second);
		client->doAskPassword(i->second.first);
	}

	// Remove the OP flag
	inf->del(UIID('O','P'));
}

void Accounts::on(ClientInfo& a, ADCClient* client, UserInfo& inf) throw()
{
	UserData* data = client->getUserData();

	// Check user nick
	if(inf.has(UIID('N','I'))) {
		// don't allow registered users to change nick
		if(data->getInt(idUserLevel)) {
			inf.del(UIID('N','I'));
			a.setState(Plugin::MODIFIED);
		// don't allow users to change to a registered nick
		} else if(users.find(inf.getNick()) != users.end()) {
			inf.del(UIID('N','I'));
			a.setState(Plugin::MODIFIED);
		}
	}

	// Remove the OP flag
	if(inf.del(UIID('O','P')))
		a.setState(Plugin::MODIFIED);
}

void Accounts::on(UserConnected&, ADCClient* client) throw()
{
	UserData* data = client->getUserData();
	if(data->getInt(idUserLevel) >= 3) {
		client->getUserInfo()->set(UIID('O','P'), "1");
	}
}

void Accounts::on(PluginMessage&, Plugin* p, void* d) throw()
{
	if(virtualfs && p == virtualfs) {
		VirtualFs::Message* m = static_cast<VirtualFs::Message*>(d);
		assert(m);
		if(m->type == VirtualFs::Message::CHDIR) {
			m->reply("This is the accounts section. Create and remove accounts and properties here.");
		} else if(m->type == VirtualFs::Message::HELP) {
			if(m->cwd == "/accounts/") {
				m->reply(
				    "The following commands are available to you:\n"
				    "load\t\t\t\tloads the users file from disk\n"
				    "save\t\t\t\tsaves the users file to disk\n"
				    "add <user> <password> <level>\tadds a user\n"
				    "del <user>\t\t\tremoves a user\n"
				    "list [wildcard]\t\t\tshows the list of registered users"
				    //"show <user>\t\tshows information about a user"
				);
			}
		} else if(m->type == VirtualFs::Message::EXEC) {
			assert(m->arg.size() >= 1);
			if(m->arg[0] == "load") {
				if(load()) {
					m->reply("Success: User accounts file reloaded.");
				} else {
					m->reply("Failure: Failed to reload user accounts file.");
				}
			} else if(m->arg[0] == "save") {
				if(save()) {
					m->reply("Success: User accounts file saved.");
				} else {
					m->reply("Failure: Failed to save user accounts file.");
				}
			} else if(m->arg[0] == "add") {
				if(m->arg.size() == 4) {
					users[m->arg[1]] = make_pair(m->arg[2], Util::toInt(m->arg[3]));
					m->reply("Success: User created/updated.");
				} else if(m->arg.size() == 3) {
					users[m->arg[1]] = make_pair(m->arg[2], 1);
					m->reply("Success: User created/updated.");
					// check if user online..
					// add OP1 if so
				} else {
					m->reply("Syntax: add <nick> <password> <level=1>");
				}
			} else if(m->arg[0] == "del") {
				if(m->arg.size() == 2) {
					Users::iterator i = users.find(m->arg[1]);
					if(i != users.end()) {
						users.erase(i);
						m->reply("Success: User deleted.");
					} else {
						m->reply("Failure: User not found.");
					}
				} else {
					m->reply("Syntax: del <nick>");
				}
			} else if(m->arg[0] == "list") {
				string ret = "Success: Registered users:";
				for(Users::iterator i = users.begin(); i != users.end(); ++i) {
					ret += '\n';
					ret += i->first;
				}
				m->reply(ret);
			}
		}
	}
}
