/*
 *  callbacks.c - functions to handle GUI events.
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
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>

#include "calc_basic.h"
#include "galculator.h"
#include "math_functions.h"
#include "general_functions.h"
#include "display.h"
#include "config_file.h"
#include "callbacks.h"

#define SELECT_RESULT_FONT _("Select result font")
#define SELECT_MODULE_FONT _("Select module font")
#define SELECT_ACT_MOD_COLOR _("Select active module color")
#define SELECT_INACT_MOD_COLOR _("Select inactive module color")
#define SELECT_RESULT_FONT_COLOR _("Select result font color")
#define SELECT_BKG_COLOR _("Select background color")
#define SELECT_BUTTON_FONT _("Select button font")

GladeXML			*prefs_xml;
static GtkWidget		*font_dialog, *color_dialog;
extern GtkWidget		*main_window;
gboolean			rpn_have_result=FALSE, allow_arith_op=TRUE;
static GtkToggleButton 		*inv_button=NULL, *hyp_button=NULL;
static GtkListStore		*store;
GtkTreeIter 			current_list_iter;
extern s_preferences		prefs;
extern s_prefs_entry		prefs_list[];
extern s_current_status 	current_status;

/* File */

void
on_quit_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	// remember display's value
	if (prefs.rem_value) g_free (prefs.rem_value);
	prefs.rem_value = display_result_get();
	gtk_main_quit();
}

/* Help */

void
on_about_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget	*about_dialog;
	GtkLabel	*about_label;
	char		*about_text;
	GladeXML	*about_dialog_xml;
	
	about_dialog_xml = glade_xml_new (ABOUT_GLADE_FILE, "about_dialog", NULL);
	glade_xml_signal_autoconnect(about_dialog_xml);
	about_dialog = glade_xml_get_widget (about_dialog_xml, "about_dialog");
	
	gtk_window_set_title ((GtkWindow *)about_dialog, g_strdup_printf (_("About %s"), PROG_NAME));
	about_label = (GtkLabel *) glade_xml_get_widget (about_dialog_xml, "about_label");
	gtk_label_set_justify (about_label, GTK_JUSTIFY_CENTER);
	about_text = g_strdup_printf (_("<span size=\"x-large\" weight=\"bold\">%s v%s</span>\n\
<span size=\"large\">a GTK 2 based scientific calculator</span>\n\n\
(c) 2002-2003 by Simon Floery (simon.floery@gmx.at)"), PROG_NAME, VERSION);
	gtk_label_set_markup (about_label, \
		about_text);
	gtk_widget_show (about_dialog);
}

void
on_about_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy (gtk_widget_get_toplevel ((GtkWidget *)button));
}

/* this callback is called if a button for entering a number is clicked. There are two
 * cases: either starting a new number or appending a digit to the existing number.
 * The decimal point leads to some specialities.
 */

void
on_number_button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{	
	button_activation (button);
	display_result_add_digit (*(gtk_button_get_label (button)));
}

/* this callback is called if a button for doing one of the arithmetic operations plus, minus, 
 * multiply, divide or power is clicked. it is mainly an interface to the calc_basic code.
 */

void
on_operation_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	s_calc_token		current_token;
	extern gboolean		calc_entry_start_new;
	double			return_value;
	
	button_activation (button);
	/* current number, get it from the display! */
	current_token.number = display_result_get_double ();
	current_token.operator = (int) g_object_get_data (G_OBJECT (button), "operation");
	// do inverse left shift is a right shift
	if ((current_token.operator == '<') && \
		(BIT (current_status.fmod, CS_FMOD_FLAG_INV) == 1)) {
			current_token.operator = '>';
			if (inv_button != NULL) gtk_toggle_button_set_active (inv_button, FALSE);
	}
	
	/* notation specific interface code */
	
	if (current_status.notation == CS_PAN) {
		/* '(' doesn't pay respect to allow_arith_op but sets it: a+((((((b-...
		 * ')' pays respect to allow_arith_op but doesn't set it: ...+a)))))-...
		 * '=' pays respect to allow_arith_op but doesn't set it: ...+a=
		 * 	(in order to continue with the result on the display)
		 * all other operator pay respect and set allow_arith_op.
		 *
		 * in general, a closing bracket is only useful if there were opening
		 *	brackets.
		 */
		
		if (((current_token.operator == '(') || allow_arith_op) && \
			((current_token.operator != ')') || (display_module_bracket_label_update (GET) > 0))) {
			return_value = calc_tree_add_token (current_token);
			display_result_set_double (return_value);
			display_module_arith_label_update (current_token.operator);
			
			/* setting of allow_arith_op. the missing breaks are wanted */
			switch (current_token.operator) {
				case '=':
					display_module_bracket_label_update (RESET);
					break;
				case ')':
					display_module_bracket_label_update (ONE_LESS);
					break;
				case '(':
					display_module_bracket_label_update (ONE_MORE);
				default:
					allow_arith_op=FALSE;
			}
		}
	} else if (current_status.notation == CS_RPN) {
		switch (current_token.operator) {
		case '=':
			calc_rpn_stack_add (current_token.number);
			rpn_have_result = FALSE;
			break;
		default:
			display_result_set_double (calc_rpn_stack_operation (current_token));
			rpn_have_result = TRUE;
		}
	} else fprintf (stderr, _("[%s] on_operation_button_clicked: unknown status. %s\n"), PROG_NAME, BUG_REPORT);

	calc_entry_start_new = TRUE;
}

