// vim:ts=4:sw=4:noet
#include "ZBuffer.h"

#include <boost/scoped_array.hpp>

#include <zlib.h>

using namespace std;
using namespace qhub;

ZBuffer::ZBuffer(Command const& c, int p) : Buffer(p)
{
	init();
	append(c);
}

ZBuffer::ZBuffer(int p) : Buffer(p)
{
	init();
}

ZBuffer::~ZBuffer()
{
	delete zcontext;
}

void ZBuffer::init()
{
	// so other end knows zlib stream is starting
	Buffer::append(Command('I', Command::ZON));

	zcontext = new z_stream;
	zcontext->zalloc = NULL;
	zcontext->zfree = NULL;
	zcontext->opaque = NULL;
	// FIXME compression level should be an option...
	if(deflateInit(zcontext, Z_BEST_COMPRESSION) != Z_OK)
		throw runtime_error("could not initialize zlib stream");
}

void ZBuffer::append(const Command& cmd)
{
	boost::scoped_array<uint8_t> zbuf(new uint8_t[BUFSIZ]); // probably doesn't need to be very big
	const string& str = cmd.toString();
	boost::scoped_array<uint8_t> readbuf(new uint8_t[str.size()]);
	copy(str.begin(), str.end(), readbuf.get());
	zcontext->next_in = readbuf.get();
	zcontext->avail_in = str.size();

	while(zcontext->avail_in) {
		zcontext->next_out = zbuf.get();
		zcontext->avail_out = BUFSIZ;
		int ret = deflate(zcontext, Z_NO_FLUSH); // compress
		if(ret == Z_STREAM_ERROR || ret == Z_BUF_ERROR)
			throw runtime_error("compression failure");
		buf.insert(buf.end(), zbuf.get(), zcontext->next_out); // copy compressed to buffer
	}
	zcontext->next_in = zcontext->next_out = NULL;
	zcontext->avail_out = 0;
}

void ZBuffer::finalize()
{
	append(Command('I', Command::ZOF)); // really shouldn't be necessary, but
	                                    // the ADC standard says so...
	finalized = true;
	boost::scoped_array<uint8_t> zbuf(new uint8_t[BUFSIZ]);
	int retval = Z_OK;

	while(retval != Z_STREAM_END) {
		zcontext->next_out = zbuf.get();
		zcontext->avail_out = BUFSIZ;
		retval = deflate(zcontext, Z_FINISH);
		if(retval == Z_STREAM_ERROR)
			throw runtime_error("compression failure");
		buf.insert(buf.end(), zbuf.get(), zcontext->next_out);
	}
	deflateEnd(zcontext);
	delete zcontext;
	zcontext = NULL;
}

const uint8_t* ZBuffer::data() const
{
	assert(finalized && "zlib stream not finalized before use!");
	return Buffer::data();
}

std::vector<uint8_t>::size_type ZBuffer::size() const
{
	assert(finalized && "zlib stream not finalized before use!");
	return Buffer::size();
}

