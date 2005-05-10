plugindir = $(libdir)/qhub

plugin_LTLIBRARIES = accounts.la virtualfs.la fsutil.la loader.la
AM_CXXFLAGS = -Wall -DDEBUG -g

##foo1_la_SOURCES = foo1.c
##foo1_la_LDFLAGS = -module

accounts_la_SOURCES = Accounts.h Accounts.cpp
accounts_la_LDFLAGS = -module
dist_pkgdata_DATA = accounts.xml

virtualfs_la_SOURCES = VirtualFsDir.h VirtualFs.h VirtualFs.cpp
virtualfs_la_LDFLAGS = -module

fsutil_la_SOURCES = FsUtil.h FsUtil.cpp
fsutil_la_LDFLAGS = -module

loader_la_SOURCES = Loader.h Loader.cpp
loader_la_LDFLAGS = -module
