// vim:ts=4:sw=4:noet
#include "accounts.h"

#include "ADC.h"
#include "ADCInf.h"

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

void Accounts::onLogin(ADC* client) throw()
{
	ADCInf* attr = client->getAttr();
	if(attr->getNewInf("NI").find("sed") != string::npos) {
		client->doAskPassword("hoi");
	} else if(attr->getNewInf("NI").find("sandos") != string::npos) {
		client->doAskPassword("majs");
	}
}

void Accounts::onInfo(ADC* client) throw()
{
	ADCInf* attr = client->getAttr();
	if(attr->newInf("NI")) {
		// don't allow registered users to change nick
		if(client->getInt("userlevel"))
			attr->setInf("NI", attr->getOldInf("NI")); // reset nick
		// don't allow users to change to a registered nick
		else if(attr->getNewInf("NI").find("sed") != string::npos ||
				attr->getNewInf("NI").find("sandos") != string::npos)
			attr->setInf("NI", attr->getOldInf("NI")); // reset nick
	}
}

void Accounts::onAuth(ADC* client) throw()
{
	client->setInt("userlevel", 1);
	ADCInf* attr = client->getAttr();
	attr->setInf("OP", "1");
}

void Accounts::onCommand(ADC* client, string const& msg) throw()
{
}
