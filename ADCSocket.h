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
#include "Util.h"

using namespace std;

namespace qhub {

class Hub;

class ADCSocket : public Socket {
public:
	/*
	 * Pseudo-FOURCC stuff
	 */
	static u_int32_t stringToFourCC(string const& c) {
		return ((u_int32_t)c[0])|((u_int32_t)c[1]<<8)|((u_int32_t)c[2]<<16)|((u_int32_t)c[3]<<24);
	}
#define CMD(n, a, b, c) n = (((u_int32_t)a<<8) | (((u_int32_t)b)<<16) | (((u_int32_t)c)<<24))
	enum {
		CMD(CTM, 'C','T','M'),
		CMD(DSC, 'D','S','C'),
		CMD(GET, 'G','E','T'),
		CMD(GFI, 'G','F','I'),
		CMD(GPA, 'G','P','A'),
		CMD(INF, 'I','N','F'),
		CMD(MSG, 'M','S','G'),
		CMD(NTD, 'N','T','D'),
		CMD(PAS, 'P','A','S'),
		CMD(QUI, 'Q','U','I'),
		CMD(RCM, 'R','C','M'),
		CMD(RES, 'R','E','S'),
		CMD(SCH, 'S','C','H'),
		CMD(SND, 'S','N','D'),
		CMD(STA, 'S','T','A'),
		CMD(SUP, 'S','U','P')
	};
#undef CMD

	/*
	 * Client states
	 */
	enum State {
		PROTOCOL,	// HSUP
		IDENTIFY,	// BINF
		VERIFY,		// HPAS
		NORMAL		// everything except HPAS
	};


	/*
	 * Normal
	 */
	ADCSocket(int fd, Domain domain, Hub* parent) throw();
	ADCSocket(Hub* parent) throw();
	virtual ~ADCSocket() throw();
	void send(string const& msg) { write(msg, 0); };
	Hub* getHub() throw() { return hub; };

	State getState() const throw() { return state; };

	/*
	 * fd_demux calls
	 */
	virtual void onRead() throw();
	virtual void onWrite() throw();

	virtual void onAlarm() throw();

	/*
	 * Do protocol stuff / Handle events
	 */
	virtual void doError(string const& msg) throw() = 0; // send FatalError message
	
protected:
	/*
	 * Do protocol stuff / Handle events
	 */
	virtual void onLine(StringList& sl, string const& full) throw() = 0;
	virtual void onConnected() throw() = 0;
	virtual void onDisconnected(string const& clue) throw() = 0;

	virtual void disconnect(string const& msg = Util::emptyString);
	void realDisconnect();

	// much easier than changing all the assignments to setState() :)
	State state;

private:
	void handleOnRead();

	char* readBuffer;
	size_t readPos;

	Hub* hub;

	// Invalid
	ADCSocket() {};
};

} //namespace qhub

#endif //_INCLUDED_ADCSOCKET_H_
