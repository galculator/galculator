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
#include "flex_parser.h"

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
			alg_free(main_alg);
			main_alg = alg_init(0);
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
			alg_free(main_alg);
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
	char		*lower_bin_string;
	
	lower_bin_string = g_ascii_strdown(bin_string, -1);
	/* according to man strtod, inf should be there in every case */
	if (strstr (lower_bin_string, "inf") != NULL) return INFINITY;

	for (counter = strlen (lower_bin_string) - 1; counter >= 0; counter--) {
		if (lower_bin_string[counter] - '0' < 10) \
			return_value += (lower_bin_string[counter] - '0') * pow (base, strlen (lower_bin_string) - 1 - counter);
		else if (lower_bin_string[counter] - 'a' < 10) \
			return_value += (lower_bin_string[counter] - 'a' + 10) * pow (base, strlen (lower_bin_string) - 1 - counter);
		else fprintf (stderr, _("[%s] failed to convert char %c in function \"axtof\". %s\n"), PROG_NAME, lower_bin_string[counter], BUG_REPORT);
	}
	
	g_free (lower_bin_string);
	
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
	if (strcmp (string, "0") == 0) return g_strdup(string);
	if (strcmp (string, MY_INFINITY_STRING) == 0) return g_strdup(string);
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
	if (toggle_button) {
		gtk_toggle_button_set_active (toggle_button, *bool_var);
		/* they say there is no good reason. i say there is */
		gtk_toggle_button_toggled (toggle_button);
	}
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

void set_entry (GladeXML *xml, char *entry_name, void *entry_text)
{
	GtkEntry	*entry;
	char 		**string_var;
	
	string_var = entry_text;	
	entry = (GtkEntry *) glade_xml_get_widget (xml, entry_name);
	if (entry) gtk_entry_set_text (entry, *string_var);	
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
			string_value = g_strdup(_("unknown number base"));
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
		current_status.calc_entry_start_new = TRUE;
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
		current_status.calc_entry_start_new = TRUE;
	}
}

/* rpn_stack_lift. The terminology used here originates from HP 15 Owner's
 * handbook. If a button doesn't terminate digit entry, it "starts" one.
 * Therefore those buttons have to push result on the stack, if stack lift is
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
			rpn_stack_push (string2double(stack[2], current_status.number));
			rpn_stack_push (string2double(stack[1], current_status.number));
			rpn_stack_push (string2double(stack[0], current_status.number));
		}
	}
}

/*
 * string2double - this function makes a string to double conversion with 
 * 	respect to supplied number base. if number base < 0 we try to 
 *	determine from string which should have then a 0? prefix.
 *	it uses axtof.
 */

