AC_INIT(qhub, 0.6)
AC_CONFIG_SRCDIR(qhub.cpp)
AM_INIT_AUTOMAKE(qhub, 0.6)
AC_PROG_CXX

AC_LANG([C++])

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL


AM_CONFIG_HEADER(config.h)

AC_CHECK_LIB(ltdl, lt_dlinit,,[AC_MSG_ERROR([libltdl needed - lt_dlopen etc..])])

# Check for libxmltok or libexpat
ac_xmllib=""
AC_CHECK_LIB(expat, XML_ParserCreate, [ac_xmllib="expat" LIBS="-lexpat $LIBS"])
if test "$ac_xmllib" = ""; then
  AC_CHECK_LIB([xmltok], main, [])
  AC_CHECK_LIB([xmlparse], XML_ParserCreate, [ac_xmllib="xmltok" LIBS="-lxmlparse -lxmltok $LIBS"])
  if test "$ac_xmllib" = ""; then
    AC_MSG_ERROR(libexpat (http://expat.sourceforge.net) or libxmltok required)
  fi
fi

AC_CHECK_LIB(oop, oop_sys_new,,[AC_MSG_ERROR([liboop needed - http://liboop.org/])])
AC_CHECK_LIB(oop-adns, oop_adns_new,,[AC_MSG_ERROR([liboop needed - http://liboop.org/])])
AC_CHECK_LIB(adns, adns_init,,[AC_MSG_ERROR([libadns needed - http://www.chiark.greenend.org.uk/~ian/adns/])])

AC_CHECK_LIB(event, event_init)
AC_CHECK_LIB(oop-event, oop_event_new)

AC_CHECK_LIB(socket, connect)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(resolv, getaddrinfo_a)



# We need to find one working header file out of a bunch.
# CHECK_HEADERS is not good enough, because it includes all headers
# that exists, but we want only those that work.

# Therefore, use CHECK_HEADER (autoheader will not put that into
# config.h.in) and stick the result into CPPFLAGS as a -D option.

tmp_headers=""
for hdr in hash_map ext/hash_map; do
  AC_CHECK_HEADER($hdr, tmp_headers="$tmp_headers $hdr")
done

AC_CACHE_CHECK(whether any hash_map header works,
 cvc_cv_hash_map_works,
 cvc_cv_hash_map_works="no"
 for hdr in $tmp_headers; do
  for ns in std __gnu_cxx; do
   if test "$cvc_cv_hash_map_works" = "no"; then
    AC_TRY_LINK([
#include<$hdr>
],
[$ns::hash_map<int, unsigned> x;]
 , cvc_cv_hash_map_works="$ns::$hdr")
   fi
  done
 done)

if test "$cvc_cv_hash_map_works" = "no"; then
  AC_MSG_ERROR(working hash_map header is not found)
else
  CPPFLAGS="$CPPFLAGS -DHAVE_"`echo $cvc_cv_hash_map_works | sed 'y%abcdefghijklmnopqrstuvwxyz.:/-%ABCDEFGHIJKLMNOPQRSTUVWXYZ____%'`
fi





##check for functions

AC_CHECK_FUNCS([gethostname strtol gettimeofday gethostbyname inet_ntoa memmove memset socket getaddrinfo_a])
AC_CHECK_FUNCS([nanosleep])
AC_CHECK_FUNCS([inet_pton inet_ntop])

if test "$ac_xmllib" = "expat"; then
  AC_CHECK_HEADERS([expat.h],, [AC_MSG_ERROR(libexpat header missing)])
else if test "$ac_xmllib" = "xmltok"; then
  AC_CHECK_HEADERS([xmltok/xmlparse.h],, [AC_MSG_ERROR(libxmltok header missing)])
fi fi	

AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h]) 
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h]) 
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([event.h])
AC_CHECK_HEADERS([oop.h],,[AC_MSG_ERROR([liboop required.])])
AC_CHECK_HEADERS([oop-event.h])
AC_CHECK_HEADERS([adns.h],,[AC_MSG_ERROR([ADNS required.])])
#AC_CHECK_HEADERS([oop-adns.h],,[AC_MSG_ERROR([Liboop-ADNS adapter required.])])
AC_CHECK_HEADER(boost/shared_ptr.hpp,,[AC_MSG_ERROR([Boost library required - http://boost.org/])])
AC_C_INLINE
AC_C_CONST

AC_FUNC_MALLOC
AC_FUNC_VPRINTF

AC_STRUCT_TM
AC_TYPE_SIGNAL
AC_HEADER_TIME
#AC_HEADER_STDBOOL
AC_HEADER_STDC

AC_OUTPUT(Makefile)

