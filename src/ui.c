/*
 *  ui.c - general user interface code.
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
#include <locale.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "galculator.h"
#include "ui.h"
#include "display.h"
#include "math_functions.h"
#include "config_file.h"
#include "general_functions.h"
#include "callbacks.h"

GladeXML		*main_window_xml, *button_box_xml, *prefs_xml, *about_dialog_xml;
char			dec_point[2];
GtkListStore	*store;

static void set_disp_ctrl_object_data ();

// assume TRUE for all other bases/modes!

s_active_buttons active_buttons[] = {\
	{"button_2", ~(AB_BIN)}, \
	{"button_3", ~(AB_BIN)}, \
	{"button_4", ~(AB_BIN)}, \
	{"button_5", ~(AB_BIN)}, \
	{"button_6", ~(AB_BIN)}, \
	{"button_7", ~(AB_BIN)}, \
	{"button_8", ~(AB_BIN | AB_OCT)}, \
	{"button_9", ~(AB_BIN | AB_OCT)}, \
	{"button_a", ~(AB_DEC | AB_BIN | AB_OCT)}, \
	{"button_b", ~(AB_DEC | AB_BIN | AB_OCT)}, \
	{"button_c", ~(AB_DEC | AB_BIN | AB_OCT)}, \
	{"button_d", ~(AB_DEC | AB_BIN | AB_OCT)}, \
	{"button_e", ~(AB_DEC | AB_BIN | AB_OCT)}, \
	{"button_f", ~(AB_DEC | AB_BIN | AB_OCT)}, \
	{"button_ee", AB_DEC | AB_RPN | AB_PAN}, \
	{"button_sin", AB_DEC | AB_RPN | AB_PAN}, \
	{"button_cos", AB_DEC | AB_RPN | AB_PAN}, \
	{"button_tan", AB_DEC | AB_RPN | AB_PAN}, \
	{"button_point", ~(AB_BIN | AB_OCT | AB_HEX)}, \
	{"button_sign", ~(AB_BIN | AB_OCT | AB_HEX)}, \
	{"button_paropen", ~(AB_RPN)}, \
	{"button_parclose", ~(AB_RPN)}, \
	{NULL}\
};

static GladeXML *glade_file_open (char *filename, 
				char *root_widget, 
				gboolean fatal)
{
	GladeXML	*xml;
	
	xml = glade_xml_new (filename, root_widget, NULL);
	
	if (xml == NULL) {
		fprintf (stderr, _("[%s] Couldn't load %s. This file is necessary \
to build galculator's user interface. Make sure you did a make install and the file \
is accessible!\n"), PACKAGE, filename);
		if (fatal == TRUE) exit(EXIT_FAILURE);
	}
	return xml;
}

GtkWidget *ui_main_window_create ()
{
	main_window_xml = glade_file_open (MAIN_GLADE_FILE, "main_window", TRUE);
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(main_window_xml);
	set_disp_ctrl_object_data ();
	return glade_xml_get_widget (main_window_xml, "main_window");
}

static void apply_object_data (s_operation_map operation_map[],
			s_gfunc_map gfunc_map[],
			s_function_map function_map[])
{
	int 		counter;
	gpointer	*func;
	
	counter = 0;
	while (operation_map[counter].button_name != NULL) {
		g_object_set_data (G_OBJECT (glade_xml_get_widget (button_box_xml, 
			operation_map[counter].button_name)),
			"operation", GINT_TO_POINTER(operation_map[counter].operation));
		counter++;
	}
	
	counter = 0;
	while (gfunc_map[counter].button_name != NULL) {
		g_object_set_data (G_OBJECT (glade_xml_get_widget (button_box_xml, 
			gfunc_map[counter].button_name)),
			"func", gfunc_map[counter].func);
		counter++;
	};
	
	counter = 0;
	while (function_map[counter].button_name != NULL) {
		func = (void *) malloc (sizeof (function_map[counter].func));
		memcpy (func, function_map[counter].func, sizeof (function_map[counter].func));
		g_object_set_data (G_OBJECT (glade_xml_get_widget (button_box_xml, 
			function_map[counter].button_name)),
			"func", func);
		g_object_set_data (G_OBJECT (glade_xml_get_widget (button_box_xml, 
			function_map[counter].button_name)),
			"is_trigonometric", GINT_TO_POINTER((int)function_map[counter].is_trigonometric));		
		counter++;
	};
}

static void set_scientific_object_data ()
{
	s_operation_map	operation_map[] = {\
		{"button_pow", '^'},\
		{"button_lsh", '<'},\
		{"button_mod", 'm'},\
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
		{"button_percent", '%'},\
		{NULL}\
	};
	
	s_gfunc_map gfunc_map[] = {\
		{"button_sign", display_result_toggle_sign},\
		{"button_ee", display_append_e},\
		{NULL}\
	};
	
	s_function_map function_map[] = {\
		{"button_sin", {sin, asin, sinh, asinh}, TRUE},\
		{"button_cos", {cos, acos, cosh, acosh}, TRUE},\
		{"button_tan", {tan, atan, tanh, atanh}, TRUE},\
		{"button_log", {log10, pow10y, log10, log10}, FALSE},\
		{"button_ln", {log, exp, log, log}, FALSE},\
		{"button_sq", {powx2, sqrt, powx2, powx2}, FALSE},\
		{"button_sqrt", {sqrt, powx2, sqrt, sqrt}, FALSE},\
		{"button_fac", {factorial, factorial, factorial, factorial}, FALSE},\
		{"button_cmp", {cmp, cmp, cmp, cmp}, FALSE},\
		{NULL}\
	};

	apply_object_data (operation_map, gfunc_map, function_map);
}

static void set_basic_object_data ()
{
	s_operation_map	operation_map[] = {\
		{"button_enter", '='},\
		{"button_plus", '+'},\
		{"button_minus", '-'},\
		{"button_mult", '*'},\
		{"button_div", '/'},\
		{"button_paropen", '('},\
		{"button_parclose", ')'},\
		{"button_percent", '%'},\
		{NULL}\
	};
	
	s_gfunc_map gfunc_map[] = {\
		{"button_sign", display_result_toggle_sign},\
		{NULL}\
	};
	
	s_function_map function_map[] = {\
		{"button_sqrt", {sqrt, powx2, sqrt, sqrt}, FALSE},\
		{NULL}\
	};
	
	apply_object_data (operation_map, gfunc_map, function_map);
}

static void set_disp_ctrl_object_data ()
{
	int	counter=0;
	
	s_gfunc_map map[] = {\
		{"button_clr", clear},\
		{"button_backspace", display_result_backspace},\
		{"button_allclr", all_clear},\
		{NULL}\
	};

	while (map[counter].button_name != NULL) {
		g_object_set_data (G_OBJECT (glade_xml_get_widget (
			main_window_xml, map[counter].button_name)),
			"func", map[counter].func);
		counter++;
	};
}

void ui_main_window_buttons_destroy ()
{
	GtkWidget	*box;
	GList		*children;
	
	box = glade_xml_get_widget (main_window_xml, "window_vbox");
	children = gtk_container_get_children ((GtkContainer *)box);
	children = g_list_last (children);
	if (children->data != NULL) gtk_widget_destroy (children->data);
	g_list_free (children);
}

void ui_main_window_buttons_create (int mode)
{
	GtkWidget	*button_box, *box;
	struct lconv 	*locale_settings;
	GList		*closure_list;
	GtkAccelGroup	*accel_group;
	
	if (mode == BASIC_MODE) 
		button_box_xml = glade_file_open (BASIC_GLADE_FILE, "button_box", TRUE);
	else if (prefs.mode == SCIENTIFIC_MODE) 
		button_box_xml = glade_file_open (SCIENTIFIC_GLADE_FILE, "button_box", TRUE);
	else error_message (_("Unknown mode."));
	glade_xml_signal_autoconnect (button_box_xml);
	button_box = glade_xml_get_widget (button_box_xml, "button_box");
	/* we have to add the accel_group of button_box to the main_window
	 * in order to get working accelerators.
	 */
	
	closure_list = gtk_widget_list_accel_closures (
		glade_xml_get_widget (button_box_xml, "button_1"));
	accel_group = gtk_accel_group_from_accel_closure (
		(GClosure *) closure_list->data);
	gtk_window_add_accel_group ((GtkWindow *) glade_xml_get_widget (
		main_window_xml, "main_window"), accel_group);
	/* the buttons we want */
	
	/* find old buttons and remove them */
	box = glade_xml_get_widget (main_window_xml, "window_vbox");
	gtk_box_pack_start ((GtkBox *) box, button_box, TRUE, TRUE, 0);
	gtk_widget_show (button_box);
	
	/* associate buttons with some data */
	if (mode == BASIC_MODE) 
		set_basic_object_data (button_box_xml);
	else if (mode == SCIENTIFIC_MODE) 
		set_scientific_object_data (button_box_xml);
	else error_message (_("Unknown mode."));
	
	/* update "decimal point" button to locale's decimal point */
	dec_point[0] = DEFAULT_DEC_POINT;
	locale_settings = localeconv();
	if (strlen (locale_settings->decimal_point) != 1) {
		fprintf (stderr, _("[%s] length of decimal point (in locale) \
is not supported: >%s<\nYou might face problems when using %s! %s\n)"), 
		PACKAGE, locale_settings->decimal_point, PROG_NAME, BUG_REPORT);
	} else dec_point[0] = locale_settings->decimal_point[0];
	dec_point[1] = '\0';

	gtk_button_set_label ((GtkButton *) glade_xml_get_widget (
		button_box_xml, "button_point"), dec_point);
}

