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

double			memory_value=0;
extern GladeXML		*main_window_xml;
GladeXML		*font_xml, *color_xml;

// assume TRUE for all other bases/modes!

s_active_buttons active_buttons[] = {\
	{"button_2", {TRUE, TRUE, TRUE, FALSE}, {TRUE, TRUE}}, \
	{"button_3", {TRUE, TRUE, TRUE, FALSE}, {TRUE, TRUE}}, \
	{"button_4", {TRUE, TRUE, TRUE, FALSE}, {TRUE, TRUE}}, \
	{"button_5", {TRUE, TRUE, TRUE, FALSE}, {TRUE, TRUE}}, \
	{"button_6", {TRUE, TRUE, TRUE, FALSE}, {TRUE, TRUE}}, \
	{"button_7", {TRUE, TRUE, TRUE, FALSE}, {TRUE, TRUE}}, \
	{"button_8", {TRUE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_9", {TRUE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_a", {FALSE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_b", {FALSE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_c", {FALSE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_d", {FALSE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_e", {FALSE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_f", {FALSE, TRUE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_const", {TRUE, TRUE, TRUE, TRUE}, {TRUE, TRUE}}, \
	{"button_ee", {TRUE, FALSE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_sin", {TRUE, FALSE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_cos", {TRUE, FALSE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_tan", {TRUE, FALSE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_reci", {TRUE, FALSE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_point", {TRUE, FALSE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_sign", {TRUE, FALSE, FALSE, FALSE}, {TRUE, TRUE}}, \
	{"button_paropen", {TRUE, TRUE, TRUE, TRUE}, {TRUE, FALSE}}, \
	{"button_parclose", {TRUE, TRUE, TRUE, TRUE}, {TRUE, FALSE}}, \
	{NULL}\
};

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
	extern gboolean 		rpn_have_result;
	extern s_current_status 	current_status;

	clear();
	if (current_status.notation == CS_PAN) calc_tree_free();
	else {
		calc_rpn_free();
		rpn_have_result=FALSE;
	}
	display_module_bracket_label_update (RESET);
}

/*
 * helper function for set_button_group_size
 */

void set_table_child_size (gpointer data, gpointer user_data)
{
	s_point		*size;
	GtkTableChild	*table_child;
	
	size = user_data;				/* dereference */
	table_child = data;
	gtk_widget_set_size_request (table_child->widget, size->x, size->y);
}

/*
 * run through all children of the given table and resize them (done by 
 * set_table_child_size)
 */

void set_button_group_size (GladeXML *xml, char *table_name, int width, int height)
{
	GtkTable	*this_table;
	s_point		size;
	
	size.x = width;
	size.y = height;
	this_table = (GtkTable *) glade_xml_get_widget (xml, table_name);
	g_list_foreach (this_table->children, set_table_child_size, &size);
	//gtk_window_resize ((GtkWindow *)gtk_widget_get_toplevel(a_parent_widget), 1, 1);
}

/*
 * the same as for set_button_group_size. helper function for set_button_group_font
 */

void set_table_child_font (gpointer data, gpointer user_data)
{
	PangoFontDescription	*font;
	GtkTableChild		*table_child;
	
	font = user_data;				/* dereference */
	table_child = data;
	/* ATTENTION: we don't want to change the button's font but the font of the button's label! */
	gtk_widget_modify_font (gtk_bin_get_child ((GtkBin *)(table_child->widget)), font);
}

/*
 * new font for the labels of the buttons of the given table
 */ 

void set_button_group_font (GladeXML *xml, char *table_name, char *font_string)
{
	GtkTable		*this_table;
	PangoFontDescription	*pango_font;

	pango_font = pango_font_description_from_string (font_string);
	this_table = (GtkTable *) glade_xml_get_widget (xml, table_name);
	g_list_foreach (this_table->children, set_table_child_font, pango_font);
	pango_font_description_free(pango_font);
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
	
	int_var = value;
	spin_button = (GtkSpinButton *) glade_xml_get_widget (xml, spinbutton_name);
	gtk_spin_button_set_value (spin_button, (double) *int_var);
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

GtkWidget *show_font_dialog (char *title, GtkButton *button)
{
	GtkWidget		*font_dialog;
	
	font_xml = glade_xml_new (FONT_GLADE_FILE, "font_dialog", NULL);
	if (font_xml == NULL) glade_file_not_found (FONT_GLADE_FILE);
	glade_xml_signal_autoconnect(font_xml);
	font_dialog = glade_xml_get_widget (font_xml, "font_dialog");
	
	gtk_window_set_title ((GtkWindow *) font_dialog, title);
	gtk_font_selection_dialog_set_font_name ((GtkFontSelectionDialog *) font_dialog, \
		gtk_button_get_label (button));
	gtk_widget_hide (((GtkFontSelectionDialog *)font_dialog)->apply_button);
	gtk_widget_show (font_dialog);

	return font_dialog;
}

GtkWidget *show_color_dialog (char *title, GtkButton *button)
{
	GtkWidget	*color_dialog, *da;
	GdkColor	color;
	GtkRcStyle	*style;
	GtkBox		*box;
	GtkBin		*vp;
	
	box = (GtkBox *)gtk_bin_get_child ((GtkBin *) button);
	vp = (GtkBin *) (((GtkBoxChild *)g_list_nth_data (box->children, 1))->widget);
	da = gtk_bin_get_child(vp);
	
	style = gtk_widget_get_modifier_style (da);
	color = style->fg[GTK_STATE_NORMAL];
	
	color_xml = glade_xml_new (COLOR_GLADE_FILE, "color_dialog", NULL);
	if (color_xml == NULL) glade_file_not_found (COLOR_GLADE_FILE);
	glade_xml_signal_autoconnect(color_xml);
	color_dialog = glade_xml_get_widget (color_xml, "color_dialog");
	
	gtk_window_set_title ((GtkWindow *) color_dialog, title);
	gtk_color_selection_set_current_color ((GtkColorSelection *)((GtkColorSelectionDialog *)color_dialog)->colorsel, \
		&color);
	gtk_widget_hide (((GtkColorSelectionDialog *)color_dialog)->help_button);
	gtk_widget_show (color_dialog);
	return color_dialog;
}

/*
 * "apply" preference dialog.
 */

void update_all (s_preferences prefs)
{
	GtkCheckMenuItem	*show_menubar_item;
	char			*button_font;
	
	display_update_tags ();
	display_set_bkg_color (prefs.bkg_color);
	
	// don't forget the buttons right from the display
	set_button_group_size (main_window_xml, "table_func", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_bin", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_standard", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_dispctrl", prefs.button_width, prefs.button_height);
	
	if (prefs.custom_button_font == TRUE) button_font = g_strdup (prefs.button_font);
	else button_font = g_strdup ("");
	set_button_group_font (main_window_xml, "table_func", button_font);
	set_button_group_font (main_window_xml, "table_bin", button_font);	
	set_button_group_font (main_window_xml, "table_standard", button_font);	
	set_button_group_font (main_window_xml, "table_dispctrl", button_font);
	
	set_widget_visibility (main_window_xml, "table_bin", prefs.vis_logic);
	set_widget_visibility (main_window_xml, "table_func", prefs.vis_funcs);
	set_widget_visibility (main_window_xml, "table_dispctrl", prefs.vis_dispctrl);
	
	set_widget_visibility (main_window_xml, "menubar", prefs.show_menu);
	show_menubar_item = (GtkCheckMenuItem *) glade_xml_get_widget (main_window_xml, "show_menubar1");
	gtk_check_menu_item_set_active (show_menubar_item, prefs.show_menu);
	
		// we need just a widget of the main window. take show_menubar_item as its there.
	gtk_window_resize ((GtkWindow *)gtk_widget_get_toplevel((GtkWidget *)show_menubar_item), 1, 1);
}

/*
 * (un) hide a widget
 */ 

void set_widget_visibility (GladeXML *xml, char *widget_name, gboolean visible)
{
	GtkWidget	*widget;
	
	widget = glade_xml_get_widget (xml, widget_name);
	if (visible) gtk_widget_show_all (widget);
	else gtk_widget_hide_all (widget);
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

void update_active_buttons (GladeXML *xml, int number_base, int notation_mode)
{
	int		counter=0;
	GtkWidget	*current_button;
	gboolean	state;
	
	while (active_buttons[counter].button_name != NULL) {
		current_button = glade_xml_get_widget (xml, active_buttons[counter].button_name);
		state = active_buttons[counter].number_active[number_base] & \
			active_buttons[counter].notation_active[notation_mode];
		gtk_widget_set_sensitive (current_button, state);
		counter++;
	}
}

gboolean is_valid_number (int number_base, char number)
{
	char *valid_numbers[4]={"1234567890", "1234567890abcdef", "12345670", "01"};
	extern char dec_point;
	
	return ((strchr (valid_numbers[number_base], g_ascii_tolower (number)) != NULL) \
		|| (number == dec_point));
}

gboolean button_deactivation (gpointer data)
{
	GtkButton 	*b;
	
	b = (GtkButton*) data;
	_gtk_button_set_depressed (b, FALSE);
	gtk_widget_set_state ((GtkWidget *) b, GTK_STATE_NORMAL);
	return FALSE;	
}

void button_activation (GtkButton *b)
{
	gtk_widget_set_state ((GtkWidget *) b, GTK_STATE_ACTIVE);
	_gtk_button_set_depressed (b, TRUE);
	g_timeout_add (100, button_deactivation, (gpointer) b);
}

/* set_object_data - it is not possible to pass the user_data argument to a
 * signal handler with glade-2. In order to simplify the callbacks, we
 * set object's data to information used by the callbacks. e.g. the addition
 * button and the addition sign are associated here.
 */

void set_object_data (GladeXML *xml)
{
	int 		counter = 0;
	gpointer	*func;
	
	s_operation_map	operation_map[] = {\
		{"button_pow", '^'},\
		{"button_lsh", '<'},\
		{"button_mod", '%'},\
		{"button_and", '&'},\
		{"button_or", '|'},\
		{"button_xor", 'x'},\
		{"button_enter", '='},\
		{"button_plus", '+'},\
		{"button_minus", '-'},\
		{"button_mult", '*'},\
		{"button_div", '/'},\
		{"button_paropen", '('},\
		{"button_parclose", ')'},\
		{NULL}\
	};
	
	s_gfunc_map gfunc_map[] = {\
		{"button_sign", display_result_toggle_sign},\
		{"button_backspace", display_result_backspace},\
		{"button_ee", display_append_e},\
		{"button_clr", clear},\
		{"button_allclr", all_clear},\
		{NULL}\
	};
	
	s_function_map function_map[] = {\
		{"button_sin", {sin, asin, sinh, sin}, TRUE},\
		{"button_cos", {cos, acos, cosh, cos}, TRUE},\
		{"button_tan", {tan, atan, tanh, tan}, TRUE},\
		{"button_log", {log10, pow10y, log10, log10}, FALSE},\
		{"button_ln", {log, exp, log, log}, FALSE},\
		{"button_reci", {reciprocal, idx, reciprocal, reciprocal}, FALSE},\
		{"button_sq", {powx2, sqrt, powx2, powx2}, FALSE},\
		{"button_sqrt", {sqrt, powx2, sqrt, sqrt}, FALSE},\
		{"button_fac", {factorial, factorial, factorial, factorial}, FALSE},\
		{"button_cmp", {cmp, cmp, cmp, cmp}, FALSE},\
		{NULL}\
	};

	while (operation_map[counter].button_name != NULL) {
		g_object_set_data (G_OBJECT (glade_xml_get_widget (xml, 
			operation_map[counter].button_name)),
			"operation", GINT_TO_POINTER(operation_map[counter].operation));
		counter++;
	}
	counter = 0;
	
	while (gfunc_map[counter].button_name != NULL) {
		g_object_set_data (G_OBJECT (glade_xml_get_widget (xml, 
			gfunc_map[counter].button_name)),
			"func", gfunc_map[counter].func);
		counter++;
	};
	counter = 0;
	
	while (function_map[counter].button_name != NULL) {
		func = (void *) malloc (sizeof (function_map[counter].func));
		memcpy (func, function_map[counter].func, sizeof (function_map[counter].func));
		g_object_set_data (G_OBJECT (glade_xml_get_widget (xml, 
			function_map[counter].button_name)),
			"func", func);
		g_object_set_data (G_OBJECT (glade_xml_get_widget (xml, 
			function_map[counter].button_name)),
			"is_trigonometric", GINT_TO_POINTER((int)function_map[counter].is_trigonometric));		
		counter++;
	};
}

/* menu code - e.g. used for the constant popup menu */

void position_menu (GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
	/* this code is taken from GTK 2.2.1 source, therefore credits go there.
	 *  gtk+-2.0.6/gtk/gtkoptionmenu.c (function gtk_option_menu_position)
	 * modified to fit our button menu widget.
	 */
	GtkWidget *child;
	GtkWidget *widget;
	GtkRequisition requisition;
	GList *children;
	gint screen_width;
	gint menu_xpos;
	gint menu_ypos;
	gint menu_width;
	
	g_return_if_fail (GTK_IS_BUTTON (user_data));
	
	widget = GTK_WIDGET (user_data);
	
	gtk_widget_get_child_requisition (GTK_WIDGET (menu), &requisition);
	menu_width = requisition.width;
	
	/* i guess we don't need the "active" stuff from the original positioning
		code. we don't have any active items
	 */
	 
	gdk_window_get_origin (widget->window, &menu_xpos, &menu_ypos);
	
	menu_xpos += widget->allocation.x;
	menu_ypos += widget->allocation.y + widget->allocation.height / 2 - 2;
	
	children = GTK_MENU_SHELL(menu)->children;
	while (children) {
		child = children->data;
		if (GTK_WIDGET_VISIBLE (child))	{
			gtk_widget_get_child_requisition (child, &requisition);
			menu_ypos -= requisition.height;
		}
		children = children->next;
	}
	
	//screen_width = gdk_screen_get_width (gtk_widget_get_screen (widget));
	screen_width = gdk_screen_width ();
	
	if (menu_xpos < 0) menu_xpos = 0;
	else if ((menu_xpos + menu_width) > screen_width)
		menu_xpos -= ((menu_xpos + menu_width) - screen_width);
	
	*x = menu_xpos;
	*y = menu_ypos;
	*push_in = TRUE;
}

GtkWidget *create_constants_menu (s_constant *constant, GCallback const_handler)
{
	GtkWidget	*menu, *child;
	int		counter=0;
	char		*label;
	
	menu = gtk_menu_new();
	while (constant[counter].desc != NULL) {
		label = g_strdup_printf ("%s: %s (%s)", constant[counter].name, constant[counter].value, constant[counter].desc);
		child = gtk_menu_item_new_with_label(label);
		g_free (label);
		gtk_menu_shell_append ((GtkMenuShell *) menu, child);
		gtk_widget_show (child);
		g_signal_connect (G_OBJECT (child), "activate", const_handler, constant[counter].value);
		counter++;
	}
	return menu;
}

GtkWidget *create_memory_menu (s_array memory, GCallback const_handler, char *last_item)
{
	GtkWidget	*menu, *child;
	int		counter=0;
	char		*label;
	
	menu = gtk_menu_new();
	for (counter = 0; counter < memory.len; counter++) {
		label = g_strdup_printf ("%f", memory.data[counter]);
		child = gtk_menu_item_new_with_label(label);
		g_free (label);
		gtk_menu_shell_append ((GtkMenuShell *) menu, child);
		gtk_widget_show (child);
		g_signal_connect (G_OBJECT (child), "activate", const_handler, GINT_TO_POINTER(counter));
	}
	if (last_item != NULL) {
		label = g_strdup (last_item);
		child = gtk_menu_item_new_with_label(label);
		g_free (label);
		gtk_menu_shell_append ((GtkMenuShell *) menu, child);
		gtk_widget_show (child);
		g_signal_connect (G_OBJECT (child), "activate", const_handler, GINT_TO_POINTER(counter));
	}
	return menu;
}

void glade_file_not_found (char *filename)
{
	fprintf (stderr, _("[%s] Couldn't load %s. This file is necessary \
to build galculator's user interface. Make sure you did a make install and the file \
is accessible!\n"), PACKAGE, filename);
}
