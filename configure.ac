#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([qhub],[0.11-pre])
AC_CONFIG_SRCDIR([src/qhub.cpp])
AM_INIT_AUTOMAKE
AC_PROG_CXX

AC_LANG([C++])

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL


AM_CONFIG_HEADER(config.h)


AC_CHECK_LIB(dl, dlopen)

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

AC_CHECK_LIB(cares, ares_init,,[AC_MSG_ERROR([c-ares needed])])
AC_CHECK_LIB(event, event_init,,[AC_MSG_ERROR([libevent needed])])
AC_CHECK_LIB(z, deflate,,[AC_MSG_ERROR([zlib needed])])

AC_CHECK_LIB(socket, connect)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(resolv, getaddrinfo_a)


#sockaddr_in6 support
AC_DEFUN([TYPE_STRUCT_SOCKADDR_IN6],[
  ds6_have_sockaddr_in6=
  AC_CHECK_TYPES([struct sockaddr_in6],[
    ds6_have_sockaddr_in6=yes
  ],[
    ds6_have_sockaddr_in6=no
  ],[
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
  ])

  if test "X$ds6_have_sockaddr_in6" = "Xyes"; then :
    $1
  else :
    $2
  fi
])

ipv6=yes
TYPE_STRUCT_SOCKADDR_IN6(,[
AC_MSG_NOTICE([Disabling IPv6 support: 'struct sockaddr_in6' support is required])
ipv6=no
])

#IPV6 support
AC_DEFUN([PROTO_INET6],[
  AC_CACHE_CHECK([for INET6 protocol support], [ds6_cv_proto_inet6],[
    AC_PREPROC_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/socket.h>

#ifndef PF_INET6
#error Missing PF_INET6
#endif
#ifndef AF_INET6
#error Mlssing AF_INET6
#endif
    ]])],[
      ds6_cv_proto_inet6=yes
    ],[
      ds6_cv_proto_inet6=no
    ])
  ])

  if test "X$ds6_cv_proto_inet6" = "Xyes"; then :
    $1
  else :
    $2
  fi
])

PROTO_INET6(,[
AC_MSG_NOTICE([Disabling IPv6 support: INET6 protocol support is required])
ipv6=no
])

if test "X$ipv6" = "Xyes"; then
  AC_DEFINE([ENABLE_IPV6], 1, [Define if IPv6 support is enabled.])
fi

# see if we have unordered_(map|set), either normally or as part of tr1
AC_CXX_HEADER_UNORDERED_MAP
AC_CXX_HEADER_UNORDERED_SET
AC_CXX_HEADER_TR1_UNORDERED_MAP
AC_CXX_HEADER_TR1_UNORDERED_SET

##check for functions

AC_CHECK_FUNCS([gethostname strtol gettimeofday gethostbyname inet_ntoa memmove memset socket getaddrinfo_a])
AC_CHECK_FUNCS([nanosleep strerror])
AC_CHECK_FUNCS([inet_pton inet_ntop])

if test "$ac_xmllib" = "expat"; then
  AC_CHECK_HEADERS([expat.h],, [AC_MSG_ERROR(libexpat header missing)])
else if test "$ac_xmllib" = "xmltok"; then
  AC_CHECK_HEADERS([xmltok/xmlparse.h],, [AC_MSG_ERROR(libxmltok header missing)])
fi fi

AC_CHECK_HEADERS([event.h])
AC_CHECK_HEADERS([ares.h])
AC_CHECK_HEADERS([boost/shared_ptr.hpp boost/format.hpp],
		,[AC_MSG_ERROR([Boost library required - http://boost.org/])])
##AC_CHECK_LIB(boost_program_options,,,[AC_MSG_ERROR([boost::program_options needed])])

AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])

AC_C_INLINE
AC_C_CONST

AC_STRUCT_TM
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_HEADER_STDBOOL
AC_HEADER_STDC

AC_CONFIG_FILES([Makefile src/Makefile plugins/Makefile])
AC_OUTPUT
