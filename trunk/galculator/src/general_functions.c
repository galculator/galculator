/*
 *  general_functions.c - this and that.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "galculator.h"
#include "general_functions.h"
#include "math_functions.h"
#include "calc_basic.h"
#include "display.h"
#include "config_file.h"
#include "callbacks.h"
#include "ui.h"

#include <gtk/gtk.h>
#include <glade/glade.h>

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
	display_result_set (CLEARED_DISPLAY);
	if (current_status.notation == CS_FORMULA) ui_formula_entry_set ("");
}

/* backspace. if formula_entry is active, backspace works anyway. but if we
 * press GUI's backspace button this gives the expected result.
 */

void backspace ()
{
	if (current_status.notation == CS_FORMULA) ui_formula_entry_backspace();
	else display_result_backspace();
}

/* clear all: display ("0"), calc_tree ... */

void all_clear ()
{
	clear();
	switch (current_status.notation) {
		case CS_PAN:
			alg_free();
			alg_init(0);
			break;
		case CS_RPN:
			rpn_free();
			rpn_init(prefs.stack_size, 0);
			display_stack_remove();
			display_stack_create();
			/* it's a stack lifting disabling functon */
			current_status.rpn_stack_lift_enabled = FALSE;
			break;
		case CS_FORMULA:
			alg_free();
			rpn_free();
			display_stack_remove();
			break;
		default:
			fprintf (stderr, _("[%s] unknown notation mode in function \"all_clear\". %s\n"), PROG_NAME, BUG_REPORT);
			
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
	
	/* I don't want "0" to become "000000000" or whatever */
	if (strcmp (string, "0") == 0) return string;
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
	if (button) gtk_button_set_label (button, *string_var);	
}

void set_checkbutton (GladeXML *xml, char *checkbutton_name, void *is_active)
{
	GtkToggleButton		*toggle_button;
	gboolean		*bool_var;
	
	bool_var = is_active;
	toggle_button = (GtkToggleButton *) glade_xml_get_widget (xml, checkbutton_name);	
	if (toggle_button) gtk_toggle_button_set_active (toggle_button, *bool_var);
}

void set_spinbutton (GladeXML *xml, char *spinbutton_name, void *value)
{
	GtkSpinButton	*spin_button;
	int		*int_var;
	
	int_var = value;
	/*d_var = (double) *int_var;*/
	spin_button = (GtkSpinButton *) glade_xml_get_widget (xml, spinbutton_name);
	if (spin_button) gtk_spin_button_set_value (spin_button, *int_var);
}

void set_optmenu (GladeXML *xml, char *optmenu_name, void *index)
{
	GtkOptionMenu	*opt_menu;
	int		*int_var;
	
	int_var = index;	
	opt_menu = (GtkOptionMenu *) glade_xml_get_widget (xml, optmenu_name);
	if (opt_menu) gtk_option_menu_set_history (opt_menu, *int_var);
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

	/* dereference */
	string_var = color_string;
	gdk_color_parse (*string_var, &color);
	
	da = glade_xml_get_widget (xml, button_name);

	gtk_widget_modify_fg (da, 0, &color);
	
	g_signal_connect (G_OBJECT (da), "expose_event", G_CALLBACK (da_expose_event_cb), NULL);
}

void set_stacksize (GladeXML *xml, char *name, void *stack_size)
{
	int		*size;
	GtkToggleButton	*tb;
	
	/* name is NULL */
	size = stack_size;
	if (*size == RPN_FINITE_STACK)
		tb = (GtkToggleButton *) glade_xml_get_widget (xml, "finite_stack_size");
	else tb = (GtkToggleButton *) glade_xml_get_widget (xml, "infinite_stack_size");
	gtk_toggle_button_set_active (tb, TRUE);
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
	
	set_widget_visibility (main_window_xml, "menubar", prefs.show_menu);
	menu_item = glade_xml_get_widget (main_window_xml, "show_menubar1");
	gtk_check_menu_item_set_active ((GtkCheckMenuItem *) menu_item, prefs.show_menu);

	if (prefs.mode == BASIC_MODE) menu_item = 
		glade_xml_get_widget (main_window_xml, "basic_mode");
	else if (prefs.mode == SCIENTIFIC_MODE) menu_item = 
		glade_xml_get_widget (main_window_xml, "scientific_mode");
	gtk_menu_item_activate ((GtkMenuItem *) menu_item);

	set_all_buttons_size (prefs.button_width, prefs.button_height);

	if (prefs.custom_button_font == TRUE) button_font = g_strdup (prefs.button_font);
	else button_font = g_strdup ("");
	set_all_buttons_font (button_font);
	g_free (button_font);
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
	GtkMenuItem		*menu_item;
	
	menu_item = (GtkMenuItem *) glade_xml_get_widget (main_window_xml, \
		g_strstrip (g_ascii_strdown (item_name, -1)));
	if (menu_item)
	/* as we use this only for menu boxes, a simple activate is enough.
	 * the extra magic in src/callbacks.c::on_scientific_mode_activate
	 * is necessary only for checkboxmenuitems.
	 */
		gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	else fprintf (stderr, _("[%s] failed to find widget %s in function \"activate_menu_item\". %s\n"), PROG_NAME, item_name, BUG_REPORT);
}

/* get_number_string - converts value to a string, with repect to base. returned
 *	string should be freed.
 */

char *get_display_number_string (double value, int base)
{
	char 	*string_value;
	
	switch (base) {
		case CS_DEC:
			string_value = g_strdup_printf ("%.*g", get_display_number_length(current_status.number), value);
			break;
		case CS_HEX:
			string_value = ftoax (value, 16, prefs.hex_bits, prefs.hex_signed);
			break;
		case CS_OCT:
			string_value = ftoax (value, 8, prefs.oct_bits, prefs.oct_signed);
			break;
		case CS_BIN:
			string_value = ftoax (value, 2, prefs.bin_bits, prefs.bin_signed);
			if (prefs.bin_fixed == TRUE) 
				string_value = add_leading_zeros (string_value, 
					prefs.bin_length);
			break;
		default:
			string_value = _("unknown number base");
			fprintf (stderr, _("[%s] unknown number base in function \"get_display_number_string\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return string_value;
}

/* get_display_number_length - returns the maximum length of a number in current
 * 	mode.
 */

int get_display_number_length (int base)
{
	switch (base) {
		case CS_DEC:
			return DISPLAY_RESULT_PRECISION;
		case CS_HEX:
			return prefs.hex_bits/4;
		case CS_OCT:
			return prefs.oct_bits/3;
		case CS_BIN:
			return prefs.bin_bits/1;
		default:
			fprintf (stderr, _("[%s] unknown number base in function \"get_display_number_length\". %s\n"), PROG_NAME, BUG_REPORT);
			return 0;
		}
}

/* gfunc_f1. The parenthesis open (PAN) resp swapxy (RPN) button is special. It
 *	has to be operation and gfunc button both in one. There (see also
 *	ui.c::set_basic_object_data it gets both information. in both cases
 *	it is connected to on_gfunc_button_clicked, which calls this function,
 *	which itseld seperates on behalf of current notation, how to continue.
 *	gfunc_f2 is the same. If there will be more buttons of this kind, we
 *	should consider introducing a new button class.
 */

void gfunc_f1 (GtkToggleButton *button)
{
	double		*stack;
	
	if (current_status.notation == CS_PAN) 
		on_operation_button_clicked (button, NULL);
	else {
		display_result_set_double (rpn_stack_swapxy(
			display_result_get_double()));
		stack = rpn_stack_get (RPN_FINITE_STACK);
		display_stack_set_yzt_double (stack);
		free (stack);
		current_status.rpn_stack_lift_enabled = TRUE;
	}
}

/* gfunc_f2. see gfunc1
 */

void gfunc_f2 (GtkToggleButton *button)
{
	double 		*stack;
	
	if (current_status.notation == CS_PAN)
		on_operation_button_clicked (button, NULL);
	else {
		display_result_set_double (rpn_stack_rolldown(
			display_result_get_double()));
				stack = rpn_stack_get (RPN_FINITE_STACK);
		display_stack_set_yzt_double (stack);
		free (stack);
		current_status.rpn_stack_lift_enabled = TRUE;
	}
}

/* push_rpn_result. The terminology used here originates from HP 15 Owner's
 * handbook. If a button doesn't terminate digit entry, it "starts" one.
 * Therefore those buttons have to push result on the stack, is stack lift is
 * enabled (current_status.rpn_stack_lift_enabled).
 * Buttons not terminating digit entry are:
 *	0-9, A-F, ., EE, const, MR
 */

void rpn_stack_lift ()
{
	double	*stack;
	
	if ((current_status.notation == CS_RPN) && 
		(current_status.rpn_stack_lift_enabled == TRUE)) {
		rpn_stack_push (display_result_get_double ());
		stack = rpn_stack_get (RPN_FINITE_STACK);
		display_stack_set_yzt_double (stack);
		free (stack);
		current_status.rpn_stack_lift_enabled = FALSE;
	}
}

void remember_display_values()
{
	char 	*stack[3];
	
	if (prefs.rem_display == TRUE) {
		display_result_set (prefs.rem_valuex);
		/* for the result setting the display string is enough */
		if (current_status.notation == CS_RPN) {
			stack[0] = prefs.rem_valuey;
			stack[1] = prefs.rem_valuez;
			stack[2] = prefs.rem_valuet;
			display_stack_set_yzt (stack);
			/* for the stack we have to update calc_basic */
			rpn_stack_push (string2double(stack[2]));
			rpn_stack_push (string2double(stack[1]));
			rpn_stack_push (string2double(stack[0]));
		}
	}
}

/*
 * string2double - this function makes a string to double conversion with 
 * 	respect to internal calculator settings. it uses axtof.
 */

double string2double (char *string)
{
	char 	*end_ptr;
	double	ret_val;
	
	switch (current_status.number) {
		case CS_DEC:
			ret_val = strtod(string, &end_ptr);
			if (*end_ptr != '\0')
				fprintf (stderr, _("[%s] failed to convert %s to a number properly in function \"string2double\". Have you changed your locales? %s\n"), PACKAGE, string, BUG_REPORT);
			return ret_val;
			break;
		case CS_HEX:
			return axtof(string, 16, prefs.hex_bits, 
				prefs.hex_signed);
			break;
		case CS_OCT:
			return axtof(string, 8, prefs.oct_bits, 
				prefs.oct_signed);
			break;
		case CS_BIN:
			return axtof(string, 2, prefs.bin_bits, 
				prefs.bin_signed);
			break;
		default:
			fprintf (stderr, _("[%s] unknown number base in function \"string2double\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return 0;
}


void set_button_label_and_tooltip (GladeXML *xml, char *button_name, 
	char *label, char *tooltip)
{
	GtkWidget	*w;
	GtkTooltipsData	*tooltip_data;
	
	w = glade_xml_get_widget (xml, button_name);
	if (w) {
		gtk_button_set_label ((GtkButton *)w, label);
		tooltip_data = gtk_tooltips_data_get (w);
		g_free (tooltip_data->tip_text);
		tooltip_data->tip_text = g_strdup (tooltip);
	}
}

gboolean formula_entry_is_active ()
{
	GtkWidget	*active_widget, *main_window;
	
	main_window = glade_xml_get_widget (main_window_xml, "main_window");
	active_widget = gtk_window_get_focus ((GtkWindow *)main_window);
	return !strcmp (gtk_widget_get_name (active_widget), "formula_entry");
}
