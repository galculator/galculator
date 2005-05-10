#!/bin/sh

aclocal \
  && libtoolize --force --copy \
  && intltoolize --copy --automake \
  && autoheader \
  && automake --add-missing --gnu --copy \
  && autoconf \
  && ./configure $@
