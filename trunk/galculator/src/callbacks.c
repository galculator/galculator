/*
 *  callbacks.c - functions to handle GUI events.
 *	part of galculator
 *  	(c) 2002-2003 Simon Floery (chimaira@users.sf.net)
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

#include "calc_basic.h"
#include "galculator.h"
#include "math_functions.h"
#include "general_functions.h"
#include "display.h"
#include "config_file.h"
#include "callbacks.h"
#include "ui.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>

#define SELECT_RESULT_FONT _("Select result font")
#define SELECT_STACK_FONT _("Select stack font")
#define SELECT_MODULE_FONT _("Select module font")
#define SELECT_ACT_MOD_COLOR _("Select active module color")
#define SELECT_INACT_MOD_COLOR _("Select inactive module color")
#define SELECT_RESULT_FONT_COLOR _("Select result font color")
#define SELECT_STACK_COLOR _("Select stack color")
#define SELECT_BKG_COLOR _("Select background color")
#define SELECT_BUTTON_FONT _("Select button font")	

static GtkWidget		*font_dialog, *color_dialog;

/* File */

void
on_quit_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	char 	**stack;
	
	/* remember some things */
	if (prefs.mode == SCIENTIFIC_MODE) {
		/* save number and angle mode only in scientific mode. */
		prefs.def_number = current_status.number;
		prefs.def_angle = current_status.angle;
	}
	if (prefs.rem_valuex) g_free (prefs.rem_valuex);
	prefs.rem_valuex = display_result_get();
	if (current_status.notation == CS_RPN) {
		if (prefs.rem_valuey) g_free (prefs.rem_valuey);
		if (prefs.rem_valuez) g_free (prefs.rem_valuez);
		if (prefs.rem_valuet) g_free (prefs.rem_valuet);
		/* we only save the visible stack */
		stack = display_stack_get_yzt();
		prefs.rem_valuey = g_strdup(stack[0]);
		g_free (stack[0]);
		prefs.rem_valuez = g_strdup(stack[1]);
		g_free (stack[1]);
		prefs.rem_valuet = g_strdup(stack[2]);
		g_free (stack[2]);
		g_free (stack);
	}
	prefs.def_notation = current_status.notation;
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
	
	about_dialog = ui_about_dialog_create();
	
	gtk_window_set_title ((GtkWindow *)about_dialog, g_strdup_printf (_("About %s"), PROG_NAME));
	about_label = (GtkLabel *) glade_xml_get_widget (about_dialog_xml, "about_label");
	gtk_label_set_justify (about_label, GTK_JUSTIFY_CENTER);
	about_text = g_strdup_printf (_("<span size=\"x-large\" weight=\"bold\">%s v%s</span>\n\
<span size=\"large\">a GTK 2 based scientific calculator</span>\n\n\
(c) 2002-2003 by Simon Floery (chimaira@users.sf.net)"), PROG_NAME, VERSION);
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
on_number_button_clicked               (GtkToggleButton  *button,
                                        gpointer         user_data)
{	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	if (current_status.notation == CS_FORMULA)
		ui_formula_entry_insert (gtk_button_get_label ((GtkButton *)button));
	else
		display_result_add_digit (*(gtk_button_get_label ((GtkButton *)button)));
	return;
}

/* this callback is called if a button for doing one of the arithmetic operations plus, minus, 
 * multiply, divide or power is clicked. it is mainly an interface to the calc_basic code.
 */

