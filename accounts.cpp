// vim:ts=4:sw=4:noet
#include "accounts.h"

#include "ADC.h"
#include "ADCInf.h"

using namespace qhub;

/*
 * Plugin loader
 */

static Accounts* accounts = NULL;

extern "C" {

void* start()
{
	accounts = new Accounts();
	return accounts;
}

void stop()
{
	delete accounts;
}

} //extern "C"
	


/*
 * Plugin details
 */

void Accounts::onLogin(ADC* client) throw() {
	ADCInf* attr = client->getAttr();
	if(attr->getNewInf("NI").find("sed") != string::npos) {
		client->doAskPassword("hoi");
	} else if(attr->getNewInf("NI").find("sandos") != string::npos) {
		client->doAskPassword("majs");
	}
}

void Accounts::onInfo(ADC* client) throw() {
	ADCInf* attr = client->getAttr();
	// don't allow registered nicks to be used
	if(attr->newInf("NI"))
		if(attr->getNewInf("NI").find("sed") != string::npos || attr->getNewInf("NI").find("sandos") != string::npos)
			attr->setInf("NI", attr->getOldInf("NI")); // reset nick if registered
	// fix so registered users can't change nicks at all
	// (except maybe from sed -> sed|away e.g.)
	// else they can't get their nick back :P
}

void Accounts::onAuth(ADC* client) throw() {
	ADCInf* attr = client->getAttr();
	attr->setInf("OP", "1");
}
