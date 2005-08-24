#ifndef __INTERHUB_H_
#define __INTERHUB_H_

#include "ADCSocket.h"
#include "DNSUser.h"
#include "UserInfo.h"

#include <string>
#include "Util.h"
#include "compat_hash_map.h"
#include "string8.h"

using namespace std;


namespace qhub {

class Hub;

class InterHub : public ADCSocket {
public:
	InterHub(Hub* h, const string& hn, short p) throw();
	InterHub(Hub* h, int fd, Domain d) throw();
	virtual ~InterHub() throw() {};

	//we have nothing to add to onWrite() and onRead(),
	//so just let them fall through to ADCSocket

	//from DNSUser
	virtual void onLookup();

	short getPort() const { return port; }
	const string& getCID32() const { return cid; }
	bool hasClient(const string& cid) const;

	size_t getNumUsers() const { return users.size(); }
	void appendUserList(string& tmp) throw();

	// from ADCSocket
	virtual void doError(const string& msg) throw();

protected:
	/*
	 * Calls from ADCSocket
	 */
	virtual void onLine(StringList& sl, const string& full) throw();
	virtual void onConnected() throw();
	virtual void onDisconnected(string const& clue) throw();

private:
	void doSupports() throw();
	void doInf() throw();
	void doAskPassword() throw();
	void doPassword(const StringList& sl) throw();

	void handle(const StringList& sl, const string& full, uint32_t command) throw();
	void handlePassword(const StringList& sl) throw();

	string cid;
	string hostname;
	short port;
	const bool outgoing;

	typedef hash_map<string,UserInfo*> Users;
	Users users;

	string8 salt;
};

}


#endif
