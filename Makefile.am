bin_PROGRAMS = qhub
qhub_CXXFLAGS = -Wall
qhub_SOURCES  = qhub.cpp qhub.h MyTime.h types.h 
qhub_SOURCES += Util.h Util.cpp string8.h
qhub_SOURCES += Encoder.h Encoder.cpp TigerHash.h TigerHash.cpp
qhub_SOURCES += TokenBucket.h TokenBucket.cpp Socket.h
qhub_SOURCES += Socket.cpp ServerSocket.cpp ServerSocket.h
qhub_SOURCES += DNSUser.h InterHub.cpp InterHub.h
qhub_SOURCES += Hub.h Hub.cpp
qhub_SOURCES += ADCSocket.h ADCSocket.cpp ADC_Attributes.cpp ADC.h ADC.cpp
qhub_SOURCES += Buffer.h
qhub_SOURCES += compat_hash_map.h
qhub_SOURCES += Plugin.h Plugin.cpp
qhub_SOURCES += Settings.h Settings.cpp

EXTRA_DIST = doc/protocol.txt doc/ADC-commands_by_type.txt TODO Settings.xml
EXTRA_DIST += config.sub config.guess foo1.c


lib_LTLIBRARIES = foo1.la

foo1_la_SOURCES = foo1.c
foo1_la_LDFLAGS = -module