static void set_table_child_size (gpointer data, gpointer user_data)
{
	GtkRequisition	*size;
	GtkTableChild	*table_child;
	
	size = user_data;				/* dereference */
	table_child = data;
	gtk_widget_set_size_request (table_child->widget, size->width, size->height);
}

static void set_table_child_font (gpointer data, gpointer user_data)
{
	PangoFontDescription	*font;
	GtkTableChild		*table_child;
	GtkWidget		*w;
	
	font = user_data;				/* dereference */
	table_child = data;
	w = gtk_bin_get_child ((GtkBin *)(table_child->widget));
	/* if it's a normal button, w is now the label we want to font-change.
	 * if it's a popup button, we have to get the most left child first.
	 */
	if (GTK_IS_BOX (w)) w = ((GtkBoxChild *)((((GtkBox *)w)->children)->data))->widget;
	if (GTK_IS_LABEL(w)) gtk_widget_modify_font (w, font);
	// else do nothing
}

static void set_all_buttons_property (GFunc func, gpointer data)
{
	GtkTable	*table;
	
	/* at first the display control table. always there; somehor */
	table = (GtkTable *) glade_xml_get_widget (main_window_xml, 
		"table_dispctrl");
	g_list_foreach (table->children, func, data);
	/* now depending on mode the remaining buttons */
	if (prefs.mode == BASIC_MODE) {
		table = (GtkTable *) glade_xml_get_widget (button_box_xml, 
			"table_buttons");
		g_list_foreach (table->children, func, data);
	
	} else if (prefs.mode == SCIENTIFIC_MODE) {
		table = (GtkTable *) glade_xml_get_widget (button_box_xml, 
			"table_standard_buttons");
		g_list_foreach (table->children, func, data);
		table = (GtkTable *) glade_xml_get_widget (button_box_xml, 
			"table_bin_buttons");
		g_list_foreach (table->children, func, data);
		table = (GtkTable *) glade_xml_get_widget (button_box_xml, 
			"table_func_buttons");
		g_list_foreach (table->children, func, data);
	}
	else error_message (_("Unknown mode."));
}

