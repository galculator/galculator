/*
 *  display.h
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
 
#ifndef _DISPLAY_H
#define _DISPLAY_H 1

#define DISPLAY_RESULT_PRECISION	12
#define DISPLAY_RESULT_E_LENGTH		3

#define DISPLAY_MARK_NUMBER		"mark_number"
#define DISPLAY_MARK_ANGLE		"mark_angle"
#define DISPLAY_MARK_NOTATION		"mark_notation"
#define DISPLAY_MARK_ARITH		"mark_arith"
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

extern gboolean calc_entry_start_new;

/* general */


gboolean on_textview_button_press_event (GtkWidget *widget,
                                        GdkEventButton *event,
                                        gpointer user_data);
void on_textview_selection_received (GtkWidget *widget,
                                            GtkSelectionData *data,
                                            guint time,
                                            gpointer user_data);
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
void display_module_number_activate (int number_base);
void display_module_angle_activate (int angle_unit);
void display_module_notation_activate (int mode);

/* the result field */

void display_result_add_digit (char digit);
void display_result_set (char *string_value);
void display_result_set_double (double value);
void display_result_feed (char *string);
char *display_get_line (int line_nr);
char *display_result_get ();
double display_result_get_double ();
void display_append_e (GtkToggleButton *button);
void display_result_toggle_sign (GtkToggleButton *button);
void display_result_backspace ();

void display_set_line_double (double value, int line, char *tag);
void display_set_line (char *string, int line, char *tag);

void display_stack_create ();
void display_stack_remove ();
void display_stack_set_yzt (char **stack);
void display_stack_set_yzt_double (double *stack);
char **display_stack_get_yzt ();
double *display_stack_get_yzt_double ();

#endif /* display.h */
