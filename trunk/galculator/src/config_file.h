/*
 *  config_file.h - header file for config_file.c, manages config file access.
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
 
#include <gtk/gtk.h>
#include <glib.h>
#include <glade/glade.h>

#include "galculator.h"

#ifndef _CONFIG_FILE_H
#define _CONFIG_FILE_H 1

#define MAX_FILE_LINE_LENGTH	1024

#define SECTION_GENERAL "[general]"
#define SECTION_CONSTANTS "[constants]"

#define DEFAULT_BKG_COLOR			"#e6edbd"
#define DEFAULT_RESULT_FONT			"Sans Bold 26"
#define	DEFAULT_RESULT_COLOR 		"black"
#define DEFAULT_MOD_FONT			"Sans Bold 9"
#define DEFAULT_ACT_MOD_COLOR		"black"
#define DEFAULT_INACT_MOD_COLOR		"grey"
#define	DEFAULT_VIS_NUMBER			TRUE
#define	DEFAULT_VIS_ANGLE			TRUE
#define	DEFAULT_VIS_NOTATION		TRUE
#define DEFAULT_VIS_ARITH			TRUE
#define DEFAULT_VIS_BRACKET			TRUE
#define DEFAULT_CUSTOM_BUTTON_FONT	FALSE
#define DEFAULT_BUTTON_FONT			"Sans 10"
#define DEFAULT_BUTTON_WIDTH 		40
#define DEFAULT_BUTTON_HEIGHT 		25
#define DEFAULT_VIS_FUNCS			TRUE
#define DEFAULT_VIS_LOGIC			TRUE
#define DEFAULT_VIS_DISPCTRL		TRUE
#define DEFAULT_NUMBER				CS_DEC
#define DEFAULT_ANGLE				CS_RAD
#define DEFAULT_NOTATION			CS_PAN
#define DEFAULT_REM_DISPLAY			FALSE
#define	DEFAULT_REM_VALUE			"0"			// must not end with a newline!
#define DEFAULT_SHOW_MENU			TRUE

typedef struct {
	// 1st pref page
	char 		*bkg_color;		// gdk_color_parse
	char		*result_font; 	// pango_font_description_from_string
	char 		*result_color;
	char		*mod_font;
	char 		*act_mod_color;
	char 		*inact_mod_color;
	gboolean	vis_number;
	gboolean	vis_angle;
	gboolean	vis_notation;
	gboolean	vis_arith;
	gboolean	vis_bracket;
	// 2nd pref page
	gboolean	custom_button_font;
	char 		*button_font;		// buttons
	int		button_width;
	int		button_height;
	gboolean	vis_funcs;
	gboolean	vis_logic;
	gboolean	vis_dispctrl;
	// 3rd pref page
	int		def_number;		// in accordance with enums in
	int		def_angle;		// galculator.h
	int		def_notation;
	gboolean	rem_display;
	char		*rem_value;		// done as string
	gboolean	show_menu;
} s_preferences;

// default value ?
// update_handler

typedef struct {
	char 	*key;
	void	*variable;
	int		key_type;
	char 	*widget_name;
	void	(*set_handler)(GladeXML *, char *, void *);
} s_prefs_entry;

enum {
	STRING,
	BOOLEAN,
	INTEGER,
	NOT_FOUND
};

enum {
	GENERAL,
	CONSTANTS
};

void config_file_read (char *filename);
void config_file_write (char *filename, s_preferences this_prefs);

#endif /* config_file.h */