/* this callback is called if a button for a function manipulating the current entry directly
 * is clicked. the array function_list knows the relation between button label and function to
 * call.
 */

void
on_function_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	extern gboolean		calc_entry_start_new;
	double			(*func[4])(double);
	gboolean		is_trigonometric;
	
	button_activation (button);
	memcpy (func, g_object_get_data (G_OBJECT (button), "func"), sizeof (func));
	is_trigonometric = (gboolean) g_object_get_data (G_OBJECT (button), "is_trigonometric");
	/* hyperbolic versions of trigonometric functions doesn't have to pay attention to
		angle base. therefore do it like a normal function
	 */
	if (is_trigonometric && (BIT (current_status.fmod, CS_FMOD_FLAG_HYP) == 0)) {
		if (BIT (current_status.fmod, CS_FMOD_FLAG_INV) == 0) {
			display_result_set_double (\
				func[current_status.fmod](display_result_get_rad_angle()));
		} else {
			display_result_set_radiant (\
				func[current_status.fmod](display_result_get_double()));
		}	
	} else {
		display_result_set_double (\
			func[current_status.fmod](display_result_get_double()));
	}
	if (inv_button != NULL)	{
		gtk_toggle_button_set_active (inv_button, FALSE);
		gtk_toggle_button_set_active (hyp_button, FALSE);
	}
	calc_entry_start_new = TRUE;	
	if (current_status.notation == CS_RPN) rpn_have_result = TRUE;
}

void constants_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	char		*const_value;
	
	const_value = user_data;
	display_result_set (const_value);
}


void
on_constant_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget		*menu;
	extern s_constant	*constant;
	
	button_activation (button);
	menu = create_constants_menu(constant, (GCallback)constants_menu_handler);
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu, 
		button, 0, 0);
}

/* tbutton_fmod - these are function modifiers such as INV (inverse) and HYP (hyperbolic).
 * 	as standard math.h doesn't support the inverse of a hyperbolic function, we allow only
 *	one button to be active.
 */

void
on_tbutton_fmod_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	extern GladeXML		*main_window_xml;
	
	if (inv_button == NULL) {
		inv_button = (GtkToggleButton *)glade_xml_get_widget (main_window_xml, "tbutton_inv");
		hyp_button = (GtkToggleButton *)glade_xml_get_widget (main_window_xml, "tbutton_hyp");
	}
	if (strcmp (gtk_button_get_label (button), "inv") == 0) {
		if (BIT (current_status.fmod, CS_FMOD_FLAG_HYP) == 1) gtk_toggle_button_set_active (hyp_button, FALSE);
		current_status.fmod ^= 1 << CS_FMOD_FLAG_INV;
	}
	else if (strcmp (gtk_button_get_label (button), "hyp") == 0) {
		if (BIT (current_status.fmod, CS_FMOD_FLAG_INV) == 1) gtk_toggle_button_set_active (inv_button, FALSE);
		current_status.fmod ^= 1 << CS_FMOD_FLAG_HYP;
	}
	else error_message (_("unknown function modifier (INV/HYP)"));
}

void
on_gfunc_button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	void	(*func)();
	
	button_activation (button);
	func = g_object_get_data (G_OBJECT (button), "func");
	func();
}

