/*
 *  general_functions.h
 *	part of galculator
 *  	(c) 2002-2005 Simon Floery (chimaira@users.sf.net)
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

#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H 1

#include "config_file.h"
#include "flex_parser.h"

#include <glade/glade.h>

#define BIT(val, index) ((val & (1 << index)) >> index)

void statusbar_init (GtkWidget *a_parent_widget);
double error_unsupported_inv (double dummy);
double error_unsupported_hyp (double dummy);
void error_message (char *format_string, ...);
void clear ();
void backspace ();
void all_clear ();

double axtof (char *bin_string, int base, int nr_bits, gboolean is_signed);
char *ftoax (double x, int base, int nr_bits, gboolean is_signedh);

char *add_leading_zeros (char *string, int multiple);

gboolean da_expose_event_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data);

void set_button_font (GladeXML *xml, char *button_name, void *new_label);
void set_checkbutton (GladeXML *xml, char *checkbutton_name, void *checked);
void set_spinbutton (GladeXML *xml, char *spinbutton_name, void *value);
void set_optmenu (GladeXML *xml, char *optmenu_name, void *index);
void set_button_color (GladeXML *xml, char *button_name, void *color_string);
void set_stacksize (GladeXML *xml, char *name, void *stack_size);
void set_entry (GladeXML *xml, char *entry_name, void *entry_text);

char *gdk_color_to_string (GdkColor color);

void apply_preferences (s_preferences prefs);

void gtk_widget_really_modify_fg (GtkWidget *widget, GdkColor color);

gboolean is_valid_number (int number_base, char number);

void activate_menu_item (char *item_name);

char *get_display_number_string (double value, int base);
int get_display_number_length (int base);

void gfunc_f1 (GtkToggleButton *button);
void gfunc_f2 (GtkToggleButton *button);

void rpn_stack_lift ();

void remember_display_values();

double string2double (char *string, int number_base);

char *string_add_separator (char *string, gboolean separate, int block_length, char separator, char dpoint);
char *string_del_separator (char *string, char separator);

void set_button_label_and_tooltip (GladeXML *xml, char *button_name, 
	char *label, char *tooltip);
	
GtkWidget *formula_entry_is_active (GtkWidget *window_widget);
GtkWidget *formula_entry_is_active_no_toplevel_check ();

s_flex_parser_result compute_user_function (char *expression, char *variable, char *value);

double x2rad (double x);
double rad2x (double x);

gboolean get_sep (int number_base);
int get_sep_length (int number_base);
char get_sep_char (int number_base);

void prefs_sep_char_changed (GtkEditable *editable, char *prefs_sep, int number_base);

void change_option (int new_status, int opt_group);

#endif /* general_functions.h */
