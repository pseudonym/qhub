// vim:ts=4:sw=4:noet
#include <boost/cast.hpp>
#include <sstream>
#include <iterator>

#include "Bans.h"

#include "../ADCClient.h"
#include "../Hub.h"
#include "../UserInfo.h"
#include "../UserData.h"
#include "../XmlTok.h"
#include "../Logs.h"
#include "../Settings.h"
#include "VirtualFs.h"

using namespace qhub;

/*
 * Plugin loader
 */

extern "C" {
	void* getPlugin() { return new Bans(); }
} //extern "C"



/*
 * Plugin details
 */

UserData::Key Bans::idVirtualFs = UserData::toKey("virtualfs");

bool Bans::load() throw()
{
	bool success = false;
	XmlTok root;
	if(root.load(Settings::getFilename(getId())) && root.findChild("bans")) {
		XmlTok* p = &root;
		ipBans.clear(), nickBans.clear(), cidBans.clear(); // clean old bans
		success = true;
		p = p->getNextChild();
		if(p->findChild("ipbans")) {
			p = p->getNextChild();
			XmlTok* tmp;
			p->findChild("ip");
			while((tmp = p->getNextChild())) {
				const string& banner = tmp->getAttr("banner");
				const string& reason = tmp->getAttr("reason");
				const string& ip = tmp->getData();
				time_t t = boost::lexical_cast<time_t>(tmp->getAttr("timeout"));
				BanInfo bi(t, banner, reason);
				ipBans.insert(make_pair(ip, bi));
			}
			p = p->getParent();
		}
		if(p->findChild("nickbans")) {
			p = p->getNextChild();
			XmlTok* tmp;
			p->findChild("nick");
			while((tmp = p->getNextChild())) {
				const string& banner = tmp->getAttr("banner");
				const string& reason = tmp->getAttr("reason");
				const string& nick = tmp->getData();
				time_t t = boost::lexical_cast<time_t>(tmp->getAttr("timeout"));
				BanInfo bi(t, banner, reason);
				nickBans.insert(make_pair(nick, bi));
			}
			p = p->getParent();
		}
		if(p->findChild("cidbans")) {
			p = p->getNextChild();
			XmlTok* tmp;
			p->findChild("cid");
			while((tmp = p->getNextChild())) {
				const string& banner = tmp->getAttr("banner");
				const string& reason = tmp->getAttr("reason");
				const string& cid = tmp->getData();
				time_t t = boost::lexical_cast<time_t>(tmp->getAttr("timeout"));
				BanInfo bi(t, banner, reason);
				cidBans.insert(make_pair(cid, bi));
			}
			p = p->getParent();
		}
	}
	return success;
}

bool Bans::save() throw()
{
	XmlTok root;
	XmlTok* p = &root;
	p = p->addChild("bans");
	p = p->addChild("ipbans");
	for(BanList::iterator i = ipBans.begin(); i != ipBans.end(); ) {
		if(i->second.timeout <= time(0)) {
			ipBans.erase(i++);
			continue;
		}
		XmlTok* tmp = p->addChild("ip");
		tmp->setAttr("banner", i->second.banner);
		tmp->setAttr("reason", i->second.reason);
		tmp->setAttr("timeout", Util::toString(i->second.timeout));
		tmp->setData(i->first);
		++i;
	}
	p = p->getParent();
	p = p->addChild("nickbans");
	for(BanList::iterator i = nickBans.begin(); i != nickBans.end(); ) {
		if(i->second.timeout <= time(0)) {
			nickBans.erase(i++);
			continue;
		}
		XmlTok* tmp = p->addChild("nick");
		tmp->setAttr("banner", i->second.banner);
		tmp->setAttr("reason", i->second.reason);
		tmp->setAttr("timeout", Util::toString(i->second.timeout));
		tmp->setData(i->first);
		++i;
	}
	p = p->getParent();
	p = p->addChild("cidbans");
	for(BanList::iterator i = cidBans.begin(); i != cidBans.end(); ) {
		if(i->second.timeout <= time(0)) {
			cidBans.erase(i++);
			continue;
		}
		XmlTok* tmp = p->addChild("cid");
		tmp->setAttr("banner", i->second.banner);
		tmp->setAttr("reason", i->second.reason);
		tmp->setAttr("timeout", Util::toString(i->second.timeout));
		tmp->setData(i->first);
		++i;
	}

	return root.save(Settings::getFilename(getId()));
}

