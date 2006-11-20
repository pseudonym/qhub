#ifndef BUFFER_H
#define BUFFER_H

#define BOOST_SP_DISABLE_THREADS
#include <boost/shared_ptr.hpp>

#include "Command.h"

//Buffer for writeQueues for example.
//Has operator<() for priority_queue() compatibility, and an associated priority

namespace qhub {

class Buffer {
public:
	Buffer() : prio(0) {}
	Buffer(Buffer& b) : buf(b.buf), prio(b.prio) {}
	explicit Buffer(std::string const& b, int p=0) : buf(b.begin(), b.end()), prio(p) {}
	explicit Buffer(Command const& c, int p=0) : buf(c.toString().begin(), c.toString().end()), prio(p) {}
	explicit Buffer(int p) : prio(p) {}
	virtual ~Buffer() {}

	bool operator<(const Buffer& b) const { return prio < b.prio; }

	virtual void append(const Command& cmd)
	{
		buf.insert(buf.end(), cmd.toString().begin(), cmd.toString().end());
	}

	virtual const uint8_t* data() const { return &buf[0]; }
	virtual std::vector<uint8_t>::size_type size() const { return buf.size(); }

	typedef boost::shared_ptr<const Buffer> Ptr;
	typedef boost::shared_ptr<Buffer> MutablePtr;
protected:
	std::vector<uint8_t> buf;
	int prio;
};

}

#endif