void
on_dec_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_DEC, DISPLAY_OPT_NUMBER);
}


void
on_hex_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_HEX, DISPLAY_OPT_NUMBER);
}


void
on_oct_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_OCT, DISPLAY_OPT_NUMBER);
}


void
on_bin_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_BIN, DISPLAY_OPT_NUMBER);
}


void
on_deg_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_DEG, DISPLAY_OPT_ANGLE);
}


void
on_rad_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_RAD, DISPLAY_OPT_ANGLE);
}


void
on_grad_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_GRAD, DISPLAY_OPT_ANGLE);
}


void
on_ordinary_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkButton			*button;
	extern GladeXML		*main_window_xml;
	
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_PAN, DISPLAY_OPT_NOTATION);
	calc_rpn_free ();
	all_clear ();
	button = (GtkButton *) glade_xml_get_widget (main_window_xml, "button_enter");
	gtk_button_set_label (button, "=");
}


void
on_rpn_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkButton		*button;
	extern GladeXML		*main_window_xml;
	
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	display_change_option (CS_RPN, DISPLAY_OPT_NOTATION);
	calc_tree_free ();
	all_clear ();
	button = (GtkButton *) glade_xml_get_widget (main_window_xml, "button_enter");
	gtk_button_set_label (button, _("ENT"));
}

static void const_list_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
        GtkTreeModel 	*model;
	char 		*string;
	GtkWidget	*entry;
	
        if (gtk_tree_selection_get_selected (selection, &model, &current_list_iter))
        {
                gtk_tree_model_get (model, &current_list_iter, NAME_COLUMN, &string, -1);
		entry = glade_xml_get_widget (prefs_xml, "prefs_cname_entry");
		gtk_entry_set_text ((GtkEntry *) entry, string);
                g_free (string);
		gtk_tree_model_get (model, &current_list_iter, VALUE_COLUMN, &string, -1);
		entry = glade_xml_get_widget (prefs_xml, "prefs_cvalue_entry");
		gtk_entry_set_text ((GtkEntry *) entry, string);
                g_free (string);
		gtk_tree_model_get (model, &current_list_iter, DESC_COLUMN, &string, -1);
		entry = glade_xml_get_widget (prefs_xml, "prefs_cdesc_entry");
		gtk_entry_set_text ((GtkEntry *) entry, string);
                g_free (string);
        }
 
}

void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	int			counter=0;
	GtkWidget		*w, *prefs_dialog;
	GtkTreeIter   		iter;
	GtkWidget		*tree_view;
	GtkCellRenderer 	*renderer;
	GtkTreeViewColumn 	*column;
	GtkTreeSelection	*select;
	extern s_constant	*constant;
	
	prefs_xml = glade_xml_new(PREFS_GLADE_FILE, "prefs_dialog", NULL);
	glade_xml_signal_autoconnect(prefs_xml);
	prefs_dialog = glade_xml_get_widget (prefs_xml, "prefs_dialog");;
	
	gtk_window_set_title ((GtkWindow *)prefs_dialog, \
		g_strdup_printf (_("%s Preferences"), PACKAGE));

	// preferences -> gui
	while (prefs_list[counter].key != NULL) {
		if (prefs_list[counter].set_handler != NULL) \
			prefs_list[counter].set_handler (prefs_xml, \
				prefs_list[counter].widget_name, \
				prefs_list[counter].variable);
		counter++;
	}
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font_label");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
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
	gtk_widget_show (prefs_dialog);
}

void
on_prefs_result_font_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	font_dialog = show_font_dialog (SELECT_RESULT_FONT, button);
}


void
on_prefs_result_color_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = show_color_dialog (SELECT_RESULT_FONT_COLOR, button);
}


void
on_prefs_mod_font_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	font_dialog = show_font_dialog (SELECT_MODULE_FONT, button);
}


void
on_prefs_act_mod_color_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = show_color_dialog (SELECT_ACT_MOD_COLOR, button);
}


void
on_prefs_inact_mod_color_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = show_color_dialog (SELECT_INACT_MOD_COLOR, button);
}


void
on_prefs_bkg_color_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = show_color_dialog (SELECT_BKG_COLOR, button);
}


void
on_prefs_button_font_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	font_dialog = show_font_dialog (SELECT_BUTTON_FONT, button);
}

