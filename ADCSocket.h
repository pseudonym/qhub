// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADCSOCKET_H_
#define _INCLUDED_ADCSOCKET_H_

#include "Socket.h"
#include "compat_hash_map.h"
#include <vector>
#include <string>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "Buffer.h"

using namespace std;

namespace qhub {

//class Hub;

class ADCSocket : public Socket {
public:
	typedef vector<string> StringList;
	
	ADCSocket(int fd);
	virtual ~ADCSocket();

	virtual void on_read();
	virtual void on_write();

	virtual void onLine(StringList const& sl) = 0;

private:
	StringList data;
	
	/*
	void growBuffer();
	*/

	int readBufferSize;
	unsigned char* readBuffer;
	enum ReadState { NORMAL, PARTIAL } state;

	// hidden
	ADCSocket() {};
};

} //namespace qhub

#endif //_INCLUDED_ADCSOCKET_H_
