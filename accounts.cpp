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
UserData::Key Accounts::idVirtualPath = UserData::toKey("virtualpath");

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
		
void Accounts::onLogin(int& a, ADCClient* client) throw()
{
	a |= HANDLED;
	ADCInf* attr = client->getAttr();
	Users::const_iterator i = users.find(attr->getNewInf("NI"));
	if(i != users.end()) {
		client->doAskPassword(i->second);
	}
}

void Accounts::onInfo(int& a, ADCClient* client) throw()
{
	a |= HANDLED;
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

void Accounts::onAuth(int& a, ADCClient* client) throw()
{
	a |= HANDLED;
	UserData* data = client->getData();
	data->setInt(idUserLevel, 1);
	assert(data->getInt(idUserLevel) == 1);
	ADCInf* attr = client->getAttr();
	attr->setInf("OP", "1");
}

void Accounts::onCommand(int& a, ADCClient* client, string const& msg) throw()
{
	if(client->getData()->getInt(idUserLevel)) {
		UserData* data = client->getData();
		StringList sl = Util::lazyStringTokenize(msg);
		string const& pwd = data->getString(idVirtualPath);
		size_t siz = sl.size();
		if(a != HANDLED && a != STOP && siz == 1 && sl[0] == "pwd") {
			a |= HANDLED;
			client->doPrivateMessage(pwd.empty() ? "/" : pwd);
		} else if(pwd.empty() && siz == 1 && sl[0] == "ls") {
			a |= HANDLED;
			client->doPrivateMessage("accounts/");
		} else if(a != HANDLED && a != STOP && 
				((siz == 1 && sl[0] == "cd") || (siz == 2 && sl[0] == "cd" && sl[1] == "/"))) {
			a |= HANDLED;
			data->setString(idVirtualPath, Util::emptyString);
			client->doPrivateMessage("/");
		}
		if(a == HANDLED || a == STOP)
			return;

		if(siz == 2 && sl[0] == "cd" && sl[1] == "/accounts" || sl[1] == "/accounts/" ||
				(pwd.empty() && (sl[1] == "accounts/" || sl[1] == "accounts"))) {
			a |= HANDLED;
			data->setString(idVirtualPath, "/accounts/");
			client->doPrivateMessage("/accounts/");
		} else if(pwd.find("/accounts/") == 0) {
			a |= HANDLED;
			if(siz == 1 && sl[0] == "ls") {
				client->doPrivateMessage("add <nick> <password>");
				client->doPrivateMessage("del <nick>");
				client->doPrivateMessage("list [wildcards]");
				client->doPrivateMessage("load");
				client->doPrivateMessage("save");
			} else if(siz == 1 && sl[0] == "load") {
				if(load()) {
					client->doPrivateMessage("success: accounts reloaded");
				} else {
					client->doPrivateMessage("failure: accounts file loading failed");
				}
			} else if(siz == 1 && sl[0] == "save") {
				if(save()) {
					client->doPrivateMessage("success: accounts saved");
				} else {
					client->doPrivateMessage("failure: accounts file saving failed");
				}
			} else if(sl.size() == 3 && sl[0] == "add") {
				users[sl[1]] = sl[2];
				client->doPrivateMessage("success: account added/updated");
			} else if(sl.size() == 2 && sl[0] == "del") {
				Users::iterator i = users.find(sl[1]);
				if(i != users.end()) {
					users.erase(i);
					client->doPrivateMessage("success: account deleted");
				} else {
					client->doPrivateMessage("failure: account not found");
				}
			} else if(sl.size() >= 1 && sl[0] == "list") {
				// accept wildcards.. fixme
				for(Users::const_iterator i = users.begin(); i != users.end(); ++i) {
					client->doPrivateMessage(i->first + '\t' + i->second);
				}
			} else {
				client->doPrivateMessage("failure: command or parameters unexpected");
			}
		}
	}
}