void
on_color_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	const char 	*title;
	GtkWidget	*da=NULL;
	GdkColor	color;

	title = gtk_window_get_title ((GtkWindow *) color_dialog);
	gtk_color_selection_get_current_color ((GtkColorSelection *)(((GtkColorSelectionDialog *) color_dialog)->colorsel), \
		&color);
	gtk_widget_destroy (color_dialog);
	/* possibilities: background color, result color, act/inactive option color */
	if (strstr (title, SELECT_BKG_COLOR) != NULL)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_bkg_color");
		if (prefs.bkg_color != NULL) g_free (prefs.bkg_color);
		prefs.bkg_color = gdk_color_to_string(color);
		display_set_bkg_color (prefs.bkg_color);
	}
	else if (strstr (title, SELECT_RESULT_FONT_COLOR) != NULL)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_result_color");
		if (prefs.result_color != NULL) g_free (prefs.result_color);
		prefs.result_color = gdk_color_to_string(color);
		display_update_tags();
	}
	else if (strstr (title, SELECT_ACT_MOD_COLOR) != NULL)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_act_mod_color");
		if (prefs.act_mod_color != NULL) g_free (prefs.act_mod_color);
		prefs.act_mod_color = gdk_color_to_string(color);
		display_update_tags();
	}
	else if (strstr (title, SELECT_INACT_MOD_COLOR) != NULL)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_inact_mod_color");
		if (prefs.inact_mod_color != NULL) g_free (prefs.inact_mod_color);
		prefs.inact_mod_color = gdk_color_to_string(color);
		display_update_tags();
	}
	else fprintf (stderr, _("[%s] Color Dialog (%s) not found. %s\n"), PACKAGE, title, BUG_REPORT);

	gtk_widget_really_modify_fg (da, color);
}


void
on_font_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkButton		*font_button=NULL;
	const char 		*title;
	char			*font_name, *button_font;
	extern GladeXML		*main_window_xml;

	title = gtk_window_get_title ((GtkWindow *) font_dialog);
	font_name = gtk_font_selection_dialog_get_font_name ((GtkFontSelectionDialog *)font_dialog);
	gtk_widget_destroy (font_dialog);
	/*possibilities: result font, option font, button font */
	if (strstr (title, SELECT_RESULT_FONT) != NULL)
	{
		font_button = (GtkButton *) glade_xml_get_widget (prefs_xml, "prefs_result_font");
		if (prefs.result_font != NULL) g_free (prefs.result_font);
		prefs.result_font = g_strdup(font_name);
		display_update_tags();
	}
	else if (strstr (title, SELECT_MODULE_FONT) != NULL)
	{
		font_button = (GtkButton *) glade_xml_get_widget (prefs_xml, "prefs_mod_font");
		if (prefs.mod_font != NULL) g_free (prefs.mod_font);
		prefs.mod_font = g_strdup(font_name);
		display_update_tags();
	}
	else if (strstr (title, SELECT_BUTTON_FONT) != NULL)
	{
		font_button = (GtkButton *) glade_xml_get_widget (prefs_xml, "prefs_button_font");
		if (prefs.button_font != NULL) g_free (prefs.button_font);
		prefs.button_font = g_strdup(font_name);
		if (prefs.custom_button_font == TRUE) button_font = g_strdup (prefs.button_font);
		else button_font = g_strdup ("");
		set_button_group_font (main_window_xml, "table_func", button_font);
		set_button_group_font (main_window_xml, "table_bin", button_font);	
		set_button_group_font (main_window_xml, "table_standard", button_font);	
		set_button_group_font (main_window_xml, "table_dispctrl", button_font);
		g_free (button_font);
	}
	else fprintf (stderr, _("[%s] Font Dialog (%s) not found. %s\n"), PACKAGE, title, BUG_REPORT);
	
	gtk_button_set_label (font_button, font_name);
}



void
on_prefs_close_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy (gtk_widget_get_toplevel((GtkWidget *)button));
}


void
on_color_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy (color_dialog);
}


void
on_font_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy (font_dialog);
}


void
on_show_menubar1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	extern GladeXML		*main_window_xml;
	
	prefs.show_menu = gtk_check_menu_item_get_active ((GtkCheckMenuItem *) menuitem);;
	set_widget_visibility (main_window_xml, "menubar", prefs.show_menu);
}

