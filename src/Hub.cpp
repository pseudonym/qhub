// vim:ts=4:sw=4:noet
#include "Hub.h"

#include "error.h"
#include "ServerSocket.h"
#include "Client.h"
#include "InterHub.h"
#include "Buffer.h"
#include "EventHandler.h"
#include "Logs.h"
#include "XmlTok.h"
#include "Settings.h"

#include <cstdlib>
#include <boost/format.hpp>

using namespace qhub;
using namespace std;

Hub::Hub() throw()
{
	XmlTok* p = Settings::instance()->getConfig("__hub");
	setName(p->getAttr("name"));
	Logs::stat << "Name: " << getName() << endl;
	sidpre = p->getAttr("prefix");
	assert(sidpre.size() == 2);
	Logs::stat << "SID prefix: " << getSidPrefix() << endl;
	setDescription(p->getAttr("description"));

	setInterPass(p->getAttr("interpass"));
}

void Hub::motd(Client* c) throw()
{
	//boost::format f("Hubconnections: %d.\nWe have %d (of which %d are passive) local users, and %d remote users.");
	//f % interhubs.size() % (activeUsers.size()+passiveUsers.size()) % passiveUsers.size() % remoteUsers.size();
	//c->doHubMessage(f.str());
}

Command Hub::getAdcInf() const throw()
{
	return Command('I', Command::INF) << CmdParam("NI", getName())
			<< CmdParam("VE", PACKAGE_NAME "/" PACKAGE_VERSION)
			<< CmdParam("DE", getDescription())
			<< CmdParam("HU", "1") << CmdParam("BO", "1") << CmdParam("OP", "1");
}

/*void Hub::userDisconnect(string const& actor, string const& victim, string const& msg) throw()
{
	Users::iterator i;
	if((i = activeUsers.find(victim)) != activeUsers.end())
		i->second->doDisconnectBy(actor, msg);
	else if((i = passiveUsers.find(victim)) != passiveUsers.end())
		i->second->doDisconnectBy(actor, msg);
}*/

