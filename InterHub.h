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

class InterHub : public ADCSocket, public DNSUser {
public:
	InterHub(Hub* h, const string& hn, short p) throw();
	InterHub(Hub* h, int fd, Domain d) throw();
	virtual ~InterHub() throw() {};

	static void addPass(const string& cid, const string& pass) throw()
	{ passes[cid] = pass; }
	static const string& getPass(const string& cid) throw();

	//we have nothing to add to onWrite() and onRead(),
	//so just let them fall through to ADCSocket

	//from DNSUser
	virtual void onLookup(adns_answer* reply);

	void setHostName(string h) { hostname = h; }
	void setPort(short p) { port = p; }
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

	virtual void disconnect(const string& msg = Util::emptyString);

private:
	void doSupports() throw();
	void doInf() throw();
	void doAskPassword() throw();
	void doPassword(const StringList& sl) throw();

	void handle(const StringList& sl, const string& full, uint32_t command) throw();
	void handlePassword(const StringList& sl) throw();

	static StringMap passes;

	string cid;
	string hostname;
	short port;
	bool outgoing;

	typedef hash_map<string,UserInfo*> Users;
	Users users;

	string8 salt;
};

}


#endif