void
on_prefs_custom_button_font_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	GtkWidget	*w;
	char		*button_font;
	extern GladeXML	*main_window_xml;
	
	prefs.custom_button_font = gtk_toggle_button_get_active (togglebutton);
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font_label");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
	if (prefs.custom_button_font == TRUE) button_font = g_strdup (prefs.button_font);
	else button_font = g_strdup ("");
	set_button_group_font (main_window_xml, "table_func", button_font);
	set_button_group_font (main_window_xml, "table_bin", button_font);	
	set_button_group_font (main_window_xml, "table_standard", button_font);	
	set_button_group_font (main_window_xml, "table_dispctrl", button_font);
	g_free (button_font);
}


/* this code is taken from the GTK 2.0 tutorial: 
 *		http://www.gtk.org/tutorial
 */

gboolean on_textview_button_press_event (GtkWidget *widget,
						GdkEventButton *event,
						gpointer user_data)
{
	static 			GdkAtom targets_atom = GDK_NONE;
	int			x, y;
	GtkTextIter		start, end;
	char 			*selected_text;
	extern GtkTextView	*view;
	extern GtkTextBuffer	*buffer;
	
	if (event->button == 1)	{
		gtk_widget_get_pointer (widget, &x, &y);
		gtk_text_view_get_iter_at_location (view, &start, x, y);
		// we return if we are in the first line
		if (gtk_text_iter_get_line (&start) != DISPLAY_MODULES_LINE) return FALSE;
		// we return if its the end iterator
		if (gtk_text_iter_is_end (&start) == TRUE) return FALSE;
		end = start;
		if (!gtk_text_iter_starts_word(&start)) gtk_text_iter_backward_word_start (&start);
		if (!gtk_text_iter_ends_word(&end)) gtk_text_iter_forward_word_end (&end);
		selected_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
		// in a rare case, we get two options as selected_text
		if (strchr (selected_text, ' ') != NULL) return FALSE;
		/* rather a hack: last_arith is ignored as one char only gets selected as
			a word with spaces (because of iter_[back|for]ward_..). So we have to
			ignore the open brackets. */
		if (strlen (selected_text) <= 2) return FALSE;
	
		activate_menu_item (selected_text);
	}	
	else if (event->button == 2) {
		/* it's pasting selection time ...*/
		/* Get the atom corresponding to the string "STRING" */
		if (targets_atom == GDK_NONE) \
			targets_atom = gdk_atom_intern ("STRING", FALSE);
	
		/* And request the "STRING" target for the primary selection */
		gtk_selection_convert (widget, GDK_SELECTION_PRIMARY, targets_atom, \
			GDK_CURRENT_TIME);
	}		
	return FALSE;
}

/* this code is taken from the GTK 2.0 tutorial: 
 *		http://www.gtk.org/tutorial
 */

void on_textview_selection_received (GtkWidget *widget,
					GtkSelectionData *data,
					guint time,
					gpointer user_data)
{
	int		counter;
	
	/* **** IMPORTANT **** Check to see if retrieval succeeded  */
	/* occurs if we just press the middle button with no active selection */
	if (data->length < 0) return;
	
	/* Make sure we got the data in the expected form */
	if (data->type != GDK_SELECTION_TYPE_STRING) return;
	
	for (counter = 0; counter < data->length; counter++) {
		if (is_valid_number(current_status.number, data->data[counter])) \
			display_result_add_digit (data->data[counter]);
	}
	if (data->data[0] == '-') display_result_toggle_sign ();

	return;
}

void on_prefs_vis_number_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	prefs.vis_number = gtk_toggle_button_get_active (togglebutton);
	display_update_modules ();
}
										
void on_prefs_vis_angle_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	prefs.vis_angle = gtk_toggle_button_get_active (togglebutton);
	display_update_modules ();
}

void on_prefs_vis_notation_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	prefs.vis_notation = gtk_toggle_button_get_active (togglebutton);
	display_update_modules ();
}

void on_prefs_vis_arith_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	prefs.vis_arith = gtk_toggle_button_get_active (togglebutton);
	display_update_modules ();
}
	
void on_prefs_vis_bracket_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	prefs.vis_bracket = gtk_toggle_button_get_active (togglebutton);
	display_update_modules ();
}