void
on_operation_button_clicked            (GtkToggleButton       *button,                                        gpointer         user_data)
{
	s_cb_token		current_token;
	double			return_value, *stack;
	GtkWidget		*tbutton;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	
	if (current_status.notation == CS_FORMULA) {
		ui_formula_entry_insert (
			g_object_get_data (G_OBJECT (button), "display_string"));
		return;
	}
	
	current_token.operation = (int) g_object_get_data (G_OBJECT (button), "operation");
	/* current number, get it from the display! */
	if (current_token.operation != '(') 
		current_token.number.value = display_result_get_double ();
	else current_token.number.func = NULL;	
	/* do inverse left shift is a right shift */
	if ((current_token.operation == '<') && \
		(BIT (current_status.fmod, CS_FMOD_FLAG_INV) == 1)) {
			tbutton = glade_xml_get_widget (button_box_xml, "button_inv");
			gtk_toggle_button_set_active ((GtkToggleButton *) tbutton, FALSE);
			current_token.operation = '>';
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
		
		if (((current_token.operation == '(') || current_status.allow_arith_op) && \
			((current_token.operation != ')') || (display_module_bracket_label_update (GET) > 0))) {
			return_value = alg_add_token (current_token);
			display_result_set_double (return_value);
			display_module_arith_label_update (current_token.operation);
			
			/* setting of allow_arith_op. the missing breaks are wanted */
			switch (current_token.operation) {
				case '=':
					display_module_bracket_label_update (RESET);
					break;
				case ')':
					display_module_bracket_label_update (ONE_LESS);
					break;
				case '(':
					display_module_bracket_label_update (ONE_MORE);
				default:
					current_status.allow_arith_op=FALSE;
			}
		}
	} else if (current_status.notation == CS_RPN) {
		switch (current_token.operation) {
		case '=':
			rpn_stack_push (current_token.number.value);
			stack = rpn_stack_get (RPN_FINITE_STACK);
			display_stack_set_yzt_double (stack);
			free (stack);
			current_status.rpn_have_result = FALSE;
			/* display line isn't cleared! */
			break;
		default:
			current_status.rpn_have_result = FALSE;
			display_result_set_double (rpn_stack_operation (current_token));
			stack = rpn_stack_get (RPN_FINITE_STACK);
			display_stack_set_yzt_double (stack);
			free (stack);
			current_status.rpn_have_result = TRUE;
		}
	} else error_message ("on_operation_button_clicked: unknown status");

	current_status.calc_entry_start_new = TRUE;
	return;
}

/* this callback is called if a button for a function manipulating the current 
 * entry directly is clicked. the array function_list knows the relation between 
 * button label and function to call.
 */

void
on_function_button_clicked             (GtkToggleButton	*button,
                                        gpointer user_data)
{
	double		(*func[4])(double);
	char 		**display_name;
	gboolean	is_trigonometric;
	GtkWidget	*tbutton;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	if (current_status.notation == CS_FORMULA) {
		display_name = (char **) g_object_get_data (G_OBJECT (button), "display_names");
		ui_formula_entry_insert (display_name[current_status.fmod]);
		return;
	}	
	memcpy (func, g_object_get_data (G_OBJECT (button), "func"), sizeof (func));
	is_trigonometric = (gboolean) g_object_get_data (G_OBJECT (button), "is_trigonometric");
	if (!func) error_message ("This button has no function associated with");
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
	current_status.calc_entry_start_new = TRUE;	
	if (current_status.notation == CS_RPN) current_status.rpn_have_result = TRUE;
	if (current_status.fmod != 0) {
		tbutton = glade_xml_get_widget (button_box_xml, "button_inv");
		gtk_toggle_button_set_active ((GtkToggleButton *) tbutton, FALSE);
		tbutton = glade_xml_get_widget (button_box_xml, "button_hyp");
		gtk_toggle_button_set_active ((GtkToggleButton *) tbutton, FALSE);
	}
}

void constants_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	char		*const_value;
	
	const_value = user_data;
	current_status.rpn_have_result = TRUE;
	display_result_set (const_value);
	current_status.rpn_have_result = TRUE;
	current_status.calc_entry_start_new = TRUE;
}


void
on_constant_button_clicked             (GtkToggleButton       *button,
                                        gpointer         user_data)
{
	GtkWidget		*menu;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	menu = ui_constants_menu_create(constant, (GCallback)constants_menu_handler);
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu, 
		button, 0, 0);
}

