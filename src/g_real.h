#ifndef _G_REAL_H
#define _G_REAL_H

/* We need to include config.h here in order to know about HAVE_LIBQUADMATH */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_LIBQUADMATH

#include <quadmath.h>

typedef __float128 G_REAL;
typedef unsigned long long int G_HUGEINT;
typedef struct {
	G_HUGEINT a;
	G_HUGEINT b;
} G_HUGEINT2;

G_HUGEINT2 greal2hugeint(G_REAL d);
G_REAL hugeint2greal(G_HUGEINT2 h);

#define G_SIN sinq
#define G_COS cosq
#define G_TAN tanq
#define G_ASIN asinq
#define G_ACOS acosq
#define G_ATAN atanq

#define G_SINH sinhq
#define G_COSH coshq
#define G_TANH tanhq
#define G_ASINH asinhq
#define G_ACOSH acoshq
#define G_ATANH atanhq

#define G_EXP expq
#define G_LOG10 log10q
#define G_LOG logq
#define G_POW powq
#define G_SQRT sqrtq

#define G_FLOOR floorq
#define G_FMOD fmodq
#define G_LDEXP ldexpq

#define G_STRTOD strtodq

#define G_LMOD "Q"

#else // HAVE_LIBQUADMATH

typedef double G_REAL;
typedef long long int G_HUGEINT;

#define G_SIN sin
#define G_COS cos
#define G_TAN tan
#define G_ASIN asin
#define G_ACOS acos
#define G_ATAN atan

#define G_SINH sinh
#define G_COSH cosh
#define G_TANH tanh
#define G_ASINH asinh
#define G_ACOSH acosh
#define G_ATANH atanh

#define G_EXP exp
#define G_LOG10 log10
#define G_LOG log
#define G_POW pow
#define G_SQRT sqrt

#define G_FLOOR floor
#define G_FMOD fmod
#define G_LDEXP ldexp

#define G_STRTOD strtod

#define G_LMOD ""

#endif // HAVE_LIBQUADMATH

#endif // G_REAL
