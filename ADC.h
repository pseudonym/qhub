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

	// Data handlers	
	void handleA(StringList const& sl, string const& full);
	void handleB(StringList const& sl, string const& full);
	void handleBINF(StringList const& sl, string const& full);
	void handleD(StringList const& sl, string const& full);
	void handleH(StringList const& sl, string const& full);
	void handleHSUP(StringList const& sl, string const& full);
	void handleP(StringList const& sl, string const& full);

	// Calls from ADCSocket
	virtual void onLine(StringList const& sl, string const& full);
	virtual void onLineError(string const& msg);
	virtual void onDisconnect();
private:
	class Attributes {
	public:
		Attributes(ADC* p) : parent(p) {};
		bool setInf(StringList const& sl);
		bool setInf(string const& key, string const& val);
		string const& getFullInf() const { return full; }
		string getChangedInf();
	private:
		void updateInf();
		typedef hash_map<string, string> Inf;
		Inf current;
		Inf changes;
		string full;
		ADC* parent;
	} attributes;
		
	Hub* hub;

	enum State {
		START,
		PROTOCOL_ERROR,
		GOT_SUP,
		LOGGED_IN
	};
	int state;
	string guid;
	bool added;
	
	// Invalid
	ADC() : attributes(0), ADCSocket(-1) {};
};

}

#endif //_INCLUDED_ADC_H_