/* tbutton_fmod - these are function modifiers such as INV (inverse) 
 *	and HYP (hyperbolic).
 */

void
on_tbutton_fmod_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	if (strcmp (gtk_button_get_label (button), "inv") == 0)
		current_status.fmod ^= 1 << CS_FMOD_FLAG_INV;
	else if (strcmp (gtk_button_get_label (button), "hyp") == 0)
		current_status.fmod ^= 1 << CS_FMOD_FLAG_HYP;
	else error_message ("unknown function modifier (INV/HYP)");
}

void
on_gfunc_button_clicked                (GtkToggleButton       *button,
                                        gpointer         user_data)
{
	void	(*func)();
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	func = g_object_get_data (G_OBJECT (button), "func");
	if (func) func(button);
	else error_message ("This button has no function associated with");
}

/*
 * MENU
 */

void
on_dec_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	/*printf ("dec\n");*/
	display_change_option (CS_DEC, DISPLAY_OPT_NUMBER);
}


void
on_hex_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	/*printf ("hex\n");*/
	display_change_option (CS_HEX, DISPLAY_OPT_NUMBER);
}


void
on_oct_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	/*printf ("oct\n");*/
	display_change_option (CS_OCT, DISPLAY_OPT_NUMBER);
}


void
on_bin_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	/*printf ("bin\n");*/
	display_change_option (CS_BIN, DISPLAY_OPT_NUMBER);
}


void
on_deg_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	/*printf ("deg\n");*/
	display_change_option (CS_DEG, DISPLAY_OPT_ANGLE);
}


void
on_rad_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	/*printf ("rad\n");*/
	display_change_option (CS_RAD, DISPLAY_OPT_ANGLE);
}


void
on_grad_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (!gtk_check_menu_item_get_active((GtkCheckMenuItem *)menuitem)) return;
	/*printf ("grad\n");*/
	display_change_option (CS_GRAD, DISPLAY_OPT_ANGLE);
}

void
on_ordinary_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (((GtkCheckMenuItem *)menuitem)->active == FALSE) return;
	display_change_option (CS_PAN, DISPLAY_OPT_NOTATION);
	set_widget_visibility (main_window_xml, "formula_entry_hbox", FALSE);
	rpn_free ();
	all_clear ();
	set_button_label_and_tooltip (button_box_xml, "button_enter", 
		_("="), _("Enter"));
	set_button_label_and_tooltip (button_box_xml, "button_pow", 
		_("x^y"), _("Power"));
	set_button_label_and_tooltip (button_box_xml, "button_f1", 
		_("("), _("Open Bracket"));
	set_button_label_and_tooltip (button_box_xml, "button_f2", 
		_(")"), _("Close Bracket"));
	display_stack_remove();
	update_dispctrl();
	/* pixel above/below display result line */
	display_update_tags ();
}

void
on_rpn_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (((GtkCheckMenuItem *)menuitem)->active == FALSE) return;
	display_change_option (CS_RPN, DISPLAY_OPT_NOTATION);
	set_widget_visibility (main_window_xml, "formula_entry_hbox", FALSE);
	alg_free ();
	all_clear ();
	set_button_label_and_tooltip (button_box_xml, "button_enter", 
		_("ENT"), _("Enter"));
	set_button_label_and_tooltip (button_box_xml, "button_pow", 
		_("y^x"), _("Power"));
	set_button_label_and_tooltip (button_box_xml, "button_f1", 
		_("x<>y"), _("swap current number with top of stack"));
	set_button_label_and_tooltip (button_box_xml, "button_f2", 
		_("roll"), _("roll down stack"));
	/* stack is created by all_clear */
	update_dispctrl();
	/* pixel above/below display result line */
	display_update_tags ();
}

void 
on_form_activate 			(GtkMenuItem     *menuitem,
					gpointer         user_data)
{
	if (((GtkCheckMenuItem *)menuitem)->active == FALSE) return;
	display_change_option (CS_FORMULA, DISPLAY_OPT_NOTATION);
	all_clear();
	update_dispctrl();
	set_widget_visibility (main_window_xml, "formula_entry_hbox", TRUE);	
}

