// vim:ts=4:sw=4:noet
#include "accounts.h"

#include "ADCClient.h"
#include "ADCInf.h"
#include "Encoder.h"
#include "UserData.h"
#include "XmlTok.h"

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

void Accounts::load() throw()
{
	XmlTok root;
	if(root.load("/etc/qhub/accounts.xml")) {
		XmlTok* p = &root;
		if(p->findChild("accounts")) {
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
}

void Accounts::save() const throw()
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
	root.save("/etc/qhub/accounts.xml");
}
		
void Accounts::onLogin(ADCClient* client) throw()
{
	ADCInf* attr = client->getAttr();
	Users::const_iterator i = users.find(attr->getNewInf("NI"));
	if(i != users.end()) {
		client->doAskPassword(i->second);
	}
}

void Accounts::onInfo(ADCClient* client) throw()
{
	ADCInf* attr = client->getAttr();
	UserData* data = client->getData();
	if(attr->newInf("NI")) {
		// don't allow registered users to change nick
		if(data->getInt(idUserLevel))
			attr->setInf("NI", attr->getOldInf("NI")); // reset nick
		// don't allow users to change to a registered nick
		else if(users.find(attr->getNewInf("NI")) != users.end())
			attr->setInf("NI", attr->getOldInf("NI")); // reset nick
	}
}

void Accounts::onAuth(ADCClient* client) throw()
{
	UserData* data = client->getData();
	data->setInt(idUserLevel, 1);
	assert(data->getInt(idUserLevel) == 1);
	ADCInf* attr = client->getAttr();
	attr->setInf("OP", "1");
}

void Accounts::onCommand(ADCClient* client, string const& msg) throw()
{
	if(client->getData()->getInt(idUserLevel)) {
		StringList sl = Util::lazyStringTokenize(msg);
		if(sl.size() >= 1 && sl[0] == "acct") {
			if(sl.size() == 2 && sl[1] == "load") {
				load();
				client->doHubMessage("accounts reloaded");
			} else if(sl.size() == 2 && sl[1] == "save") {
				save();
				client->doHubMessage("accounts saved");
			} else if(sl.size() == 4 && sl[1] == "add") {
				users[sl[2]] = sl[3];
				client->doHubMessage("account added/updated");
			} else if(sl.size() == 3 && sl[1] == "del") {
				Users::iterator i = users.find(sl[2]);
				if(i != users.end()) {
					users.erase(i);
					client->doHubMessage("account deleted");
				} else {
					client->doHubMessage("account not found");
				}	
			}
		}
	}
}
