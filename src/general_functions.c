/*
 *  general_functions.c - this and that.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "galculator.h"
#include "general_functions.h"
#include "math_functions.h"
#include "calc_basic.h"
#include "display.h"
#include "config_file.h"
#include "callbacks.h"
#include "ui.h"

double error_unsupported_inv (double dummy)
{
	error_message (_("unsupported inverse function"));
	return dummy;
}

double error_unsupported_hyp (double dummy)
{
	error_message (_("unsupported hyperbolic function"));
	return dummy;
}

void error_message (char *message)
{	
	fprintf (stderr, "[%s] %s. %s\n", PROG_NAME, message, BUG_REPORT);
}

/* the last entered number is removed. it is impossible to remove the last operation:
 * this would be a quite difficult taks, as every possible computation is done asap.
 * so if you entered 1+2- and you want to correct to 1+2/ calc_basic would have already
 * calculated 1+2=3 and set the tree to 3-. you would have to make a backup of the tree!*/

void clear ()
{
	display_result_set (DEFAULT_REM_VALUE);
}

/* clear all: display ("0"), calc_tree ... */

void all_clear ()
{
	clear();
	if (current_status.notation == CS_PAN) {
		alg_free();
		alg_init(0);
	}
	else {
		rpn_free();
		rpn_init(0);
		current_status.rpn_have_result=FALSE;
	}
	display_module_bracket_label_update (RESET);
}

/* axtof: convert string to float 
 *	works up to base 19. only for integers!
 *  a number is called negative, if its msb is set!
 */

double axtof (char *bin_string, int base, int nr_bits, gboolean is_signed)
{
	double 		return_value=0;
	int		counter;
	
	/* according to man strtod, inf should be there in every case */
	if (strstr (g_ascii_strdown (bin_string, -1), "inf") != NULL) return INFINITY;

	for (counter = strlen (bin_string) - 1; counter >= 0; counter--) {
		if (bin_string[counter] - '0' < 10) \
			return_value += (bin_string[counter] - '0') * pow (base, strlen (bin_string) - 1 - counter);
		else if (bin_string[counter] - 'A' < 10) \
			return_value += (bin_string[counter] - 'A' + 10) * pow (base, strlen (bin_string) - 1 - counter);
		else fprintf (stderr, _("[%s] failed to convert char %c in function \"axtof\". %s\n"), PROG_NAME, bin_string[counter], BUG_REPORT);
	}
	
	/* handle negative numbers. */

	/* if most significant bit is set, its a negative number. using */
	if (is_signed == TRUE) 
		if (return_value >= pow (2, nr_bits - 1))
			return_value = (-1) * (pow (2, nr_bits) - return_value);
	return return_value;
}

/* rem: my own remainder function to deal with precision problems */

int rem (double x, long long int y)
{
	return x - ((long long int)(x/((double)y)))*y;
}

/* ftoax - number to string conversion.
 *	works up to base 19. only for integers!
 *  see axtof for details about what is a negative number.
 */

char *ftoax (double x, int base, int nr_bits, gboolean is_signed)
{
	char		*return_string;
	int		length=0, counter, remainder;
	double		localx;
	
	/* handle huge values --> infinity */
	if (is_signed == TRUE) {
		if (x < (-1)*pow (2, nr_bits - 1)) return g_strdup(MY_INFINITY_STRING);
		if (x >= pow (2, nr_bits - 1)) return g_strdup(MY_INFINITY_STRING);
		/* handle negative numbers */
		if (x < 0) x = pow (2, nr_bits) + x;
	} else {
		if (x >= pow (2, nr_bits)) return g_strdup(MY_INFINITY_STRING);
		if (x < 0) return g_strdup(MY_INFINITY_STRING);
	}
	/* doing it this way and not with logs as this is much more numerical stable */
	localx = x;
	while ((localx=floor(localx/(double)base)) >= 1) length++;
	length+=2;
	
	localx = x;
	return_string = (char *) malloc (length * sizeof(char));
	return_string [length-1] = '\0';
	for (counter = length-2; counter >= 0; counter--) {
		remainder = rem (localx, base);
		if (remainder < 10) return_string[counter] = '0' + remainder;
		else if (remainder < 20) return_string[counter] = 'A' + remainder - 10;
		else fprintf (stderr, _("[%s] failed to convert %f in function \"ftoax\". %s\n"), PROG_NAME, x, BUG_REPORT);
		localx = floor (localx / (double)base);
	}
	return return_string;
}

char *add_leading_zeros (char *string, int multiple)
{
	char	*new_string;
	int	length, offset, counter;
	
	length = strlen(string);
	offset = (multiple - length%multiple)%multiple;
	length += offset;
	new_string = (char *) malloc ((length + 1) * sizeof(char));
	for (counter = 0; counter < offset; counter++)
		new_string[counter] = '0';
	for (counter = offset; counter <= length; counter++)
		new_string[counter] = string[counter-offset];
	free (string);
	return new_string;
}

/*
 * preference dialog sets. these are handler for the big configuration struct.
 */

