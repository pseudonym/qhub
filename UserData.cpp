#include "UserData.h"
#include "Encoder.h"

using namespace qhub;

u_int32_t UserData::toKey(string const& id) throw()
{
	return Encoder::toCrc32(id);
}
