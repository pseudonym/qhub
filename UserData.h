// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_USERDATA_H_
#define _INCLUDED_USERDATA_H_

#include <map>
#include <string>
#include "Util.h"

using namespace std;

namespace qhub {

class UserData {
public:
	typedef string key_type;

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
	typedef map<key_type, type> Type##Map; \
	Type##Map type##Map;
			 
	GET_AND_SET(int, Int)
	GET_AND_SET(string, String)
	GET_AND_SET(voidPtr, VoidPtr)

#undef GET_AND_SET
};	

} //namespace qhub

#endif //_INCLUDED_USERDATA_H_
