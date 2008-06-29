// vim:ts=4:sw=4:noet
#include "Hub.h"

#include "ADC.h"
#include "Client.h"
#include "Command.h"
#include "Logs.h"
#include "ServerManager.h"
#include "Settings.h"
#include "XmlTok.h"

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