void on_prefs_vis_funcs_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	extern GladeXML	*main_window_xml;
	
	prefs.vis_funcs = gtk_toggle_button_get_active (togglebutton);
	set_widget_visibility (main_window_xml, "table_func", prefs.vis_funcs);
}

void on_prefs_vis_logic_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	extern GladeXML	*main_window_xml;
	
	prefs.vis_logic = gtk_toggle_button_get_active (togglebutton);
	set_widget_visibility (main_window_xml, "table_bin", prefs.vis_logic);
}

void on_prefs_vis_dispctrl_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	extern GladeXML	*main_window_xml;
	
	prefs.vis_dispctrl = gtk_toggle_button_get_active (togglebutton);
	set_widget_visibility (main_window_xml, "table_dispctrl", prefs.vis_dispctrl);

}

void on_prefs_show_menu_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	extern GladeXML			*main_window_xml;
	GtkCheckMenuItem		*show_menubar_item;
	
	prefs.show_menu = gtk_toggle_button_get_active (togglebutton);
	set_widget_visibility (main_window_xml, "menubar", prefs.show_menu);
	show_menubar_item = (GtkCheckMenuItem *) glade_xml_get_widget (main_window_xml, "show_menubar1");
	gtk_check_menu_item_set_active (show_menubar_item, prefs.show_menu);
}

void on_prefs_rem_display_toggled (GtkToggleButton *togglebutton, 
										gpointer user_data)
{
	prefs.rem_display = gtk_toggle_button_get_active (togglebutton);
	// only is important when leaving galculator
}

void on_prefs_def_number_changed (GtkOptionMenu *optionmenu,
										gpointer user_data)
{
	prefs.def_number = gtk_option_menu_get_history (optionmenu);
}
										
void on_prefs_def_angle_changed (GtkOptionMenu *optionmenu,
										gpointer user_data)
{
	prefs.def_number = gtk_option_menu_get_history (optionmenu);
}
										
void on_prefs_def_notation_changed (GtkOptionMenu *optionmenu,
										gpointer user_data)
{
	prefs.def_number = gtk_option_menu_get_history (optionmenu);
}

void on_prefs_button_width_changed (GtkSpinButton *spinbutton,
											GtkScrollType arg1,
											gpointer user_data)
{
	extern GladeXML			*main_window_xml;
	
	prefs.button_width = (int) gtk_spin_button_get_value (spinbutton);
	set_button_group_size (main_window_xml, "table_func", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_bin", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_standard", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_dispctrl", prefs.button_width, prefs.button_height);
}

void on_prefs_button_height_changed (GtkSpinButton *spinbutton,
											GtkScrollType arg1,
											gpointer user_data)
{
	extern GladeXML			*main_window_xml;
	
	prefs.button_height = (int) gtk_spin_button_get_value (spinbutton);
	set_button_group_size (main_window_xml, "table_func", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_bin", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_standard", prefs.button_width, prefs.button_height);
	set_button_group_size (main_window_xml, "table_dispctrl", prefs.button_width, prefs.button_height);
}

void ms_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	extern s_array	memory;
	int		index;
	
	index = GPOINTER_TO_INT(user_data);
	if (index >= memory.len) {
		index = memory.len;
		memory.data = (double *) realloc (memory.data, (index + 1) * sizeof(double));
		memory.len++;
	}
	memory.data[index] = display_result_get_double();
}


void
on_ms_button_clicked             (GtkButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	extern s_array	memory;
	
	button_activation (button);
	menu = create_memory_menu(memory, (GCallback)ms_menu_handler, "save here");
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu, 
		button, 0, 0);
}

void mr_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	extern s_array	memory;
	int		index;
	
	index = GPOINTER_TO_INT(user_data);
	display_result_set_double(memory.data[index]);
}

void
on_mr_button_clicked             (GtkButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	extern s_array	memory;
	
	button_activation (button);
	menu = create_memory_menu(memory, (GCallback)mr_menu_handler, NULL);
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu,
		button, 0, 0);

}

void mplus_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	extern s_array	memory;
	int		index;
	
	index = GPOINTER_TO_INT(user_data);
	memory.data[index] += display_result_get_double();
}

void
on_mplus_button_clicked             (GtkButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	extern s_array	memory;
	
	button_activation (button);
	menu = create_memory_menu(memory, (GCallback)mplus_menu_handler, NULL);
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu,
		button, 0, 0);

}

