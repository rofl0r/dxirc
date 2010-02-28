SUBDIRS = intl po m4 src icons sounds
DIST_SUBDIRS = intl po m4 src icons icons/default icons/flags sounds

rcdir = $(prefix)/share/dxirc

EXTRA_DIST = config.h\
             ABOUT-NLS TODO BUGS dxirc.spec.in dxirc.desktop *.png\
             $(top_srcdir)/win/* $(top_srcdir)/icons/*.png $(top_srcdir)/icons/default/*.png $(top_srcdir)/icons/flags/*.png\
             $(top_srcdir)/debian/* $(top_srcdir)/sounds/*.wav

icondir = $(prefix)/share/pixmaps
icon_DATA = dxirc.png

util_DATA = dxirc.desktop
utildir = $(datadir)/applications

dist-hook: dxirc.spec
	cp dxirc.spec $(distdir)
	
uninstall-hook:
	rm -rf $(DESTDIR)$(rcdir)

ACLOCAL_AMFLAGS = -I m4
