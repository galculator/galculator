dnl Process this file with autoconf to produce a configure script.

AC_INIT([galculator], [2.1.5devel])
AC_CONFIG_SRCDIR([configure.ac])
AM_INIT_AUTOMAKE

dnl AC_INIT(configure.ac)
dnl AM_INIT_AUTOMAKE([galculator], [2.1.5devel])
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_INTLTOOL(, no-xml)
AC_ISC_POSIX

AC_HEADER_STDC

dnl
dnl libm
dnl 

AC_CHECK_LIBM
AC_SUBST(LIBM)

dnl
dnl GTK2 or GTK 3
dnl 

AC_ARG_ENABLE([gtk3],
    [AC_HELP_STRING([--enable-gtk3],
        [compile galculator against gtk+ 3.0 (default: yes)])],
    [enable_gtk3="${enableval}"],
    [enable_gtk3="yes"]
)

if test x"$enable_gtk3" = x"yes"; then
    pkg_modules="gtk+-3.0"
else
    pkg_modules="gtk+-2.0 >= 2.16.0"
fi

PKG_CHECK_MODULES(GTK, [$pkg_modules])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl
dnl double- or quad-precision arithmetics
dnl

AC_ARG_ENABLE([quadmath],
    [AC_HELP_STRING([--enable-quadmath],
        [enable support for quad-precision floating point arithmetics (default: yes)])],
    [enable_quadmath="${enableval}"],
    [enable_quadmath="yes"]
)

if test x"$enable_quadmath" = x"yes"; then
    dnl If the check succeeds, -lquadmath is added to LIBS and we get a 
	dnl HAVE_LIBQUADMATH defined
	AC_CHECK_LIB(quadmath,sinhq)
fi

dnl
dnl (f)lex
dnl
AM_PROG_LEX

AM_PROG_CC_C_O

GETTEXT_PACKAGE=galculator
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of gettext package])
AC_SUBST(GETTEXT_PACKAGE)

AM_PROG_LIBTOOL

dnl Add the languages which your application supports here.
ALL_LINGUAS="am ar be bg cs da de el en_AU en_GB es_MX es et eu fi fr gl hr hu id it ja kk ko lt mr ms nl pl pt_BR pt ro ru sk sl sq sr sv tr uk zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT

dnl If we don't have msgfmt, then po/ is going to fail -- ensure that
dnl AM_GLIB_GNU_GETTEXT found it (this is from the libpurple project)

if test x$MSGFMT = xno
then
        AC_ERROR([

The msgfmt command is required to build galculator.  If it is installed
on your system, ensure that it is in your path.  If it is not, install
GNU gettext to continue.
])
fi

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

dnl m4/Makefile
AC_OUTPUT([Makefile
galculator.spec
src/Makefile
po/Makefile.in 
ui/Makefile
ui/about.ui
doc/Makefile
pixmaps/Makefile
])
