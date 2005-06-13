bin_PROGRAMS = qhub
qhub_CXXFLAGS = -Wall -DDEBUG -g -DCONFIGDIR=\"$(pkgsysconfdir)\"
qhub_LDFLAGS = -export-dynamic
qhub_SOURCES  = qhub.cpp qhub.h MyTime.h types.h 
qhub_SOURCES += Util.h Util.cpp string8.h
qhub_SOURCES += Encoder.h Encoder.cpp TigerHash.h TigerHash.cpp
qhub_SOURCES += TokenBucket.h TokenBucket.cpp Socket.h
qhub_SOURCES += Socket.cpp ServerSocket.cpp ServerSocket.h
qhub_SOURCES += DNSUser.h InterHub.cpp InterHub.h
qhub_SOURCES += Hub.h Hub.cpp
qhub_SOURCES += ADCSocket.h ADCSocket.cpp ADCClient.h ADCClient.cpp
qhub_SOURCES += UserInfo.h ADC.h ADC.cpp
qhub_SOURCES += Buffer.h
qhub_SOURCES += compat_hash_map.h error.h Logs.h Logs.cpp
qhub_SOURCES += Plugin.h Plugin.cpp
qhub_SOURCES += UserData.h UserData.cpp XmlTok.h XmlTok.cpp
qhub_SOURCES += Settings.h Settings.cpp
qhub_SOURCES += Timer.h Timer.cpp

EXTRA_DIST = doc/protocol.txt doc/ADC-commands_by_type.txt TODO
EXTRA_DIST += config.sub config.guess README.cygwin

pkgsysconfdir = $(sysconfdir)/$(PACKAGE)

dist_pkgsysconf_DATA = Settings.xml

SUBDIRS = plugins