void set_all_buttons_size (int width, int height)
{
	GtkRequisition	size;
	
	size.width = width;
	size.height = height;
	set_all_buttons_property (set_table_child_size, (gpointer) &size);
}

void set_all_buttons_font (char *font_string)
{
	PangoFontDescription	*pango_font;

	pango_font = pango_font_description_from_string (font_string);
	set_all_buttons_property (set_table_child_font, pango_font);
}

gboolean button_deactivation (gpointer data)
{
	GtkToggleButton 	*b;
	
	b = (GtkToggleButton*) data;
	//_gtk_button_set_depressed (b, FALSE);
	//gtk_widget_set_state ((GtkWidget *) b, GTK_STATE_NORMAL);
	gtk_toggle_button_set_active (b, FALSE);
	return FALSE;	
}

void button_activation (GtkToggleButton *b)
{
	//gtk_widget_set_state ((GtkWidget *) b, GTK_STATE_ACTIVE);
	//_gtk_button_set_depressed (b, TRUE);
	g_timeout_add (100, button_deactivation, (gpointer) b);
}

void update_active_buttons (int number_base, int notation_mode)
{
	int		counter=0;
	GtkWidget	*current_button;
	unsigned int	state;
	
	state = (1 << number_base) | (1 << (notation_mode + NR_NUMBER_BASES));
	while (active_buttons[counter].button_name != NULL) {
		current_button = glade_xml_get_widget (button_box_xml, 
			active_buttons[counter].button_name);
		if (current_button == NULL) {
			counter++;
			continue;
		}
		gtk_widget_set_sensitive (current_button, 
			(active_buttons[counter].mask & state) == state);
		counter++;
	}
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

GtkWidget *ui_font_dialog_create (char *title, GtkButton *button)
{
	GladeXML	*font_xml;
	GtkWidget	*font_dialog;
	
	font_xml = glade_file_open (FONT_GLADE_FILE, "font_dialog", FALSE);
	glade_xml_signal_autoconnect(font_xml);
	font_dialog = glade_xml_get_widget (font_xml, "font_dialog");
	
	gtk_window_set_title ((GtkWindow *) font_dialog, title);
	gtk_font_selection_dialog_set_font_name ((GtkFontSelectionDialog *) font_dialog, \
		gtk_button_get_label (button));
	gtk_widget_hide (((GtkFontSelectionDialog *)font_dialog)->apply_button);
	gtk_widget_show (font_dialog);

	return font_dialog;
}


GtkWidget *ui_color_dialog_create (char *title, GtkButton *button)
{
	GladeXML	*color_xml;
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
	
	color_xml = glade_file_open (COLOR_GLADE_FILE, "color_dialog", FALSE);
	glade_xml_signal_autoconnect(color_xml);
	color_dialog = glade_xml_get_widget (color_xml, "color_dialog");
	
	gtk_window_set_title ((GtkWindow *) color_dialog, title);
	gtk_color_selection_set_current_color ((GtkColorSelection *)((GtkColorSelectionDialog *)color_dialog)->colorsel, \
		&color);
	gtk_widget_hide (((GtkColorSelectionDialog *)color_dialog)->help_button);
	gtk_widget_show (color_dialog);
	return color_dialog;
}

/* menu code - e.g. used for the constant popup menu */

void position_menu (GtkMenu *menu, 
		gint *x, 
		gint *y, 
		gboolean *push_in, 
		gpointer user_data)
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

GtkWidget *ui_constants_menu_create (s_constant *constant, GCallback const_handler)
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

GtkWidget *ui_memory_menu_create (s_array memory, GCallback const_handler, char *last_item)
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

GtkWidget *ui_pref_dialog_create ()
{
	int			counter=0;
	GtkWidget		*w, *prefs_dialog;
	GtkTreeIter   		iter;
	GtkWidget		*tree_view;
	GtkCellRenderer 	*renderer;
	GtkTreeViewColumn 	*column;
	GtkTreeSelection	*select;
	GtkSizeGroup		*sgroup;
	s_prefs_entry		*prefs_list;
	
	
	prefs_xml = glade_file_open (PREFS_GLADE_FILE, "prefs_dialog", FALSE);
	glade_xml_signal_autoconnect(prefs_xml);
	
	prefs_dialog = glade_xml_get_widget (prefs_xml, "prefs_dialog");
	
	gtk_window_set_title ((GtkWindow *)prefs_dialog, \
		g_strdup_printf (_("%s Preferences"), PACKAGE));
	// preferences -> gui
	prefs_list = config_file_get_prefs_list();
	while (prefs_list[counter].key != NULL) {
		if (prefs_list[counter].set_handler != NULL) {
			prefs_list[counter].set_handler (prefs_xml,
				prefs_list[counter].widget_name,
				prefs_list[counter].variable);
		}
		counter++;
	}
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font_label");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
	w = glade_xml_get_widget (prefs_xml, "prefs_bin_length");
	gtk_widget_set_sensitive (w, prefs.bin_fixed);

	// make user defined constants list.
	
	store = gtk_list_store_new (NR_CONST_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	counter = 0;
	while (constant[counter].desc != NULL) {
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 
			NAME_COLUMN, constant[counter].name, 
			VALUE_COLUMN, constant[counter].value, 
			DESC_COLUMN, constant[counter].desc,
			-1);
		counter++;
	}
	tree_view = glade_xml_get_widget (prefs_xml, "treeview1");
	gtk_tree_view_set_model ((GtkTreeView *) tree_view, GTK_TREE_MODEL (store));
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", NAME_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
	column = gtk_tree_view_column_new_with_attributes ("Value", renderer, "text", VALUE_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
	column = gtk_tree_view_column_new_with_attributes ("Description", renderer, "text", DESC_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
                  G_CALLBACK (const_list_selection_changed_cb),
                  NULL);

	/* pack widget of same size into GtkSizeGroup */

	sgroup = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget (sgroup, 
		glade_xml_get_widget (prefs_xml, "prefs_button_font_label"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_button_width_label"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_button_height_label"));
	
	sgroup = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget (sgroup, 
		glade_xml_get_widget (prefs_xml, "prefs_const_add_button"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_const_update_button"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_const_delete_button"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_const_clear_button"));
	
	sgroup = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget (sgroup, 
		glade_xml_get_widget (prefs_xml, "prefs_hex_bits_label"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_oct_bits_label"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_bin_bits_label"));
	gtk_size_group_add_widget (sgroup,
		glade_xml_get_widget (prefs_xml, "prefs_bin_fixed"));
	
	gtk_widget_show (prefs_dialog);
	return prefs_dialog;
}

GtkWidget *ui_about_dialog_create()
{	
	about_dialog_xml = glade_file_open (ABOUT_GLADE_FILE, 
		"about_dialog", FALSE);
	glade_xml_signal_autoconnect(about_dialog_xml);
	return glade_xml_get_widget (about_dialog_xml, "about_dialog");
}
