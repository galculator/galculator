#!/bin/sh

aclocal \
  && intltoolize --copy --automake \
  && autoheader \
  && automake --add-missing --gnu --copy \
  && autoconf \
  && ./configure $@
