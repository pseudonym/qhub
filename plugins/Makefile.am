lib_LTLIBRARIES  = qhub-accounts.la qhub-virtualfs.la qhub-fsutil.la
lib_LTLIBRARIES += qhub-loader.la qhub-bans.la qhub-networkctl.la
AM_CXXFLAGS = -Wall -DDEBUG -g -I../src


qhub_bans_la_SOURCES = Bans.h Bans.cpp
qhub_bans_la_LDFLAGS = -module

qhub_accounts_la_SOURCES = Accounts.h Accounts.cpp
qhub_accounts_la_LDFLAGS = -module

qhub_virtualfs_la_SOURCES = VirtualFsDir.h VirtualFs.h VirtualFs.cpp
qhub_virtualfs_la_LDFLAGS = -module

qhub_fsutil_la_SOURCES = FsUtil.h FsUtil.cpp
qhub_fsutil_la_LDFLAGS = -module

qhub_loader_la_SOURCES = Loader.h Loader.cpp
qhub_loader_la_LDFLAGS = -module

qhub_networkctl_la_SOURCES = NetworkCtl.cpp NetworkCtl.h
qhub_networkctl_la_LDFLAGS = -module
