lib_LTLIBRARIES = qhub-accounts.la qhub-virtualfs.la qhub-fsutil.la qhub-loader.la
AM_CXXFLAGS = -Wall -DDEBUG -g -DCONFIGDIR=\"$(pkgsysconfdir)\"
pkgsysconfdir = $(sysconfdir)/$(PACKAGE)


##foo1_la_SOURCES = foo1.c
##foo1_la_LDFLAGS = -module

qhub_accounts_la_SOURCES = Accounts.h Accounts.cpp
qhub_accounts_la_LDFLAGS = -module

qhub_virtualfs_la_SOURCES = VirtualFsDir.h VirtualFs.h VirtualFs.cpp
qhub_virtualfs_la_LDFLAGS = -module

qhub_fsutil_la_SOURCES = FsUtil.h FsUtil.cpp
qhub_fsutil_la_LDFLAGS = -module

qhub_loader_la_SOURCES = Loader.h Loader.cpp
qhub_loader_la_LDFLAGS = -module

dist_pkgsysconf_DATA = accounts.xml plugins.xml
