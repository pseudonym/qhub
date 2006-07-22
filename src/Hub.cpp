// vim:ts=4:sw=4:noet
#include "Hub.h"

#include "Client.h"
#include "Logs.h"
#include "XmlTok.h"
#include "Settings.h"
#include "ServerManager.h"
#include "ADC.h"

using namespace qhub;
using namespace std;

Hub::Hub() throw()
{
	XmlTok* p = Settings::instance()->getConfig("__hub");
	setName(p->getAttr("name"));
	Logs::stat << "Name: " << getName() << endl;
	sid = Util::toInt(p->getAttr("sid"));
	assert(sid == (sid & ServerManager::instance()->getHubSidMask()));
	Logs::stat << "SID: " << ADC::fromSid(sid) << endl;
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