void mc_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	extern s_array	memory;
	int		index, counter;
	
	index = GPOINTER_TO_INT(user_data);
	if (index >= memory.len) {
		if (memory.len > 0) free (memory.data);
		memory.data = NULL;
		memory.len = 0;
	} else {
		for (counter = index; counter < (memory.len - 1); counter++) 
			memory.data[counter] = memory.data[counter + 1];
		memory.len--;
		memory.data = (double *) realloc (memory.data, memory.len * sizeof(double));
	}
}

void
on_mc_button_clicked             (GtkButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	extern s_array	memory;
	
	button_activation (button);
	menu = create_memory_menu(memory, (GCallback)mc_menu_handler, "clear all");
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu,
		button, 0, 0);

}

void mx_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	extern s_array	memory;
	int		index, temp;
	
	index = GPOINTER_TO_INT(user_data);
	temp = memory.data[index];
	memory.data[index] = display_result_get_double();
	display_result_set_double (temp);
}

void
on_mx_button_clicked             (GtkButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	extern s_array	memory;
	
	button_activation (button);
	menu = create_memory_menu(memory, (GCallback)mx_menu_handler, NULL);
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu,
		button, 0, 0);

}

void on_prefs_cclear_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget	*entry;
	
	entry = glade_xml_get_widget (prefs_xml, "prefs_cname_entry");
	gtk_entry_set_text ((GtkEntry *) entry, "");
        entry = glade_xml_get_widget (prefs_xml, "prefs_cvalue_entry");
	gtk_entry_set_text ((GtkEntry *) entry, "");
        entry = glade_xml_get_widget (prefs_xml, "prefs_cdesc_entry");
	gtk_entry_set_text ((GtkEntry *) entry, "");
}

void on_prefs_cadd_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget		*entry;
	GtkTreeIter   		iter;
	extern s_constant	*constant;
	int			nr_consts;
	
	nr_consts = store->length;
	constant = (s_constant *) realloc (constant, (nr_consts + 2) * sizeof(s_constant));
	constant[nr_consts + 1].desc = NULL;
	
	entry = glade_xml_get_widget (prefs_xml, "prefs_cname_entry");
	constant[nr_consts].name = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
        entry = glade_xml_get_widget (prefs_xml, "prefs_cvalue_entry");
	constant[nr_consts].value = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
        entry = glade_xml_get_widget (prefs_xml, "prefs_cdesc_entry");
	constant[nr_consts].desc = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
	
	gtk_list_store_append (store, &iter);	
	gtk_list_store_set (store, &iter, 
		NAME_COLUMN, constant[nr_consts].name, 
		VALUE_COLUMN, constant[nr_consts].value, 
		DESC_COLUMN, constant[nr_consts].desc, 
		-1);
}

void on_prefs_cdelete_clicked (GtkButton *button, gpointer user_data)
{
	extern s_constant	*constant;
	GtkTreePath		*path;
	int			index, counter, nr_consts;
	
	nr_consts = store->length;
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &current_list_iter);
	index = *(gtk_tree_path_get_indices (path));

	gtk_list_store_remove (store, &current_list_iter);
	on_prefs_cclear_clicked (NULL, NULL);

	for (counter = index; counter < (nr_consts - 1); counter++)
		memcpy (&constant[counter], &constant[counter+1], sizeof(s_constant));
	
	nr_consts--;
	constant = (s_constant *) realloc (constant, (nr_consts + 1) * sizeof(s_constant));
	
	constant[nr_consts].desc = NULL;
}

void on_prefs_cupdate_clicked (GtkButton *button, gpointer user_data)
{
	extern s_constant	*constant;
	GtkWidget		*entry;
	GtkTreePath		*path;
	int			index;
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &current_list_iter);
	index = *(gtk_tree_path_get_indices (path));
	
	entry = glade_xml_get_widget (prefs_xml, "prefs_cname_entry");
	constant[index].name = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
        entry = glade_xml_get_widget (prefs_xml, "prefs_cvalue_entry");
	constant[index].value = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
        entry = glade_xml_get_widget (prefs_xml, "prefs_cdesc_entry");
	constant[index].desc = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
	
	gtk_list_store_set (store, &current_list_iter, 
		NAME_COLUMN, constant[index].name, 
		VALUE_COLUMN, constant[index].value, 
		DESC_COLUMN, constant[index].desc, 
		-1);
		
	
}
/* END */
