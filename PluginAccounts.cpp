// vim:ts=4:sw=4:noet
#include "PluginAccounts.h"

#include "ADCClient.h"
#include "ADCInf.h"
#include "UserData.h"
#include "XmlTok.h"
#include "PluginVirtualFs.h"

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
	if(root.load("etc/qhub/accounts.xml")) {
		XmlTok* p = &root;
		if(p->findChild("accounts")) {
			users.clear(); // clean old users
			success = true;
			p = p->getNextChild();
			if(p->findChild("user")) {
				XmlTok* tmp;
				while((tmp = p->getNextChild())) {
					users[tmp->getAttr("nick")] = tmp->getAttr("password");
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
		tmp->setAttr("password", i->second);
	}
	p = p->getParent();
	return root.save("etc/qhub/accounts.xml");
}

void Accounts::initVFS() throw()
{
	assert(virtualfs->mkdir("/accounts", this));
	assert(virtualfs->mknod("/accounts/load", this));
	assert(virtualfs->mknod("/accounts/save", this));
	assert(virtualfs->mknod("/accounts/add", this));
	assert(virtualfs->mknod("/accounts/del", this));
	assert(virtualfs->mknod("/accounts/list", this));
	assert(virtualfs->mknod("/accounts/show", this));
}

void Accounts::deinitVFS() throw()
{
	assert(virtualfs->rmdir("/accounts"));
}
	
void Accounts::on(PluginStarted&, Plugin* p) throw()
{
	if(p == this) {
		load();
		virtualfs = (VirtualFs*)Plugin::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			fprintf(stderr, "success: Plugin Accounts: VirtualFs interface found.\n");
			initVFS();
		} else {
			fprintf(stderr, "warning: Plugin Accounts: VirtualFs interface not found.\n");
		}
		fprintf(stderr, "success: Plugin Accounts: Started.\n");
	} else if(!virtualfs) {
		virtualfs = (VirtualFs*)Plugin::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			fprintf(stderr, "success: Plugin Accounts: VirtualFs interface found.\n");
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
		fprintf(stderr, "success: Plugin Accounts: Stopped.\n");
	} else if(virtualfs && p == virtualfs) {
		fprintf(stderr, "warning: Plugin Accounts: VirtualFs interface disabled.\n");
		virtualfs = NULL;
	}
}
	
void Accounts::on(ClientLogin&, ADCClient* client) throw()
{
	ADCInf* attr = client->getAttr();
	Users::const_iterator i = users.find(attr->getNewInf("NI"));
	if(i != users.end()) {
		UserData* data = client->getData();
		data->setInt(idUserLevel, 1);
		client->doAskPassword(i->second);
	}
}

void Accounts::on(ClientInfo&, ADCClient* client) throw()
{
	ADCInf* attr = client->getAttr();
	UserData* data = client->getData();
	if(attr->newInf("NI")) {
		// don't allow registered users to change nick
		if(data->getInt(idUserLevel))
			attr->setInf("NI", attr->getSetInf("NI")); // reset nick
		// don't allow users to change to a registered nick
		else if(users.find(attr->getNewInf("NI")) != users.end())
			attr->setInf("NI", attr->getSetInf("NI")); // reset nick
	}
}

void Accounts::on(UserConnected&, ADCClient* client) throw()
{
	UserData* data = client->getData();
	if(data->getInt(idUserLevel) >= 1) {
		ADCInf* attr = client->getAttr();
		attr->setInf("OP", "1");
	}
}

void Accounts::on(PluginMessage&, Plugin* p, void* d) throw()
{
	if(virtualfs && p == virtualfs) {
		VirtualFs::Message* m = (VirtualFs::Message*)d;
		assert(m);
		if(m->type == VirtualFs::Message::CHDIR) {
			m->reply("This is the accounts section. Create and remove accounts and properties here.");
		} else if(m->type == VirtualFs::Message::HELP) {
			if(m->cwd == "/accounts/") {
				m->reply(
						"The following commands are available to you:\r\n"
						"load\t\t\tloads the users file from disk\r\n"
						"save\t\t\tsaves the users file to disk\r\n"
						"add <user> <password>\tadds a user\r\n"
						"del <user>\t\tremoves a user\r\n"
						"list [wildcard]\t\tshows the list of registered users\r\n"
						"show <user>\t\tshows information about a user\r\n"
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
				if(m->arg.size() == 3) {
					users[m->arg[1]] = m->arg[2];
					m->reply("Success: User created/updated.");
					// check if user online..
					// add OP1 if so
				} else {
					m->reply("Syntax: add <nick> <password>");
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
				string ret = "Success: Registered users:\r\n";
				for(Users::iterator i = users.begin(); i != users.end(); ++i) {
					ret += i->first + "\r\n";
				}
				m->reply(ret);
			}
		}
	}
}