void
on_display_control_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	prefs.vis_dispctrl = 
		gtk_check_menu_item_get_active((GtkCheckMenuItem *) menuitem);
	set_widget_visibility (dispctrl_xml, "table_dispctrl", 
		prefs.vis_dispctrl);
}

void 
on_logical_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	if (prefs.mode == BASIC_MODE) return;

	prefs.vis_logic = 
		gtk_check_menu_item_get_active((GtkCheckMenuItem *) menuitem);
	set_widget_visibility (button_box_xml, "table_bin_buttons",
		prefs.vis_logic);
}

void
on_functions_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	if (prefs.mode == BASIC_MODE) return;
	prefs.vis_funcs = 
		gtk_check_menu_item_get_active((GtkCheckMenuItem *) menuitem);
	set_widget_visibility (button_box_xml, "table_func_buttons",
		prefs.vis_funcs);
}

void
on_standard_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	if (prefs.mode == BASIC_MODE) return;
	prefs.vis_standard = 
		gtk_check_menu_item_get_active((GtkCheckMenuItem *) menuitem);
	set_widget_visibility (button_box_xml, "table_standard_buttons",
		prefs.vis_standard);
}

void
on_basic_mode_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	GtkWidget	*menu_item;
	
	if (((GtkCheckMenuItem *) menuitem)->active == FALSE) return;
	if (prefs.mode == SCIENTIFIC_MODE) {
		/* remember number and angle. notation is active in basic mode */
		prefs.def_number = current_status.number;
		prefs.def_angle = current_status.angle;
	}
	prefs.mode = BASIC_MODE;
	
	ui_main_window_buttons_destroy ();
	ui_main_window_buttons_create (prefs.mode);
	update_dispctrl();
	
	display_update_modules();

	/* In basic mode:
	 *	- number base is always decimal.
	 *	- ignore angle, as there are no angle operations in basic mode.
	 *	- notation is fully functional.
	 */	
	display_module_number_activate (CS_DEC);
	display_module_notation_activate (current_status.notation);
	
	menu_item = glade_xml_get_widget (main_window_xml, "display_control");
	if (((GtkCheckMenuItem *) menu_item)->active == prefs.vis_dispctrl)
			gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	
	update_active_buttons (current_status.number, current_status.notation);
	menu_item = glade_xml_get_widget (main_window_xml, "functions");
	gtk_widget_set_sensitive (menu_item, FALSE);
	menu_item = glade_xml_get_widget (main_window_xml, "logical");
	gtk_widget_set_sensitive (menu_item, FALSE);
	menu_item = glade_xml_get_widget (main_window_xml, "standard");
	gtk_widget_set_sensitive (menu_item, FALSE);
	menu_item = glade_xml_get_widget (main_window_xml, "base_units");
	gtk_widget_set_sensitive (menu_item, FALSE);
	menu_item = glade_xml_get_widget (main_window_xml, "angle_units");
	gtk_widget_set_sensitive (menu_item, FALSE);
}

