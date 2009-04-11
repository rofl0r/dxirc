
bin_PROGRAMS = dxirc

dxirc_SOURCES = configdialog.cpp \
			      dxirc.cpp \
			      icons.cpp \
			      ircsocket.cpp \
			      irctabitem.cpp \
			      serverdialog.cpp \
                              utils.cpp \
                              aliasdialog.cpp \
                              dxpipe.cpp \
                              FXTrayApp.cpp \
                              FXTrayIcon.cpp

dxirc_LDADD = @LIBINTL@

localedir = $(datadir)/locale

INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@ \
             -DDXIRC_DATADIR=\"$(datadir)/dxirc\"

EXTRA_DIST =  configdialog.h \
			  defs.h \
			  dxirc.h \
                          help.h \
			  i18n.h \
                          icons.h \
			  ircsocket.h \
			  irctabitem.h \
			  serverdialog.h \
                          utils.h \
                          aliasdialog.h \
                          dxpipe.h \
                          FXTrayApp.h \
                          FXTrayIcon.h


