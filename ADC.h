#ifndef __ADC_H_
#define __ADC_H_

#include "Socket.h"
#include "compat_hash_map.h"
#include <vector>
#include <string>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "Buffer.h"

namespace qhub {

using namespace std;

class Hub;

class ADC : public Socket {
public:
	ADC(int fd, Hub* parent);
	~ADC();

	void on_read();
	void on_write();

	void sendFullInf();
	string getFullInf();

	void sendHubMsg(string msg);

	bool dis() { return disconnected; }

	static string escape(string in);
private:
	ADC(){};

	Hub* hub;

	enum State {
		START,
		PROTOCOL_ERROR, //has sent message to client, at next data, disconnect
		GOT_SUP,
		LOGGED_IN
	};
	int state;

	void growBuffer();
	unsigned char* readBuffer;
	int readBufferSize;
	int rbCur;

	void handleCommand(int length);
	void handleHCommand(int length);
	void handleBCommand(int length);
	void handleDCommand(int length);

	void checkParms();
	void getParms(int length, int positionalParms);
	vector<string> posParms;
	typedef hash_map<string, string> parmMap;
	parmMap namedParms;
	typedef parmMap::iterator parmMapIterator;

	string guid;
	bool added;
	hash_map<string, string> INF;
	typedef hash_map<string, string>::iterator INFIterator;

	void realDisconnect();
};


}

#endif