void
on_scientific_mode_activate (GtkMenuItem *menuitem,
				gpointer user_data)
{
	GtkWidget	*menu_item;
	
	if (((GtkCheckMenuItem *) menuitem)->active == FALSE) return;
	prefs.mode = SCIENTIFIC_MODE;

	ui_main_window_buttons_destroy ();
	ui_main_window_buttons_create (prefs.mode);

	display_update_modules();
	display_module_number_activate (prefs.def_number);
	display_module_angle_activate (prefs.def_angle);
	display_module_notation_activate (current_status.notation);

	update_active_buttons (current_status.number, current_status.notation);
	update_dispctrl();
	
	menu_item = glade_xml_get_widget (main_window_xml, "functions");
	gtk_widget_set_sensitive (menu_item, TRUE);
	/* what's that? the problem: glade only connects the activate signal
	 * for gtkmenucheckboxitem and gtk_check_menu_item_set_active doesn't
	 * emit a signal if change has not changed. so we force a change of
	 * state here.
	 */
	if (((GtkCheckMenuItem *) menu_item)->active == prefs.vis_funcs) 
			gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	
	menu_item = glade_xml_get_widget (main_window_xml, "display_control");
	if (((GtkCheckMenuItem *) menu_item)->active == prefs.vis_dispctrl)
			gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	
	menu_item = glade_xml_get_widget (main_window_xml, "logical");
	gtk_widget_set_sensitive (menu_item, TRUE);
	if (((GtkCheckMenuItem *) menu_item)->active == prefs.vis_logic)
			gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	
	menu_item = glade_xml_get_widget (main_window_xml, "standard");
	gtk_widget_set_sensitive (menu_item, TRUE);
	if (((GtkCheckMenuItem *) menu_item)->active == prefs.vis_standard)
			gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	gtk_menu_item_activate ((GtkMenuItem *) menu_item);
	
	menu_item = glade_xml_get_widget (main_window_xml, "base_units");
	gtk_widget_set_sensitive (menu_item, TRUE);
	menu_item = glade_xml_get_widget (main_window_xml, "angle_units");
	gtk_widget_set_sensitive (menu_item, TRUE);
}	

void
on_cut_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), 
		display_result_get(), -1);
	clear ();
}

void
on_paste_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	display_result_feed (gtk_clipboard_wait_for_text (
		gtk_clipboard_get (GDK_SELECTION_CLIPBOARD)));
}

void
on_copy_activate (GtkMenuItem     *menuitem,
			gpointer         user_data)
{
	gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), 
		display_result_get(), -1);
}

/*
 * Preferences
 */

void const_list_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
        GtkTreeModel 	*model;
	char 		*string;
	GtkWidget	*entry;
	GtkTreeIter 	current_list_iter;
	
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
	ui_pref_dialog_create();
}

void
on_prefs_result_font_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	font_dialog = ui_font_dialog_create (SELECT_RESULT_FONT, button);
}

void
on_prefs_result_color_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = ui_color_dialog_create (SELECT_RESULT_FONT_COLOR, button);
}

void
on_prefs_stack_font_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	font_dialog = ui_font_dialog_create (SELECT_STACK_FONT, button);
}

void
on_prefs_stack_color_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = ui_color_dialog_create (SELECT_STACK_COLOR, button);
}

void
on_prefs_mod_font_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	font_dialog = ui_font_dialog_create (SELECT_MODULE_FONT, button);
}


void
on_prefs_act_mod_color_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = ui_color_dialog_create (SELECT_ACT_MOD_COLOR, button);
}


void
on_prefs_inact_mod_color_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = ui_color_dialog_create (SELECT_INACT_MOD_COLOR, button);
}


void
on_prefs_bkg_color_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	color_dialog = ui_color_dialog_create (SELECT_BKG_COLOR, button);
}


