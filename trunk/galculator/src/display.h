/*
 *  display.h
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
 
#ifndef _DISPLAY_H
#define _DISPLAY_H 1

#define DISPLAY_RESULT_PRECISION	12
#define DISPLAY_RESULT_E_LENGTH		3

#define DISPLAY_MARK_NUMBER			"mark_number"
#define DISPLAY_MARK_ANGLE			"mark_angle"
#define DISPLAY_MARK_NOTATION		"mark_notation"
#define DISPLAY_MARK_ARITH			"mark_arith"
#define DISPLAY_MARK_BRACKET		"mark_bracket"

#define DISPLAY_MODULES_DELIM 		"   "

enum {
	DISPLAY_OPT_NUMBER,
	DISPLAY_OPT_ANGLE,
	DISPLAY_OPT_NOTATION
};

enum {
	ONE_MORE,
	ONE_LESS,
	RESET,
	GET,
	NOP
};

/* general */

void activate_menu_item (char *item_name);
void display_init (GtkWidget *a_parent_widget);
void display_update_modules ();
void display_option_label_set (GtkLabel *label);
void display_option_label_unset (GtkLabel *label);
void display_change_option (int new_status, int new_group);
void display_option_cb (GtkWidget *widget, GdkEventButton *event, gpointer label_text);
void display_set_bkg_color (char *color_string);
void display_update_tags ();
void display_module_arith_label_update (char operation);
int display_module_bracket_label_update (int option);

/* the result field */

void display_result_add_digit (char digit);
void display_result_set (char *string_value);
void display_result_set_double (double value);
void display_result_set_angle (double value);
char *display_result_get ();
double display_result_get_as_double ();
double display_result_get_rad_angle ();
void display_append_e ();
void display_result_toggle_sign ();
void display_result_backspace ();

#endif /* display.h */
