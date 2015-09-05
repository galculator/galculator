## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS=-I m4

desktopdir = $(datadir)/applications
desktop_in_files = galculator.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_in_files = galculator.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

CLEANFILES = $(desktop_DATA) $(appdata_DATA)

SUBDIRS = src po ui doc pixmaps

# things we want to include in the source code distribution

EXTRA_DIST = galculator.spec \
	galculator.spec.in \
	galculator.desktop.in \
	intltool-merge.in \
	intltool-extract.in \
	intltool-update.in \
	galculator.appdata.xml.in \
	ABOUT-NLS

