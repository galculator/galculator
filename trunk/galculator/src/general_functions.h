/*
 *  general_functions.h
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

#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H 1

#include "config_file.h"

#include <glade/glade.h>

#define BIT(val, index) ((val & (1 << index)) >> index)

void statusbar_init (GtkWidget *a_parent_widget);
double error_unsupported_inv (double dummy);
double error_unsupported_hyp (double dummy);
void error_message (char *message);
void clear ();
void all_clear ();

void set_button_group_size (GladeXML *xml, char *table_name, int width, int height);
void set_button_group_font (GladeXML *xml, char *table_name, char *font_string);

double axtof (char *bin_string, int base, int dlength);
char *ftoax (double x, int base, int dlength);

gboolean da_expose_event_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data);

void set_button_label (GladeXML *xml, char *button_name, void *new_label);
void set_checkbutton (GladeXML *xml, char *checkbutton_name, void *checked);
void set_spinbutton (GladeXML *xml, char *spinbutton_name, void *value);
void set_optmenu (GladeXML *xml, char *optmenu_name, void *index);
void set_button_color (GladeXML *xml, char *button_name, void *color_string);

char *gdk_color_to_string (GdkColor color);
GtkWidget *show_font_dialog (char *title, GtkButton *button);
GtkWidget *show_color_dialog (char *title, GtkButton *button);

void update_all (s_preferences prefs);
void set_widget_visibility (GladeXML *xml, char *widget_name, gboolean visible);

void gtk_widget_really_modify_fg (GtkWidget *widget, GdkColor color);

void update_active_buttons (GladeXML *xml, int number_base, int notation_mode);

gboolean is_valid_number (int number_base, char number);
void button_activation (GtkButton *b);
gboolean button_deactivation (gpointer data);

void set_object_data (GladeXML *xml);

void position_menu (GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data);
GtkWidget *create_constants_menu (s_constant *constant, GCallback const_handler);
GtkWidget *create_memory_menu (s_array memory, GCallback const_handler, char *last_item);

#endif /* general_functions.h */
