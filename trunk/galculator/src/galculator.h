/*
 *  galculator.h - general definitions.
 *	part of galculator
 *  	(c) 2002-2004 Simon Floery (chimaira@users.sf.net)
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

#define CLEARED_DISPLAY	"0"

#define CONFIG_FILE_NAME ".galculator"

#define MAIN_GLADE_FILE 		PACKAGE_GLADE_DIR "/main_frame.glade"
#define SCIENTIFIC_GLADE_FILE		PACKAGE_GLADE_DIR "/scientific_buttons.glade"
#define BASIC_GLADE_FILE		PACKAGE_GLADE_DIR "/basic_buttons.glade"
#define ABOUT_GLADE_FILE 		PACKAGE_GLADE_DIR "/about.glade"
#define PREFS_GLADE_FILE 		PACKAGE_GLADE_DIR "/prefs.glade"
#define FONT_GLADE_FILE 		PACKAGE_GLADE_DIR "/font.glade"
#define COLOR_GLADE_FILE 		PACKAGE_GLADE_DIR "/color.glade"
#define DISPCTRL_RIGHT_GLADE_FILE	PACKAGE_GLADE_DIR "/dispctrl_right.glade"
#define DISPCTRL_RIGHTV_GLADE_FILE	PACKAGE_GLADE_DIR "/dispctrl_right_vertical.glade"
#define DISPCTRL_BOTTOM_GLADE_FILE	PACKAGE_GLADE_DIR "/dispctrl_bottom.glade"

#define MY_INFINITY_STRING "inf"

/* i18n */

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

/* also change this in calc_basic.h */
#ifndef BUG_REPORT
	#define BUG_REPORT	_("Please submit a bugreport.")
#endif

/* if we do not get infinity from math.h, we try to define it by ourselves */
#include <math.h>
#ifndef INFINITY
	#define INFINITY 1.0 / 0.0
#endif

#ifndef PROG_NAME
	#define PROG_NAME	PACKAGE
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
	CS_PAN,			/* _P_seudo _A_lgebraic _N_otation */
	CS_RPN,			/* reverse polish notation */
	CS_FORMULA,		/* formula entry */
	NR_NOTATION_MODES
};

enum {
	CS_FMOD_FLAG_INV,
	CS_FMOD_FLAG_HYP
};

enum {
	BASIC_MODE,
	SCIENTIFIC_MODE,
	NR_MODES
};

enum {
	NAME_COLUMN,
	VALUE_COLUMN,
	DESC_COLUMN,
	NR_CONST_COLUMNS
};

enum {
	DISPCTRL_NONE,
	DISPCTRL_RIGHT,
	DISPCTRL_RIGHTV,
	DISPCTRL_BOTTOM,
	NR_DISPCTRL_LOCS
};

typedef struct {
	unsigned	number:2;
	unsigned	angle:2;
	unsigned	notation:2;
	unsigned	fmod:2;
	gboolean	calc_entry_start_new;
	gboolean	rpn_have_result;
	gboolean	allow_arith_op;
} s_current_status;

typedef struct {
	char		*button_name;
	/* for simplicity we put the display_names not in an array */
	char		*display_names[4];
	double		(*func[4])(double);
	gboolean	is_trigonometric;
} s_function_map;

typedef struct {
	char		*button_name;
	double		constant;
} s_constant_map;

typedef struct {
	char		*button_name;
	char 		*display_name;
	void		(*func)();
} s_gfunc_map;

typedef struct {
	char		*button_name;
	/* display_string: what to display in history or formula entry. hasn't 
	 * to be the button label, e.g. "n!" and "!" 
	 */
	char 		*display_string;
	int		operation;
} s_operation_map;

typedef struct {
	int			x;
	int			y;
} s_point;

typedef struct {
	char		*desc;
	char		*name;
	char		*value;
} s_constant;

typedef struct {
	double		*data;
	int		len;
} s_array;

extern s_array		memory;
#include "config_file.h"
extern s_preferences	prefs;
extern s_constant 	*constant;
extern s_current_status	current_status;

#endif /* galculator.h */
