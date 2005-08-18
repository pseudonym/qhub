// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_STD_STRING8_H_
#define _INCLUDED_STD_STRING8_H_

#include <sys/types.h>
#include <string>

namespace std {

template<> struct char_traits<u_int8_t> {
	typedef u_int8_t char_type;
	static char_type* copy(char_type* __s1, char_type const* __s2, size_t __n) {
		return static_cast<char_type*>(memcpy(__s1, __s2, __n));
	}
	static char_type* move(char_type* __s1, char_type const* __s2, size_t __n) {
		return static_cast<char_type*>(memmove(__s1, __s2, __n));
	}
	static void assign(char_type& c1, const char_type& c2) {
		c1 = c2;
	}
};

typedef basic_string<u_int8_t, char_traits<u_int8_t>, std::allocator<u_int8_t> > string8;

} //namespace std

#endif //_INCLUDED_STD_STRING8_H_
