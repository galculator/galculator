/*
 *  galculator.h - general definitions.
 *	part of galculator
 *  	(c) 2002-2003 Simon Floery (simon.floery@gmx.at)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _GALCULATOR_H
#define _GALCULATOR_H 1

#include <glib.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define DEFAULT_DEC_POINT '.'

#define CONFIG_FILE_NAME ".galculator"

#define MAIN_GLADE_FILE 	PACKAGE_GLADE_DIR "/main.glade"
#define ABOUT_GLADE_FILE 	PACKAGE_GLADE_DIR "/about.glade"
#define PREFS_GLADE_FILE 	PACKAGE_GLADE_DIR "/prefs.glade"
#define FONT_GLADE_FILE 	PACKAGE_GLADE_DIR "/font.glade"
#define COLOR_GLADE_FILE 	PACKAGE_GLADE_DIR "/color.glade"

#define MY_INFINITY_STRING "inf"

/* i18n */

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

// also change this in calc_basic.h
#ifndef BUG_REPORT
	#define BUG_REPORT	_("Please submit a bugreport.")
#endif

/* if we do not get infinity from math.h, we try to define it by ourselves */
#include <math.h>
#ifndef INFINITY
	#define INFINITY 1.0 / 0.0
#endif

/* CS_xxxx define flags for current_status. */

enum {
	CS_DEC,
	CS_HEX,
	CS_OCT,
	CS_BIN,
	NR_NUMBER_BASES
};

enum {
	CS_DEG,
	CS_RAD,
	CS_GRAD,
	NR_ANGLE_BASES
};

enum {
	CS_PAN,		/* __P__seudo __A__lgebraic __N__otation */
	CS_RPN,		/* reverse polish notation */
	NR_NOTATION_MODES
};

enum {
	CS_FMOD_FLAG_INV,
	CS_FMOD_FLAG_HYP
};

enum {
	NAME_COLUMN,
	VALUE_COLUMN,
	DESC_COLUMN,
	NR_CONST_COLUMNS
};

typedef struct {
	unsigned char	number:2;
	unsigned char	angle:2;
	unsigned char	notation:1;
	unsigned char	fmod:2;
} s_current_status;

typedef struct {
	char		*button_name;
	double		(*func[4])(double);
	gboolean	is_trigonometric;
} s_function_map;

typedef struct {
	char		*button_name;
	double		constant;
} s_constant_map;

typedef struct {
	char		*button_name;
	void		(*func)();
} s_gfunc_map;

typedef struct {
	char		*button_name;
	int		operation;
} s_operation_map;

typedef struct {
	int			x;
	int			y;
} s_point;

typedef struct {
	char		*button_name;
	gboolean	number_active[NR_NUMBER_BASES];
	gboolean	notation_active[NR_NOTATION_MODES];
} s_active_buttons;

typedef struct {
	char		*desc;
	char		*name;
	char		*value;
} s_constant;

typedef struct {
	double		*data;
	int		len;
} s_array;

#endif /* galculator.h */
