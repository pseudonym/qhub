#ifndef ___BUFFER_H__
#define ___BUFFER_H__

#include <string>
#include <boost/shared_ptr.hpp>
	
//Buffer for writeQueues for example.
//Has operator<() for priority_queue() compatibility, and an associated priority

namespace qhub {

using namespace std;

class Buffer {
public:
	Buffer(Buffer& b) { buf = b.buf; prio=b.prio; };
	Buffer(string b, int p) : buf(b), prio(p) {};
	~Buffer() { fprintf(stderr, "Deleting me %d %d %p\n", prio, buf.size(), this); };
	bool operator<(Buffer& b) { return prio < b.prio; };

	string& getBuf() { return buf; };

	typedef boost::shared_ptr<Buffer> writeBuffer;
private:
	Buffer(){};
	string buf;
	int prio;
};

}

#endif