void Bans::initVFS() throw()
{
	virtualfs->mkdir("/bans", this);
	virtualfs->mknod("/bans/load", this);
	virtualfs->mknod("/bans/save", this);
	virtualfs->mknod("/bans/banip", this);
	virtualfs->mknod("/bans/bannick", this);
	virtualfs->mknod("/bans/bancid", this);
	virtualfs->mknod("/bans/list", this);
}

void Bans::deinitVFS() throw()
{
	virtualfs->rmdir("/bans");
}

void Bans::on(PluginStarted&, Plugin* p) throw()
{
	if(p == this) {
		load();
		virtualfs = (VirtualFs*)Plugin::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin Bans: VirtualFs interface found.\n";
			initVFS();
		} else {
			Logs::err << "warning: Plugin Bans: VirtualFs interface not found.\n";
		}
		Logs::stat << "success: Plugin Bans: Started.\n";
	} else if(!virtualfs) {
		virtualfs = (VirtualFs*)Plugin::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin Bans: VirtualFs interface found.\n";
			initVFS();
		}
	}
}

void Bans::on(PluginStopped&, Plugin* p) throw()
{
	if(p == this) {
		if(virtualfs)
			deinitVFS();
		save();
		Logs::stat << "success: Plugin Bans: Stopped.\n";
	} else if(virtualfs && p == virtualfs) {
		Logs::err << "warning: Plugin Bans: VirtualFs interface disabled.\n";
		virtualfs = NULL;
	}
}

void Bans::killUser(ADCClient* client, const Bans::BanInfo& bi) throw()
{
	string msg = ADC::ESC(bi.reason + " //" + bi.banner);
	if(bi.timeout == numeric_limits<time_t>::max())
		client->send("ISTA " + client->getHub()->getCID32() + " 231 " + msg + '\n');
	else
		client->send("ISTA " + client->getHub()->getCID32() + " 232 " + msg +
				" TL" + Util::toString(bi.timeout - time(0)) + '\n');
	client->doDisconnect("banned: " + ADC::CSE(msg));
}

void Bans::on(ClientLogin& action, ADCClient* client) throw()
{
	BanList::iterator i;
	if((i = ipBans.find(client->getPeerName())) != ipBans.end()) {
		if(i->second.timeout < time(0))
			ipBans.erase(i);
		else {
			killUser(client, i->second);
			action.setState(Plugin::DISCONNECTED);
			return;
		}
	}
	if((i = nickBans.find(client->getUserInfo()->getNick())) != nickBans.end()) {
		if(i->second.timeout < time(0))
			nickBans.erase(i);
		else {
			killUser(client, i->second);
			action.setState(Plugin::DISCONNECTED);
			return;
		}
	}
	if((i = cidBans.find(client->getCID32())) != cidBans.end()) {
		if(i->second.timeout < time(0))
			cidBans.erase(i);
		else {
			killUser(client, i->second);
			action.setState(Plugin::DISCONNECTED);
			return;
		}
	}
}

