// vim:ts=4:sw=4:noet
#ifndef QHUB_ZBUFFER_H
#define QHUB_ZBUFFER_H

#include "Buffer.h"
#include <zlib.h>

namespace qhub {

class ZBuffer : public Buffer {
public:
	explicit ZBuffer(Command const& c, int p=0);
	explicit ZBuffer(int p=0);
	virtual ~ZBuffer();

	virtual void append(const Command& cmd);

	virtual void finalize();

	virtual const uint8_t* data() const;
	virtual std::vector<uint8_t>::size_type size() const;

	typedef boost::shared_ptr<ZBuffer> MutablePtr;
protected:
	void init();

	bool finalized;
	z_streamp zcontext;
};

} // namespace qhub

#endif // QHUB_ZBUFFER_H
