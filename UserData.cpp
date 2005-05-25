// vim:ts=4:sw=4:noet
#include "UserData.h"
#include "Encoder.h"
#include "Logs.h"

using namespace qhub;

UserData::ClashMap UserData::clashMap;

UserData::Key UserData::toKey(string const& id) throw()
{
	Key k = Encoder::toCrc32(id);
	// Find Key clashes (CRC32 is not fool-proof)
	ClashMap::const_iterator i = clashMap.find(k);
	if(i != clashMap.end()) {
		if(i->second == id) {
			//log(qstat, format("UserData::toKey: %s -> %x found") % id % k);
		} else {
			Logs::err << format("UserData::toKey: %s -> %x clashes with %s .. FATAL")
					% id % k % i->second << endl;
			exit(EXIT_FAILURE);
		}
	} else {
		//log(qerr, format("UserData::toKey: %s -> %x created\n") % id % k);
		clashMap[k] = id;
	}
	return k;
}