void
on_prefs_button_font_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	font_dialog = ui_font_dialog_create (SELECT_BUTTON_FONT, button);
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
	
	if (strcmp (title, SELECT_BKG_COLOR) == 0)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_bkg_color");
		if (prefs.bkg_color != NULL) g_free (prefs.bkg_color);
		prefs.bkg_color = gdk_color_to_string(color);
		display_set_bkg_color (prefs.bkg_color);
	}
	else if (strcmp (title, SELECT_RESULT_FONT_COLOR) == 0)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_result_color");
		if (prefs.result_color != NULL) g_free (prefs.result_color);
		prefs.result_color = gdk_color_to_string(color);
		display_update_tags();
	}
	else if (strcmp (title, SELECT_STACK_COLOR) == 0)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_stack_color");
		if (prefs.stack_color != NULL) g_free (prefs.stack_color);
		prefs.stack_color = gdk_color_to_string(color);
		display_update_tags();
	}
	else if (strcmp (title, SELECT_ACT_MOD_COLOR) == 0)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_act_mod_color");
		if (prefs.act_mod_color != NULL) g_free (prefs.act_mod_color);
		prefs.act_mod_color = gdk_color_to_string(color);
		display_update_tags();
	}
	else if (strcmp (title, SELECT_INACT_MOD_COLOR) == 0)
	{
		da = glade_xml_get_widget (prefs_xml, "prefs_inact_mod_color");
		if (prefs.inact_mod_color != NULL) g_free (prefs.inact_mod_color);
		prefs.inact_mod_color = gdk_color_to_string(color);
		display_update_tags();
	}
	else fprintf (stderr, "[%s] Color Dialog (%s) not found. %s\n", PACKAGE, 
		title, BUG_REPORT);

	gtk_widget_really_modify_fg (da, color);
	
	gtk_widget_destroy (color_dialog);
}


void
on_font_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkButton		*font_button=NULL;
	const char 		*title;
	char			*font_name, *button_font;

	title = gtk_window_get_title ((GtkWindow *) font_dialog);
	font_name = gtk_font_selection_dialog_get_font_name ((GtkFontSelectionDialog *)font_dialog);
	
	if (strcmp (title, SELECT_RESULT_FONT) == 0)
	{
		font_button = (GtkButton *) glade_xml_get_widget (prefs_xml, "prefs_result_font");
		if (prefs.result_font != NULL) g_free (prefs.result_font);
		prefs.result_font = g_strdup(font_name);
		display_update_tags();
	}
	else if (strcmp (title, SELECT_STACK_FONT) == 0)
	{
		font_button = (GtkButton *) glade_xml_get_widget (prefs_xml, "prefs_stack_font");
		if (prefs.stack_font != NULL) g_free (prefs.stack_font);
		prefs.stack_font = g_strdup(font_name);
		display_update_tags();
	}
	else if (strcmp (title, SELECT_MODULE_FONT) == 0)
	{
		font_button = (GtkButton *) glade_xml_get_widget (prefs_xml, "prefs_mod_font");
		if (prefs.mod_font != NULL) g_free (prefs.mod_font);
		prefs.mod_font = g_strdup(font_name);
		display_update_tags();
	}
	else if (strcmp (title, SELECT_BUTTON_FONT) == 0)
	{
		font_button = (GtkButton *) glade_xml_get_widget (prefs_xml, "prefs_button_font");
		if (prefs.button_font != NULL) g_free (prefs.button_font);
		prefs.button_font = g_strdup(font_name);
		if (prefs.custom_button_font == TRUE) button_font = g_strdup (prefs.button_font);
		else button_font = g_strdup ("");
		set_all_buttons_font (button_font);	
		g_free (button_font);
	}
	else fprintf (stderr, "[%s] Font Dialog (%s) not found. %s\n", PACKAGE, 
		title, BUG_REPORT);
	
	gtk_widget_destroy (font_dialog);
	
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
	prefs.show_menu = gtk_check_menu_item_get_active ((GtkCheckMenuItem *) menuitem);;
	set_widget_visibility (main_window_xml, "menubar", prefs.show_menu);
}

void
on_prefs_custom_button_font_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	GtkWidget	*w;
	char		*button_font;
	
	prefs.custom_button_font = gtk_toggle_button_get_active (togglebutton);
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font_label");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
	w = glade_xml_get_widget (prefs_xml, "prefs_button_font");
	gtk_widget_set_sensitive (w, prefs.custom_button_font);
	
	if (prefs.custom_button_font == TRUE) button_font = g_strdup (prefs.button_font);
	else button_font = g_strdup ("");
	set_all_buttons_font (button_font);
	g_free (button_font);
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

