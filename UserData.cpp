// vim:ts=4:sw=4:noet
#include "UserData.h"
#include "Encoder.h"

using namespace qhub;

UserData::ClashMap UserData::clashMap;

UserData::Key UserData::toKey(string const& id) throw()
{
	Key k = Encoder::toCrc32(id);
	// Find Key clashes (CRC32 is not fool-proof)
	ClashMap::const_iterator i = clashMap.find(k);
	if(i != clashMap.end()) {
		if(i->second == id) {
			fprintf(stderr, "UserData::toKey: %s -> %u found\n", id.c_str(), k);
		} else {
			fprintf(stderr, "UserData::toKey: %s -> %u clashes with %s .. FATAL\n", id.c_str(), k, i->second.c_str());
			exit(1);
		}
	} else {
		fprintf(stderr, "UserData::toKey: %s -> %u created\n", id.c_str(), k);
		clashMap[k] = id;
	}
	return k;
}