void set_button_label (GladeXML *xml, char *button_name, void *new_label)
{
	GtkButton	*button;
	char 		**string_var;
	
	string_var = new_label;	
	button = (GtkButton *) glade_xml_get_widget (xml, button_name);
	gtk_button_set_label (button, *string_var);	
}

void set_checkbutton (GladeXML *xml, char *checkbutton_name, void *is_active)
{
	GtkToggleButton		*toggle_button;
	gboolean		*bool_var;
	
	bool_var = is_active;
	toggle_button = (GtkToggleButton *) glade_xml_get_widget (xml, checkbutton_name);	
	gtk_toggle_button_set_active (toggle_button, *bool_var);
}

void set_spinbutton (GladeXML *xml, char *spinbutton_name, void *value)
{
	GtkSpinButton	*spin_button;
	int		*int_var;
	double		d_var;
	
	int_var = value;
	d_var = (double) *int_var;
	spin_button = (GtkSpinButton *) glade_xml_get_widget (xml, spinbutton_name);
	//gtk_spin_button_set_value (spin_button, 1.0);
}

void set_optmenu (GladeXML *xml, char *optmenu_name, void *index)
{
	GtkOptionMenu	*opt_menu;
	int		*int_var;
	
	int_var = index;	
	opt_menu = (GtkOptionMenu *) glade_xml_get_widget (xml, optmenu_name);
	gtk_option_menu_set_history (opt_menu, *int_var);
}

/*
 * The callback fills the given widget with a rectangle in its foreground color.
 */

gboolean da_expose_event_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	gdk_draw_rectangle (widget->window, widget->style->fg_gc[0], TRUE, 0, 0,\
		widget->parent->allocation.width, widget->parent->allocation.height);
	return TRUE;
}

/*
 * assembling my own colored button. as gnomecolorpicker is in libgnomeui, i do
 * it myself. remove the current label widget in button and put there a 
 * gtkdrawingarea. the drawing is done in da_expose_event_cb. The color is
 * set via widget->style->"fgs"
 */

void set_button_color (GladeXML *xml, char *button_name, void *color_string)
{
	GtkWidget	*da;
	char 		**string_var;
	GdkColor	color;

	// dereference
	string_var = color_string;
	gdk_color_parse (*string_var, &color);
	
	da = glade_xml_get_widget (xml, button_name);

	gtk_widget_modify_fg (da, 0, &color);
	
	g_signal_connect (G_OBJECT (da), "expose_event", G_CALLBACK (da_expose_event_cb), NULL);
}

/*
 * convert given GdkColor to a string so that gdk_color_parse gives the 
 * same color again.
 */

char *gdk_color_to_string (GdkColor color)
{
	return g_strdup_printf ("#%04X%04X%04X", color.red, color.green, color.blue);
}

/*
 * "apply"
 */

void apply_preferences (s_preferences prefs)
{
	GtkWidget	*menu_item;
	char		*button_font;

	display_update_tags ();
	display_set_bkg_color (prefs.bkg_color);

	set_all_buttons_size (prefs.button_width, prefs.button_height);

	if (prefs.custom_button_font == TRUE) button_font = g_strdup (prefs.button_font);
	else button_font = g_strdup ("");
	set_all_buttons_font (button_font);
	g_free (button_font);

	set_widget_visibility (main_window_xml, "menubar", prefs.show_menu);
	menu_item = glade_xml_get_widget (main_window_xml, "show_menubar1");
	gtk_check_menu_item_set_active ((GtkCheckMenuItem *) menu_item, prefs.show_menu);

	if (prefs.mode == BASIC_MODE) menu_item = 
		glade_xml_get_widget (main_window_xml, "basic_mode");
	else if (prefs.mode == SCIENTIFIC_MODE) menu_item = 
		glade_xml_get_widget (main_window_xml, "scientific_mode");
	gtk_menu_item_activate ((GtkMenuItem *) menu_item);
}

/*
 * this function changes the foreground color for all states to the given string
 */

void gtk_widget_really_modify_fg (GtkWidget *widget, GdkColor color)
{
	gtk_widget_modify_fg (widget, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_fg (widget, GTK_STATE_ACTIVE, &color);
	gtk_widget_modify_fg (widget, GTK_STATE_PRELIGHT, &color);
	gtk_widget_modify_fg (widget, GTK_STATE_SELECTED, &color);
	gtk_widget_modify_fg (widget, GTK_STATE_INSENSITIVE, &color);
}

gboolean is_valid_number (int number_base, char number)
{
	char *valid_numbers[4]={"1234567890", "1234567890abcdef", "12345670", "01"};
	
	return ((strchr (valid_numbers[number_base], g_ascii_tolower (number)) != NULL) \
		|| (number == dec_point[0]));
}

/*
 * activate_menu_item - activates menu item with widget name item_name
 */

void activate_menu_item (char *item_name)
{
	GtkMenuItem		*current_item;
	
	current_item = (GtkMenuItem *) glade_xml_get_widget (main_window_xml, \
		g_strstrip (g_ascii_strdown (item_name, -1)));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem *) current_item, FALSE);
	gtk_menu_item_activate (current_item);
}