void on_prefs_show_menu_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data)
{
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
	/* only is important when leaving galculator */
}
/*
void on_prefs_def_number_changed (GtkOptionMenu *optionmenu,
				gpointer user_data)
{
	prefs.def_number = gtk_option_menu_get_history (optionmenu);
}
										
void on_prefs_def_angle_changed (GtkOptionMenu *optionmenu,
				gpointer user_data)
{
	prefs.def_angle = gtk_option_menu_get_history (optionmenu);
}
										
void on_prefs_def_notation_changed (GtkOptionMenu *optionmenu,
				gpointer user_data)
{
	prefs.def_notation = gtk_option_menu_get_history (optionmenu);
}
*/
void on_prefs_button_width_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data)
{
	prefs.button_width = (int) gtk_spin_button_get_value (spinbutton);
	set_all_buttons_size (prefs.button_width, prefs.button_height);
}

void on_prefs_button_height_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data)
{	
	prefs.button_height = (int) gtk_spin_button_get_value (spinbutton);
	set_all_buttons_size (prefs.button_width, prefs.button_height);
}

void ms_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	int		index;
	
	index = GPOINTER_TO_INT(user_data);
	if (index >= memory.len) {
		index = memory.len;
		memory.data = (double *) realloc (memory.data, (index + 1) * sizeof(double));
		memory.len++;
	}
	memory.data[index] = display_result_get_double();
	current_status.calc_entry_start_new = TRUE;
}

void
on_ms_button_clicked (GtkToggleButton       *button,
			gpointer user_data)
{
	GtkWidget	*menu;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	menu = ui_memory_menu_create (memory, (GCallback)ms_menu_handler, _("save here"));
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu, 
		button, 0, 0);
}

void mr_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	int		index;
	
	index = GPOINTER_TO_INT(user_data);
	display_result_set_double(memory.data[index]);
	current_status.rpn_have_result = TRUE;
	current_status.calc_entry_start_new = TRUE;
}

void
on_mr_button_clicked             (GtkToggleButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	menu = ui_memory_menu_create(memory, (GCallback)mr_menu_handler, NULL);
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu,
		button, 0, 0);

}

void mplus_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	int		index;
	
	index = GPOINTER_TO_INT(user_data);
	memory.data[index] += display_result_get_double();
}

void
on_mplus_button_clicked             (GtkToggleButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	menu = ui_memory_menu_create(memory, (GCallback)mplus_menu_handler, NULL);
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu,
		button, 0, 0);

}

void mc_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
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
on_mc_button_clicked             (GtkToggleButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	menu = ui_memory_menu_create(memory, (GCallback)mc_menu_handler, "clear all");
	gtk_menu_popup ((GtkMenu *)menu, NULL, NULL, (GtkMenuPositionFunc) position_menu,
		button, 0, 0);

}

void mx_menu_handler (GtkMenuItem *menuitem, gpointer user_data)
{
	int		index, temp;
	
	index = GPOINTER_TO_INT(user_data);
	temp = memory.data[index];
	memory.data[index] = display_result_get_double();
	display_result_set_double (temp);
}

void
on_mx_button_clicked             (GtkToggleButton       *button,
				gpointer         user_data)
{
	GtkWidget	*menu;
	
	if (gtk_toggle_button_get_active(button) == FALSE) return;
	button_activation (button);
	menu = ui_memory_menu_create(memory, (GCallback)mx_menu_handler, NULL);
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
	int			nr_consts;
	char 			*name, *value, *desc;
	
	entry = glade_xml_get_widget (prefs_xml, "prefs_cname_entry");
	name = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
        entry = glade_xml_get_widget (prefs_xml, "prefs_cvalue_entry");
	value = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
        entry = glade_xml_get_widget (prefs_xml, "prefs_cdesc_entry");
	desc = g_strdup (gtk_entry_get_text ((GtkEntry *) entry));
	
	if ((strlen(name) == 0) || (strlen(value) == 0) || (strlen(desc) == 0)) {
		g_free (name);
		g_free (value);
		g_free (desc);
		return;
	}
		
	nr_consts = store->length;
	constant = (s_constant *) realloc (constant, (nr_consts + 2) * sizeof(s_constant));
	constant[nr_consts + 1].desc = NULL;
	
	constant[nr_consts].name = name;
	constant[nr_consts].value = value;
	constant[nr_consts].desc = desc;
	
	gtk_list_store_append (store, &iter);	
	gtk_list_store_set (store, &iter, 
		NAME_COLUMN, constant[nr_consts].name, 
		VALUE_COLUMN, constant[nr_consts].value, 
		DESC_COLUMN, constant[nr_consts].desc, 
		-1);
}

