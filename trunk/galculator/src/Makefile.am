## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-DPACKAGE_GLADE_DIR=\""$(datadir)/galculator/glade"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = galculator

galculator_SOURCES = \
	main.c galculator.h \
	math_functions.c math_functions.h \
	display.c display.h \
	general_functions.c general_functions.h \
	calc_basic.c calc_basic.h \
	config_file.c config_file.h \
	callbacks.c callbacks.h \
	ui.c ui.h \
	flex_parser.l flex_parser.h

galculator_LDADD = @PACKAGE_LIBS@

galculator_LDFLAGS = -export-dynamic

