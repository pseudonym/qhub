#ifndef __ADC_H_
#define __ADC_H_

#include "Socket.h"
#include "compat_hash_map.h"
#include <vector>
#include <string>

namespace qhub {

using namespace std;

class Hub;

class ADC : public Socket {
public:
	ADC(int fd, Hub* parent);

	void on_read();
	void on_write();

	void sendFullInf();
	string getFullInf();
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

	void getParms(int length, int positionalParms);
	std::vector<string> posParms;
	
	typedef std::hash_map<string, string> parmMap;
	parmMap namedParms;
	
	typedef parmMap::iterator parmMapIterator;


	
	string guid;
	std::hash_map<string, string> INF;
	typedef std::hash_map<string, string>::iterator INFIterator;
};


}

#endif