double string2double (char *string, int number_base)
{
	char 	*end_ptr;
	double	ret_val;
	
	switch (number_base) {
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

/* string_separator. insert separator.
 * 	string - the string to modify
 *	separate - whether to do sth at all
 *	block_length - insert separator all block_length chars
 *	separator - separator char to insert
 * 	dpoint - char representing the decimal point
 * we do not free string here as it might be used later on!
 */

char *string_add_separator (char* string, gboolean separate, int block_length, char separator, char dpoint)
{
	int	int_length=0, frac_length=0, counter=0, new_counter=0, offset;
	char 	*new_string;
	
	
	if (!separate) return g_strdup(string);
	/* at first, get length of parts pre- and succeeding the decimal point */
	while ((string[int_length] != '\0') && (string[int_length] != dpoint))
		int_length++;
	if (string[int_length] != '\0')
		while (string[int_length + frac_length + 1] != '\0') frac_length++;
	/* then allocate memory for new string holding separators */
	new_string = (char *) malloc ((strlen(string) + (int_length-1)/block_length +
		(frac_length-1)/block_length + 1) * sizeof(char));
	/* then copy from string to new_string and insert separators */
	while ((string[counter] != '\0') && (string[counter] != dpoint)) {
		/* > ( == ) is horrible, yes, but somehow cool. avoids space
		 * between sign char '-' and leading digit
		 */
		if ((counter > (*string == '-')) && ((int_length % block_length) == (counter % block_length))) {
			new_string[new_counter] = separator;
			new_counter++;
		}
		new_string[new_counter] = string[counter];
		new_counter++;
		counter++;
	}
	if (string[int_length] != '\0') {
		/* copy decimal point */
		new_string[new_counter] = string[counter];
		new_counter++;
		counter++;
		offset = counter;
		while (string[counter] != '\0') {
			if (((counter - offset) > 0) && ((counter - offset) % block_length == 0)) {
				new_string[new_counter] = separator;
				new_counter++;
			}
			new_string[new_counter] = string[counter];
			new_counter++;
			counter++;
		}
	}
	new_string[new_counter] = '\0';
	return new_string;
}

/* string_del_separator. removes separator from string, in place.
 */

char *string_del_separator (char *string, char separator)
{
	int	counter=0, new_counter=0;
	
	while (string[counter] != '\0') {
		if (string[counter] == separator) {
			counter++;
			continue;
		}
		string[new_counter] = string[counter];
		counter++;
		new_counter++;
	}
	string[new_counter] = '\0';
	return string;
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

GtkWidget *formula_entry_is_active (GtkWidget *window_widget)
{
	GtkWidget	*active_widget=NULL, *main_window=NULL;
	
	main_window = glade_xml_get_widget (main_window_xml, "main_window");
	if (main_window != NULL)
		active_widget = gtk_window_get_focus ((GtkWindow *)main_window);
	if (active_widget != NULL)
		if ((strcmp (gtk_widget_get_name (gtk_widget_get_toplevel(window_widget)),"main_window") == 0) &&
			(strcmp (gtk_widget_get_name (active_widget), "formula_entry") == 0)) 
			return active_widget;
	return NULL;
}

/* sometimes it makes no sense to check for the toplevel widget's name, e.g.
 * for menuitems. then use this func.
 */

GtkWidget *formula_entry_is_active_no_toplevel_check ()
{
	GtkWidget	*active_widget=NULL, *main_window=NULL;
	
	main_window = glade_xml_get_widget (main_window_xml, "main_window");
	if (main_window) 
		active_widget = gtk_window_get_focus ((GtkWindow *) main_window);
	if (active_widget)
		if (strcmp (gtk_widget_get_name (active_widget), "formula_entry") == 0) 
			return active_widget;
	return NULL;
}

s_flex_parser_result compute_user_function (char *expression, char *variable, char *value)
{
	int			nr_constants;
	s_flex_parser_result	result;
	
	nr_constants = 0;
	while (constant[nr_constants].name != NULL) nr_constants++;
	constant = (s_constant *) realloc (constant, (nr_constants + 2) * sizeof(s_constant));
	constant[nr_constants + 1].name = NULL;
	
	constant[nr_constants].name = variable;
	constant[nr_constants].value = value;
	constant[nr_constants].desc = NULL;
	
	result = flex_parser(expression);
	
	constant = (s_constant *) realloc (constant, (nr_constants + 1) * sizeof(s_constant));
	constant[nr_constants].name = NULL;
	return result;
}

double x2rad (double x)
{
	switch (current_status.angle){
	case CS_DEG:
		return deg2rad(x); 
	case CS_RAD:
		return x;
	case CS_GRAD:
		return grad2rad (x);
	default:
		fprintf (stderr, _("[%s] unknown angle base in function \"x2rad\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return x;
}	

double rad2x (double rad)
{
	switch (current_status.angle) {
	case CS_DEG:
		return rad2deg(rad);
	case CS_RAD:
		return rad;
	case CS_GRAD:
		return rad2grad(rad);
	default:
		fprintf (stderr, _("[%s] unknown angle base in function \"rad2x\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return rad;
}

gboolean get_sep (int number_base)
{
	switch (number_base) {
	case CS_DEC:
		return prefs.dec_sep;
	case CS_HEX:
		return prefs.hex_sep;
	case CS_OCT:
		return prefs.oct_sep;
	case CS_BIN:
		return prefs.bin_sep;
	default:
		fprintf (stderr, _("[%s] unknown number base in function \"get_sep\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return 0;
}

int get_sep_length (int number_base)
{
	switch (number_base) {
	case CS_DEC:
		return prefs.dec_sep_length;
	case CS_HEX:
		return prefs.hex_sep_length;
	case CS_OCT:
		return prefs.oct_sep_length;
	case CS_BIN:
		return prefs.bin_sep_length;
	default:
		fprintf (stderr, _("[%s] unknown number base in function \"get_sep_length\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return 0;
}

char get_sep_char (int number_base)
{
	switch (number_base) {
	case CS_DEC:
		return prefs.dec_sep_char[0];
	case CS_HEX:
		return prefs.hex_sep_char[0];
	case CS_OCT:
		return prefs.oct_sep_char[0];
	case CS_BIN:
		return prefs.bin_sep_char[0];
	default:
		fprintf (stderr, _("[%s] unknown number base in function \"get_sep_char\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return 0;
}

void prefs_sep_char_changed (GtkEditable *editable, char *prefs_sep, int number_base)
{
	char 	*sep, *result, **stack;
	
	sep = gtk_editable_get_chars (editable, 0, -1);
	if (strlen(sep) > 0) {
		if ((is_valid_number(number_base, *sep)) ||
			((number_base == CS_DEC) && ((*sep == '-') ||
			(*sep == 'e') || (*sep == dec_point[0]))))
			gtk_editable_delete_text (editable, 0, -1);
		else {
			result = display_result_get();
			stack = display_stack_get_yzt();
			if (prefs_sep) g_free (prefs_sep);
			prefs_sep = g_strdup(sep);
			if (number_base == current_status.number) {
				display_result_set(result);
				display_stack_set_yzt(stack);
			}
			g_free (result);
			g_free (stack);
		}
	}
	g_free (sep);	
}