void on_prefs_cdelete_clicked (GtkButton *button, gpointer user_data)
{
	GtkTreePath		*path;
	int			index, counter, nr_consts;
	GtkTreeIter		current_list_iter;
		
	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection(
		(GtkTreeView *)glade_xml_get_widget (prefs_xml, "treeview1")),
		NULL, &current_list_iter)) return;
	
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
	GtkWidget	*entry;
	GtkTreePath	*path;
	int		index;
	GtkTreeIter 	current_list_iter;
	
	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection(
		(GtkTreeView *)glade_xml_get_widget (prefs_xml, "treeview1")),
		NULL, &current_list_iter)) return;
	
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

void on_prefs_hex_bits_value_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data)
{	
	prefs.hex_bits = (int) gtk_spin_button_get_value (spinbutton);
}

void on_prefs_hex_signed_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data)
{
	prefs.hex_signed = gtk_toggle_button_get_active (togglebutton);
}

void on_prefs_oct_bits_value_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data)
{
	prefs.oct_bits = (int) gtk_spin_button_get_value (spinbutton);
}

void on_prefs_oct_signed_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data)
{
	prefs.oct_signed = gtk_toggle_button_get_active (togglebutton);
}

void on_prefs_bin_bits_value_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data)
{
	prefs.bin_bits = (int) gtk_spin_button_get_value (spinbutton);
}


void on_prefs_bin_signed_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data)
{
	prefs.bin_signed = gtk_toggle_button_get_active (togglebutton);
}

void on_prefs_bin_fixed_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data)
{
	GtkWidget	*w;
	
	prefs.bin_fixed = gtk_toggle_button_get_active (togglebutton);
	w = glade_xml_get_widget (prefs_xml, "prefs_bin_length");
	gtk_widget_set_sensitive (w, prefs.bin_fixed);
}

void on_prefs_bin_length_value_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data)
{
	prefs.bin_length = (int) gtk_spin_button_get_value (spinbutton);
}

void on_togglebutton_released (GtkToggleButton *togglebutton, 
					gpointer user_data)
{
	gtk_toggle_button_set_active (togglebutton, FALSE);
}

void on_main_window_check_resize (GtkContainer *container,
                                            gpointer user_data)
{
	static gboolean		itsme=FALSE;
	
	/* is there a nicer way to to this? */
	if (itsme) {
		itsme = FALSE;
		return;
	}
	gtk_window_resize ((GtkWindow *)gtk_widget_get_toplevel((GtkWidget *)container), 1, 1);
	itsme = TRUE;
}

void on_finite_stack_size_clicked (GtkRadioButton *rb, gpointer user_data)
{
	prefs.stack_size = RPN_FINITE_STACK;
	rpn_stack_set_size (prefs.stack_size);
}

void on_infinite_stack_size_clicked (GtkRadioButton *rb, gpointer user_data)
{
	prefs.stack_size = RPN_INFINITE_STACK;
	rpn_stack_set_size (prefs.stack_size);
}

void on_main_window_button_press_event(GtkWidget *widget,
						GdkEventButton *event,
						gpointer user_data)
{
	printf ("Hallo\n");
}

gboolean on_button_press_event (GtkWidget *widget,
						GdkEventButton *event,
						gpointer user_data)
{
	printf ("Hallo22\n");
	return FALSE;
}


/* END */
