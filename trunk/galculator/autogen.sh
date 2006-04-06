#!/bin/sh
# use automake v1.9

aclocal \
  && libtoolize --force --copy \
  && intltoolize --force --copy --automake \
  && autoheader \
  && automake --add-missing --gnu --copy \
  && autoconf \
  && ./configure $@
