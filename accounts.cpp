// vim:ts=4:sw=4:noet
#include "accounts.h"

#include "ADCClient.h"
#include "ADCInf.h"
#include "Encoder.h"
#include "UserData.h"

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

void Accounts::onLogin(ADCClient* client) throw()
{
	ADCInf* attr = client->getAttr();
	if(attr->getNewInf("NI").find("sed") != string::npos) {
		client->doAskPassword("hoi");
	} else if(attr->getNewInf("NI").find("sandos") != string::npos) {
		client->doAskPassword("majs");
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
		else if(attr->getNewInf("NI").find("sed") != string::npos ||
				attr->getNewInf("NI").find("sandos") != string::npos)
			attr->setInf("NI", attr->getOldInf("NI")); // reset nick
	}
}

void Accounts::onAuth(ADCClient* client) throw()
{
	UserData* data = client->getData();
	data->setInt(idUserLevel, 1);
	ADCInf* attr = client->getAttr();
	attr->setInf("OP", "1");
}

void Accounts::onCommand(ADCClient* client, string const& msg) throw()
{
}