void Bans::on(PluginMessage&, Plugin* p, void* d) throw()
{
	if(virtualfs && p == virtualfs) {
		VirtualFs::Message* m = static_cast<VirtualFs::Message*>(d);
		assert(m);
		if(m->type == VirtualFs::Message::CHDIR) {
			m->reply("This is the bans section. Create and remove bans and properties here.");
		} else if(m->type == VirtualFs::Message::HELP) {
			if(m->cwd == "/bans/") {
				m->reply(
				    "The following commands are available to you:\n"
				    "load\t\t\t\tloads the bans file from disk\n"
				    "save\t\t\t\tsaves the bans file to disk\n"
				    "ban(ip|nick|cid) <item> <time> [description]\tbans the specified item\n"
					"\ttime has the same semantics as Verlihub, except no specifier\n"
					"\tmeans minutes and 'm' means months\n"
					"\ttime=0 means unban, time=-1 means forever\n"
				    "list\t\t\t\tshows the list of bans"
				);
			}
		} else if(m->type == VirtualFs::Message::EXEC) {
			assert(m->arg.size() >= 1);
			if(m->arg[0] == "load") {
				if(load()) {
					m->reply("Success: Bans file reloaded.");
				} else {
					m->reply("Failure: Failed to reload Bans file.");
				}
			} else if(m->arg[0] == "save") {
				if(save()) {
					m->reply("Success: Bans file saved.");
				} else {
					m->reply("Failure: Failed to save Bans file.");
				}
			} else if(m->arg[0] == "banip") {
				if(m->arg.size() > 2 && inet_addr(m->arg[1].c_str()) == INADDR_NONE) {
					m->reply("Invalid IP address.");
					return;
				}
				if(m->arg.size() >= 3) {
					ostringstream str;
					// turn the ban reason into one string
					copy(m->arg.begin()+3, m->arg.end(), ostream_iterator<string>(str, " "));
					BanInfo b(parseTime(m->arg[2]), m->client->getUserInfo()->getNick(),
							str.str());
					ipBans.insert(make_pair(m->arg[1], b));
					m->reply("Success: ban added.");
				} else {
					m->reply("Syntax: banip <ip> <time> [description]");
				}
			} else if(m->arg[0] == "bannick") {
				if(m->arg.size() >= 3) {
					ostringstream str;
					// turn the ban reason into one string
					copy(m->arg.begin()+3, m->arg.end(), ostream_iterator<string>(str, " "));
					BanInfo b(parseTime(m->arg[2]), m->client->getUserInfo()->getNick(),
							str.str());
					nickBans.insert(make_pair(m->arg[1], b));
					m->reply("Success: ban added.");
				} else {
					m->reply("Syntax: bannick <nick> <time> [description]");
				}
			} else if(m->arg[0] == "bancid") {
				if(m->arg.size() > 2 && !ADC::checkCID(m->arg[1])) {
					m->reply("Invalid CID.");
					return;
				}
				if(m->arg.size() >= 3) {
					ostringstream str;
					//turn the ban reason into one string
					copy(m->arg.begin()+3, m->arg.end(), ostream_iterator<string>(str, " "));
					BanInfo b(parseTime(m->arg[2]), m->client->getUserInfo()->getNick(),
							str.str());
					cidBans.insert(make_pair(m->arg[1], b));
					m->reply("Success: ban added.");
				} else {
					m->reply("Syntax: bancid <cid> <time> [description]");
				}
			} else if(m->arg[0] == "list") {
				string ret = "Banned IP addresses:";
				for(BanList::const_iterator i = ipBans.begin(); i != ipBans.end(); ++i) {
					ret += "\n  ";
					ret += i->first;
					ret += " (" + Util::toString(i->second.timeout) + ") ";
					ret += i->second.reason + " //" + i->second.banner;
				}
				ret += "\n\nBanned nicknames:";
				for(BanList::const_iterator i = nickBans.begin(); i != nickBans.end(); ++i) {
					ret += "\n  ";
					ret += i->first;
					ret += " (" + Util::toString(i->second.timeout) + ") ";
					ret += i->second.reason + " //" + i->second.banner;
				}
				ret += "\n\nBanned CIDs:";
				for(BanList::const_iterator i = cidBans.begin(); i != cidBans.end(); ++i) {
					ret += "\n  ";
					ret += i->first;
					ret += " (" + Util::toString(i->second.timeout) + ") ";
					ret += i->second.reason + " //" + i->second.banner;
				}
				m->reply(ret);
			}
		}
	}
}

time_t Bans::parseTime(const string& tmp)
{
	istringstream str(tmp);
	int64_t num;
	char type = 0;
	time_t t;

	if(str >> num) {
		if(num == int64_t(-1))
			return numeric_limits<time_t>::max();
		str >> type;	// if this part fails, it's ok because we default to minutes...
		switch(type) {
		case 'y': case 'Y':
			num *= 12;
		case 'm': case 'M':
			num *= 4;
		case 'w': case 'W':
			num *= 7;
		case 'd': case 'D':
			num *= 24;
		case 'h': case 'H':
			num *= 60;
		default:
			num *= 60;
		}
		num += static_cast<int64_t>(time(0));
		try {
			t = boost::numeric_cast<time_t>(num);
		} catch(boost::bad_numeric_cast&) {
			t = numeric_limits<time_t>::max();
		}
		return t;
	} else
		throw Exception("invalid ban time value");
}
