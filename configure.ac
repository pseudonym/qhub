AC_INIT(qhub, 0.1)
AC_CONFIG_SRCDIR(qhub.cpp)
AM_INIT_AUTOMAKE
AC_PROG_CXX

AC_LANG(C++)

AM_CONFIG_HEADER(config.h)


AC_CHECK_LIB(socket, connect)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(resolv, getaddrinfo_a)

AC_CHECK_LIB(event, event_init)

AC_CHECK_LIB(oop, oop_sys_new)
AC_CHECK_LIB(adns, adns_init)
AC_CHECK_LIB(oop-adns, oop_adns_new)
AC_CHECK_LIB(oop-event, oop_event_new)


##check for functions

AC_CHECK_FUNCS([gethostname strtol gettimeofday gethostbyname inet_ntoa memmove memset socket getaddrinfo_a])
AC_CHECK_FUNCS([nanosleep])

AC_CHECK_HEADER(event.h)

AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h]) 
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h]) 
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([event.h],,[AC_MSG_ERROR([Libevent is required, sorry.])])
AC_C_INLINE
AC_C_CONST

AC_FUNC_MALLOC
AC_FUNC_VPRINTF

AC_STRUCT_TM
AC_TYPE_SIGNAL
AC_HEADER_TIME
AC_HEADER_STDBOOL
AC_HEADER_STDC

AC_OUTPUT(Makefile)

