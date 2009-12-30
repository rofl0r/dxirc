
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
                  logviewer.cpp \
                  tetristabitem.cpp \
                  scriptdialog.cpp \
                  dccdialog.cpp \
		  dxtabbook.cpp

if HAVE_TRAY
dxirc_SOURCES += FXTrayApp.cpp \
                 FXTrayIcon.cpp
endif # HAVE_TRAY

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
              logviewer.h \
              tetristabitem.h \
              scriptdialog.h \
              dccdialog.h \
              dxtabbook \
              FXTrayApp.h \
              FXTrayIcon.h


