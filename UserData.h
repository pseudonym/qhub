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
	typedef u_int32_t Key;
	static Key toKey(string const& id) throw();

#define GET_AND_SET(type, Type) \
	public: \
	void set##Type(Key key, type value) throw() { type##Map[key]; }; \
	type const& get##Type(Key key) const throw() { \
		Type##Map::const_iterator i = type##Map.find(key); \
		if(i != type##Map.end()) \
			return i->second; \
		return Util::empty##Type; \
	}; \
	private: \
	typedef map<Key, type> Type##Map; \
	Type##Map type##Map;
			 
	GET_AND_SET(int, Int);
	GET_AND_SET(string, String);

#undef GET_AND_SET
	
public:
};	

} //namespace qhub

#endif //_INCLUDED_USERDATA_H_