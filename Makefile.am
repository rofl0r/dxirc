SUBDIRS = intl po m4 src icons icons/default icons/flags icons/smileys sounds
DIST_SUBDIRS = intl po m4 src icons icons/default icons/flags icons/smileys sounds

rcdir = $(prefix)/share/dxirc

EXTRA_DIST = config.h\
             ABOUT-NLS TODO BUGS dxirc.spec.in dxirc.desktop *.png\
             $(top_srcdir)/win/* $(top_srcdir)/icons/*.png $(top_srcdir)/icons/default/*.png $(top_srcdir)/icons/flags/*.png\
             $(top_srcdir)/debian/* $(top_srcdir)/sounds/*.wav $(top_srcdir)/icons/smileys/*.png $(top_srcdir)/icons/smileys/dxirc.smiley

icondir = $(prefix)/share/pixmaps
icon_DATA = dxirc.png

util_DATA = dxirc.desktop
utildir = $(datadir)/applications

dist-hook: dxirc.spec
	cp dxirc.spec $(distdir)
	
uninstall-hook:
	rm -rf $(DESTDIR)$(rcdir)

ACLOCAL_AMFLAGS = -I m4
