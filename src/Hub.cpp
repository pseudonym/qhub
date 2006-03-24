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

using namespace qhub;
using namespace std;

Hub::Hub() throw()
{
	XmlTok* p = Settings::instance()->getConfig("__hub");
	setName(p->getAttr("name"));
	Logs::stat << "Name: " << getName() << endl;
	const string& sidpre = p->getAttr("prefix");
	assert(sidpre.size() == 2);
	Logs::stat << "SID prefix: " << sidpre << endl;
	sid = (sid_type(sidpre[0]) << 24) | (sid_type(sidpre[1]) << 16) | HUB_SID_END;
	setDescription(p->getAttr("description"));
	setInterPass(p->getAttr("interpass"));
}

void Hub::motd(Client* c) throw()
{
	c->doHubMessage("This hub is running " PACKAGE_NAME "/" PACKAGE_VERSION);
	c->doHubMessage("Have a nice day, and behave ;)");
}

Command Hub::getAdcInf() const throw()
{
	return Command('I', Command::INF) << CmdParam("NI", getName())
			<< CmdParam("VE", PACKAGE_NAME "/" PACKAGE_VERSION)
			<< CmdParam("DE", getDescription())
			<< "HU1" << "BO1" << "HI1";
}

/*void Hub::userDisconnect(string const& actor, string const& victim, string const& msg) throw()
{
	Users::iterator i;
	if((i = activeUsers.find(victim)) != activeUsers.end())
		i->second->doDisconnectBy(actor, msg);
	else if((i = passiveUsers.find(victim)) != passiveUsers.end())
		i->second->doDisconnectBy(actor, msg);
}*/

