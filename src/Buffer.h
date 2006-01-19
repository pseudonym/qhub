#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <boost/shared_ptr.hpp>

//Buffer for writeQueues for example.
//Has operator<() for priority_queue() compatibility, and an associated priority

namespace qhub {

using namespace std;

class Buffer {
public:
	Buffer(Buffer& b) : buf(b.buf), prio(b.prio) {};
	Buffer(string const& b, int p=0) : buf(b), prio(p) {};
	bool operator<(Buffer& b) { return prio < b.prio; };

	string& getBuf() { return buf; };

	typedef boost::shared_ptr<Buffer> Ptr;
private:
	Buffer(){};
	string buf;
	int prio;
};

}

#endif
