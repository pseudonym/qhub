// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADC_H_
#define _INCLUDED_ADC_H_

#include "ADCSocket.h"
#include "compat_hash_map.h"
#include <vector>
#include <string>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "Buffer.h"
#include "string8.h"

namespace qhub {

using namespace std;

class Hub;

class ADC : public ADCSocket {
public:
	ADC(int fd, Hub* parent);
	virtual ~ADC();

	virtual void on_read() { ADCSocket::on_read(); };
	virtual void on_write() { ADCSocket::on_write(); };

	// Other stuff
	string getFullInf() const { return attributes.getFullInf(); }
	string const& getCID32() const { return guid; };

	// Send-to functions
	void sendHubMessage(string const& msg);

	/*
	 * Data handlers	
	 */
	void handleA(StringList const& sl, string const& full);
	void handleB(StringList const& sl, string const& full);
	void handleBINF(StringList const& sl, string const& full);
	void handleBMSG(StringList const& sl, string const& full);
	void handleD(StringList const& sl, string const& full);
	void handleH(StringList const& sl, string const& full);
	void handleHDSC(StringList const& sl, string const& full);
	void handleHPAS(StringList const& sl, string const& full);
	void handleHSUP(StringList const& sl, string const& full);
	void handleP(StringList const& sl, string const& full);

	/*
	 * Calls from ADCSocket
	 */
	virtual void doWarning(string const& msg);
	virtual void doError(string const& msg); // send FatalError message
	virtual void onLine(StringList const& sl, string const& full);
	virtual void onConnected();
	virtual void onDisconnected(string const& clue);
	
private:
	class Attributes {
	public:
		Attributes(ADC* p) : parent(p) {};
		bool setInf(StringList const& sl);
		bool setInf(string const& key, string const& val);
		string const& getFullInf() const { return full; }
		string getChangedInf();
		
		bool isOp() const;
	private:
		void updateInf();
		typedef hash_map<string, string> Inf;
		Inf current;
		Inf changes;
		string full;
		ADC* parent;
	} attributes;
		
	Hub* hub;
	void login();
	void logout();

	enum State {
		START,		// HSUP
		IDENTIFY,	// BINF
		VERIFY,		// HPAS
		NORMAL
	};
	int state;
	string guid;
	bool added;
	string8 salt;
	
	// Invalid
	ADC() : ADCSocket(-1), attributes(0) {};
};

}

#endif //_INCLUDED_ADC_H_
