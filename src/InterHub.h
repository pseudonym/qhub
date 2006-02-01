#ifndef __INTERHUB_H_
#define __INTERHUB_H_

#include "ADCSocket.h"
#include "ConnectionBase.h"
#include "DNSAdapter.h"
#include "UserInfo.h"

#include <string>
#include "Util.h"
#include "compat_hash_map.h"

using namespace std;


namespace qhub {

class Hub;

class InterHub : public ConnectionBase {
public:
	InterHub(Hub* h, const string& hn, short p, const string& pa) throw();
	InterHub(Hub* h, ADCSocket* s) throw();
	virtual ~InterHub() throw() {};

	//for DNSLookup callback
	virtual void onLookup(const string& ip);

	short getPort() const { return port; }
	const string& getCID32() const { return cid; }

	// from ConnectionBase
	virtual void doError(const string& msg) throw();

protected:
	/*
	 * Calls from ConnectionBase
	 */
	virtual void onLine(StringList& sl, const string& full) throw(command_error);
	virtual void onConnected() throw();
	virtual void onDisconnected(string const& clue) throw();

private:
	void doSupports() throw();
	void doInf() throw();
	void doAskPassword() throw();
	void doPassword(const StringList& sl) throw();

	void handle(const StringList& sl, string& full, uint32_t command) throw(command_error);
	void handlePassword(const StringList& sl) throw(command_error);

	string cid;
	string hostname;
	short port;
	string password;
	const bool outgoing;

	vector<u_int8_t> salt;
};

}


#endif
