// vim:ts=4:sw=4:noet
#ifndef QHUB_USERDATA_H
#define QHUB_USERDATA_H

#include "qhub.h"
#include "Util.h"

#include <map>
#include <string>

namespace qhub {

class UserData {
public:
	typedef std::string string;
	typedef std::string key_type;

#define GET_AND_SET(type, Type) \
public: \
	/* If setting something equal to emptyType, delete it instead */ \
	void set##Type(key_type key, type value) throw() { \
		Type##Map::iterator i = type##Map.find(key); \
		if(i != type##Map.end()) { \
			if(value == Util::empty##Type) \
				type##Map.erase(i); \
			else \
				type##Map[key] = value; \
		} else { \
			if(value != Util::empty##Type) \
				 type##Map[key] = value; \
		} \
	} \
	/* Return set value or emptyType when not found */ \
	type const& get##Type(key_type key) const throw() { \
		Type##Map::const_iterator i = type##Map.find(key); \
		if(i != type##Map.end()) \
			return i->second; \
		return Util::empty##Type; \
	} \
private: \
	typedef std::map<key_type, type> Type##Map; \
	Type##Map type##Map;
			 
	GET_AND_SET(int, Int)
	GET_AND_SET(string, String)
	GET_AND_SET(voidPtr, VoidPtr)

#undef GET_AND_SET
};	

} //namespace qhub

#endif //QHUB_USERDATA_H
