// vim:ts=4:sw=4:noet
#include <boost/cast.hpp>

#include "NetworkCtl.h"
#include "VirtualFs.h"

#include "Client.h"
#include "Hub.h"
#include "XmlTok.h"
#include "Logs.h"
#include "Settings.h"
#include "InterHub.h"
#include "ConnectionManager.h"

using namespace qhub;

/*
 * Plugin loader
 */
QHUB_GET_PLUGIN(NetworkCtl)


/*
 * Plugin details
 */

UserData::key_type NetworkCtl::idVirtualFs = "virtualfs";


void NetworkCtl::initVFS() throw()
{
	virtualfs->mkdir("/networkctl", this);
	virtualfs->mknod("/networkctl/connect", this);
	virtualfs->mknod("/networkctl/disconnect", this);
	virtualfs->mknod("/networkctl/list", this);
}

void NetworkCtl::deinitVFS() throw()
{
	virtualfs->rmdir("/networkctl");
}

void NetworkCtl::on(PluginStarted&, Plugin* p) throw()
{
	if(p == this) {
		virtualfs = (VirtualFs*)Util::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin NetworkCtl: VirtualFs interface found.\n";
			initVFS();
		} else {
			Logs::err << "warning: Plugin NetworkCtl: VirtualFs interface not found.\n";
		}
		Logs::stat << "success: Plugin NetworkCtl: Started.\n";
	} else if(!virtualfs) {
		virtualfs = (VirtualFs*)Util::data.getVoidPtr(idVirtualFs);
		if(virtualfs) {
			Logs::stat << "success: Plugin NetworkCtl: VirtualFs interface found.\n";
			initVFS();
		}
	}
}

void NetworkCtl::on(PluginStopped&, Plugin* p) throw()
{
	if(p == this) {
		if(virtualfs)
			deinitVFS();
		Logs::stat << "success: Plugin NetworkCtl: Stopped.\n";
	} else if(virtualfs && p == virtualfs) {
		Logs::err << "warning: Plugin NetworkCtl: VirtualFs interface disabled.\n";
		virtualfs = NULL;
	}
}

void NetworkCtl::on(InterConnected&, InterHub* ih) throw()
{
	interhubs.insert(ih);
}

void NetworkCtl::on(InterDisconnected&, InterHub* ih) throw()
{
	interhubs.erase(ih);
}

void NetworkCtl::on(ChDir, const string&, Client* c) throw()
{
	c->doPrivateMessage("This is the network control section."
			"  Connect and disconnect hub-hub connections here.");
}

void NetworkCtl::on(Help, const string& cwd, Client* c) throw()
{
	assert(cwd == "/networkctl/");
	c->doPrivateMessage(
			"The following commands are available to you:\n"
			"connect <host> <port> <password>\tconnect to hub\n"
			"disconnect <cid>\t\tdisconnect hub\n"
			"list\t\t\t\tshows the list of directly connected hubs"
	);
}

void NetworkCtl::on(Exec, const string& cwd, Client* c, const StringList& arg) throw()
{
	assert(arg.size() >= 1);
	if(arg[0] == "connect") {
		if(arg.size() != 4) {
			c->doPrivateMessage("Usage: connect <host> <port> <password>");
		} else {
			int port;
			try {
				port = Util::toInt(arg[2]);
			} catch(const boost::bad_lexical_cast&) {
				c->doPrivateMessage("Port parameter is not a number.");
				return;
			}
			ConnectionManager::instance()->openInterConnection(arg[1], port, arg[3]);
		}
	} else if(arg[0] == "disconnect") {
	/*	if(arg.size() != 2) {
			c->doPrivateMessage("Usage: disconnect <cid>");
		} else if(!ADC::checkCID(arg[1])) {
			c->doPrivateMessage("CID invalid");
		} else {
			for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
				if((*i)->getCID32() == arg[1]) {
					(*i)->getSocket()->disconnect("NetworkCtl");
					return;
				}
			}
		}*/
	} else if(arg[0] == "list") {
		string ret = "Connected Hubs:";
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
			ret += "\n  ";
			//ret += (*i)->getCID32();
			ret += "   ";
			ret += (*i)->getSocket()->getPeerName();
		}
		c->doPrivateMessage(ret);
	}
}
