bin_PROGRAMS = qhub
qhub_CXXFLAGS = -Wall -DDEBUG -g -DCONFIGDIR=\"$(sysconfdir)/\"
qhub_LDFLAGS = -export-dynamic -rpath $(libdir)
qhub_SOURCES  = qhub.cpp qhub.h MyTime.h types.h 
qhub_SOURCES += Util.h Util.cpp
qhub_SOURCES += Encoder.h Encoder.cpp TigerHash.h TigerHash.cpp
qhub_SOURCES += TokenBucket.h TokenBucket.cpp Socket.h
qhub_SOURCES += Socket.cpp ServerSocket.cpp ServerSocket.h
qhub_SOURCES += DNSAdapter.h DNSAdapter.cpp InterHub.cpp InterHub.h
qhub_SOURCES += Hub.h Hub.cpp
qhub_SOURCES += ADCSocket.h ADCSocket.cpp ADCClient.h ADCClient.cpp
qhub_SOURCES += UserInfo.h ADC.h ADC.cpp
qhub_SOURCES += Buffer.h
qhub_SOURCES += compat_hash_map.h error.h Logs.h Logs.cpp
qhub_SOURCES += Plugin.h Plugin.cpp
qhub_SOURCES += UserData.h XmlTok.h XmlTok.cpp
qhub_SOURCES += Settings.h Settings.cpp
qhub_SOURCES += Timer.h Timer.cpp
qhub_SOURCES += EventHandler.cpp EventHandler.h

EXTRA_DIST = doc/ADC-commands_by_type.txt TODO Settings.xml
EXTRA_DIST += config.sub config.guess README.cygwin doc/ADC-IHUB

##pkgsysconfdir = $(sysconfdir)/$(PACKAGE_NAME)

##pkgsysconf_DATA = Settings.xml

SUBDIRS = . plugins
