#include "UserData.h"
#include "Encoder.h"

using namespace qhub;

UserData::Key UserData::toKey(string const& id) throw()
{
	return Encoder::toCrc32(id);
}
