/*s
 *  display.c - code for this nifty display.
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
#include <ctype.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <glade/glade.h>

#include "galculator.h"
#include "display.h"
#include "general_functions.h"
#include "config_file.h"
#include "math_functions.h"
#include "calc_basic.h"

GtkTextView 		*view;
GtkTextBuffer 		*buffer;
static int 		display_result_counter = 0;
gboolean 		calc_entry_start_new;
extern char		dec_point;
extern s_preferences	prefs;
extern s_current_status current_status;

char	display_result[30];

char	*number_mod_labels[5] = {" DEC ", " HEX ", " OCT ", " BIN ", NULL}, 
	*angle_mod_labels[4] = {" DEG ", " RAD ", " GRAD ", NULL},
	*notation_mod_labels[3] = {" ALG ", " RPN ", NULL};	

int 	display_lengths[NR_NUMBER_BASES] = {12, 0, 0, 0};

/*
 * display.c mainly consists of two parts: first display setup code
 * and second display manipulation code.
 */

/*
 * activate_menu_item - activates menu item with widget name item_name
 */

void activate_menu_item (char *item_name)
{
	extern GladeXML		*main_window_xml;
	GtkMenuItem		*current_item;
	
	current_item = (GtkMenuItem *) glade_xml_get_widget (main_window_xml, \
		g_strstrip (g_ascii_strdown (item_name, -1)));
	gtk_menu_item_activate (current_item);
}

/*
 * display_create_text_tags - creates a tag for the result and (in-)active modules
 */

void display_create_text_tags ()
{
	/* note: wrap MUST NOT be set to none in order to justify the text! */
	gtk_text_buffer_create_tag (buffer, "result", \
		"justification", GTK_JUSTIFY_RIGHT, \
		"font", prefs.result_font, \
		"pixels_above_lines", 5, \
		"pixels_below_lines", 5, \
		"foreground", prefs.result_color, \
		NULL);	
	
	gtk_text_buffer_create_tag (buffer, "active_module", \
 		"font", prefs.mod_font, \
		"foreground", prefs.act_mod_color, \
		"wrap-mode", GTK_WRAP_NONE, \
		NULL);
	
	gtk_text_buffer_create_tag (buffer, "inactive_module", \
		"font", prefs.mod_font, \
		"foreground", prefs.inact_mod_color, \
		"wrap-mode", GTK_WRAP_NONE, \
		NULL);
}

/*
 * display_init. Call this function before any other function of this file.
 */

void display_init (GtkWidget *a_parent_widget)
{
	GtkTextIter 		iter;
	GdkColor		color;
	int			char_width;
	PangoContext 		*pango_context;
	PangoFontMetrics 	*font_metrics;
	GtkTextTag		*tag;
	PangoTabArray		*tab_array;
	GtkTextTagTable		*tag_table;
	extern GladeXML		*main_window_xml;
	
	calc_entry_start_new = FALSE;
	view = (GtkTextView *) glade_xml_get_widget (main_window_xml, "textview");
	gdk_color_parse (prefs.bkg_color, &color);
	gtk_widget_modify_base ((GtkWidget *)view, GTK_STATE_NORMAL, &color);
	
	buffer = gtk_text_view_get_buffer (view);
	display_create_text_tags ();

	// compute the approx char/digit width and create a tab stops
	tag_table = gtk_text_buffer_get_tag_table (buffer);
	tag = gtk_text_tag_table_lookup (tag_table, "active_module");
	// get the approx char width	
	pango_context = gtk_widget_get_pango_context ((GtkWidget *)view);
	font_metrics = pango_context_get_metrics(pango_context, \
		tag->values->font,
		tag->values->language);
	char_width = MAX (pango_font_metrics_get_approximate_char_width (font_metrics), \
		pango_font_metrics_get_approximate_digit_width (font_metrics));
	tab_array = pango_tab_array_new_with_positions (1, FALSE, PANGO_TAB_LEFT, 3*char_width);
	gtk_text_view_set_tabs (view, tab_array);
	pango_tab_array_free (tab_array);
	
	gtk_text_buffer_get_iter_at_line (buffer, &iter, DISPLAY_RESULT_LINE);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, 
		prefs.rem_value, -1, "result", NULL);
	
	display_update_modules ();

	/* after creating the modules, we set the three base modules and the init
		state of the calculator:
		at first, we set the notation as this clears the display. Afterwards,  
		we set the display to a maybe remembered value. Then, angle and number base
		are done
	*/
	
	activate_menu_item (notation_mod_labels[prefs.def_notation]);
	activate_menu_item (number_mod_labels[prefs.def_number]);
	activate_menu_item (angle_mod_labels[prefs.def_angle]);

	display_lengths[CS_HEX] = prefs.hex_bits/4;
	display_lengths[CS_OCT] = prefs.oct_bits/3;
	display_lengths[CS_BIN] = prefs.bin_bits/1;
}

/*
 * display_module_arith_label_update - code for the "display last arithmetic  
 * operation" display module. put operation in static current_char to survive
 * an apply f preferences.
 */

void display_module_arith_label_update (char operation)
{
	GtkTextMark	*this_mark;
	GtkTextIter	start, end;
	static char 	current_char=' ';
	
	if (prefs.vis_arith == FALSE) return;
	if (strchr ("()", operation) != NULL) return;
	
	if ((this_mark = gtk_text_buffer_get_mark (buffer, DISPLAY_MARK_ARITH)) == NULL) \
		return;
	
	gtk_text_buffer_get_iter_at_mark (buffer, &start, this_mark);
	end = start;
	gtk_text_iter_forward_chars (&end, 3);
	gtk_text_buffer_delete (buffer, &start, &end);
	
	gtk_text_buffer_get_iter_at_mark (buffer, &start, this_mark);
	if (operation != NOP) current_char = operation;

	gtk_text_buffer_insert_with_tags_by_name (buffer, &start, \
		g_strdup_printf ("\t%c\t", current_char), -1, "active_module", NULL);
}

/*
 * display_module_bracket_label_update - code for the "display number of open
 * brackets" display module
 */

int display_module_bracket_label_update (int option)
{
	GtkTextMark	*this_mark;
	GtkTextIter	start, end;
	static int	nr_brackets=0;
	int		forward_count=2;
	char 		*string;
	
	switch (option) {
		case ONE_MORE:
			nr_brackets++;
			if (nr_brackets > 0) forward_count = 5 + log10(nr_brackets);
			break;
		case ONE_LESS:
			if (nr_brackets > 0) forward_count = 5 + log10(nr_brackets);
			nr_brackets--;
			break;
		case RESET:
			if (nr_brackets > 0) forward_count = 5 + log10(nr_brackets);
			nr_brackets = 0;
			break;
		case GET:
			/* doing this here to not touch the display */
			return nr_brackets;
		case NOP:
			if (nr_brackets > 0) forward_count = 5 + log10(nr_brackets);
			break;
	}
	if (prefs.vis_bracket == FALSE) return nr_brackets;
	
	if ((this_mark = gtk_text_buffer_get_mark (buffer, DISPLAY_MARK_BRACKET)) == NULL) \
		return nr_brackets;
	gtk_text_buffer_get_iter_at_mark (buffer, &start, this_mark);
	end = start;
	gtk_text_iter_forward_chars (&end, forward_count);
	gtk_text_buffer_delete (buffer, &start, &end);

	gtk_text_buffer_get_iter_at_mark (buffer, &start, this_mark);

	if (nr_brackets > 0) {
		string = g_strdup_printf ("\t(%i\t", nr_brackets);
		gtk_text_buffer_insert_with_tags_by_name (buffer, &start, string, \
			-1, "active_module", NULL);
		g_free (string);	
	}
	else {
		nr_brackets = 0;
		gtk_text_buffer_insert_with_tags_by_name (buffer, &start, "\t\t", \
			-1, "active_module", NULL);
	}
	return nr_brackets;
}

/*
 * display_module_base_create - start_mark is the mark where this module starts.
 * Therefore, the single labels are inserted starting with the last (as the mark
 * is at the beginning!). A simple gtk_text_buffer_insert_with_tags_by_name
 * doesn't work as we insert in an already tagged region an tag priority rules out.
 * solving this matter with setting priorities is not possible, so we have to remove
 * all tags and apply the tag we want.
 */

void display_module_base_create (char **module_label, char *mark_name, int active_index)
{
	int		label_counter=0, counter;
	GtkTextIter 	start, end;
	GtkTextMark	*this_mark;
	
	if ((this_mark = gtk_text_buffer_get_mark (buffer, mark_name)) == NULL) return;
	while (module_label[label_counter] != NULL) label_counter++;
	for (counter = (label_counter-1); counter >= 0; counter--) {
		gtk_text_buffer_get_iter_at_mark (buffer, &end, this_mark);
		gtk_text_buffer_insert (buffer, &end, module_label[counter], -1);
		gtk_text_buffer_get_iter_at_mark (buffer, &start, this_mark);
		gtk_text_buffer_remove_all_tags (buffer, &start, &end);
		if (counter == active_index) {
			gtk_text_buffer_apply_tag_by_name (buffer, "active_module", &start, &end);
		} else {
			gtk_text_buffer_apply_tag_by_name (buffer, "inactive_module", &start, &end);	
		}
	}
}

/*
 * display_module_leading_spaces - the distance between two modules is done with 
 * spaces. when inserting this spaces, we have to pay attention that it really 
 * is between two modules (e.g. we want no spaces after the last module) and 
 * that we don't lose the text mark pointing to the start of a module.
 */

void display_module_leading_spaces (char *mark_name, gboolean leading_spaces)
{	
	GtkTextIter	iter;
	GtkTextMark	*start_mark;
	
	if (leading_spaces == FALSE) return;
	
	if ((start_mark = gtk_text_buffer_get_mark (buffer, mark_name)) == NULL) return;
	gtk_text_buffer_get_iter_at_mark (buffer, &iter, start_mark);
	// insert spaces
	gtk_text_buffer_insert (buffer, &iter, DISPLAY_MODULES_DELIM, -1);
	// update text mark
	gtk_text_buffer_delete_mark (buffer, start_mark);
	gtk_text_buffer_create_mark (buffer, mark_name, &iter, TRUE);
}

void display_get_line_end_iter (GtkTextBuffer *b, int line_index, GtkTextIter *end)
{
	gtk_text_buffer_get_iter_at_line (b, end, line_index);
	gtk_text_iter_forward_to_line_end (end);
}

void display_get_line_iters (GtkTextBuffer *b, int line_index, GtkTextIter *start, GtkTextIter *end)
{	
	gtk_text_buffer_get_iter_at_line (b, start, line_index);
	*end = *start;
	gtk_text_iter_forward_to_line_end (end);
}

void display_delete_line (GtkTextBuffer *b, int line_index, GtkTextIter *iter)
{
	GtkTextIter start, end;
	
	if (gtk_text_buffer_get_line_count (b) <= line_index) return;
	display_get_line_iters (b, line_index, &start, &end);
	gtk_text_buffer_delete (buffer, &start, &end);
	*iter = start;
}

/*
 * display_create_modules - the display modules compose the second line of then
 *	display. available modules:
 *		- change number base
 *		- change angle base
 *		- change notation
 */

void display_update_modules ()
{
	GtkTextIter	start, end;
	gboolean	first_module = TRUE;
	
	
	display_get_line_end_iter (buffer, DISPLAY_RESULT_LINE, &start);
	display_get_line_end_iter (buffer, DISPLAY_MODULES_LINE, &end);
	gtk_text_buffer_delete (buffer, &start, &end);
	
	/* change number base */
	if (prefs.vis_number == TRUE) {
		if (first_module) 
			gtk_text_buffer_insert_with_tags_by_name (buffer, &start, "\n", \
				-1, "result", NULL);
		gtk_text_buffer_get_iter_at_line (buffer, &start, DISPLAY_MODULES_LINE);
		gtk_text_iter_forward_to_line_end (&start);
		if (gtk_text_buffer_get_mark (buffer, DISPLAY_MARK_NUMBER) != NULL) \
			gtk_text_buffer_delete_mark_by_name (buffer, DISPLAY_MARK_NUMBER);
		gtk_text_buffer_create_mark (buffer, DISPLAY_MARK_NUMBER, &start, TRUE);
		display_module_base_create (number_mod_labels, DISPLAY_MARK_NUMBER, current_status.number);
		display_module_leading_spaces (DISPLAY_MARK_NUMBER, !first_module);
		first_module = FALSE;
	}
	
	/* angle */
	if (prefs.vis_angle == TRUE) {
		if (first_module) 
			gtk_text_buffer_insert_with_tags_by_name (buffer, &start, "\n", \
				-1, "result", NULL);
		gtk_text_buffer_get_iter_at_line (buffer, &start, DISPLAY_MODULES_LINE);
		gtk_text_iter_forward_to_line_end (&start);
		if (gtk_text_buffer_get_mark (buffer, DISPLAY_MARK_ANGLE) != NULL) \
			gtk_text_buffer_delete_mark_by_name (buffer, DISPLAY_MARK_ANGLE);
		gtk_text_buffer_create_mark (buffer, DISPLAY_MARK_ANGLE, &start, TRUE);
		display_module_base_create (angle_mod_labels, DISPLAY_MARK_ANGLE, current_status.angle);
		display_module_leading_spaces (DISPLAY_MARK_ANGLE, !first_module);
		first_module = FALSE;
	}
	
	/* notation */
	if (prefs.vis_notation == TRUE) {
		if (first_module) 
			gtk_text_buffer_insert_with_tags_by_name (buffer, &start, "\n", \
				-1, "result", NULL);
		gtk_text_buffer_get_iter_at_line (buffer, &start, DISPLAY_MODULES_LINE);
		gtk_text_iter_forward_to_line_end (&start);
		if (gtk_text_buffer_get_mark (buffer, DISPLAY_MARK_NOTATION) != NULL) \
			gtk_text_buffer_delete_mark_by_name (buffer, DISPLAY_MARK_NOTATION);
		gtk_text_buffer_create_mark (buffer, DISPLAY_MARK_NOTATION, &start, TRUE);
		display_module_base_create (notation_mod_labels, DISPLAY_MARK_NOTATION, current_status.notation);
		display_module_leading_spaces (DISPLAY_MARK_NOTATION, !first_module);
		first_module = FALSE;
	}	
	
	/* last arithmetic operation */
	if (prefs.vis_arith == TRUE) {
		if (first_module) 
			gtk_text_buffer_insert_with_tags_by_name (buffer, &start, "\n", \
				-1, "result", NULL);
		gtk_text_buffer_get_iter_at_line (buffer, &start, DISPLAY_MODULES_LINE);
		gtk_text_iter_forward_to_line_end (&start);
		if (gtk_text_buffer_get_mark (buffer, DISPLAY_MARK_ARITH) != NULL) \
			gtk_text_buffer_delete_mark_by_name (buffer, DISPLAY_MARK_ARITH);
		gtk_text_buffer_create_mark (buffer, DISPLAY_MARK_ARITH, &start, TRUE);
		display_module_arith_label_update (NOP);
		display_module_leading_spaces (DISPLAY_MARK_ARITH, !first_module);
		first_module = FALSE;
	}
	
	/* number of open brackets */
	if (prefs.vis_bracket == TRUE) {
		if (first_module) 
			gtk_text_buffer_insert_with_tags_by_name (buffer, &start, "\n", \
				-1, "result", NULL);
		gtk_text_buffer_get_iter_at_line (buffer, &start, DISPLAY_MODULES_LINE);
		gtk_text_iter_forward_to_line_end (&start);
		if (gtk_text_buffer_get_mark (buffer, DISPLAY_MARK_BRACKET) != NULL) \
			gtk_text_buffer_delete_mark_by_name (buffer, DISPLAY_MARK_BRACKET);
		gtk_text_buffer_create_mark (buffer, DISPLAY_MARK_BRACKET, &start, TRUE);
		display_module_bracket_label_update (NOP);
		display_module_leading_spaces (DISPLAY_MARK_BRACKET, !first_module);
		first_module = FALSE;
	}
}

/*
 * display_module_base_delete - delete the given module at given text mark.
 * used for display_change_option
 */

void display_module_base_delete (char *mark_name, char **text)
{
	int		counter=0, length=0;
	GtkTextIter	start, end;
	GtkTextMark	*this_mark;
	
	if ((this_mark = gtk_text_buffer_get_mark (buffer, mark_name)) == NULL) return;
		
	while (text[counter] != NULL) {
		length+=strlen(text[counter]);
		counter++;
	}

	gtk_text_buffer_get_iter_at_mark (buffer, &start, this_mark);
	end = start;
	gtk_text_iter_forward_chars (&end, length);
	gtk_text_buffer_delete (buffer, &start, &end);
}


/*
 * display_change_option - changes CURRENT_STATUS (!) and updates the display. 
 * The last function in the signal handling cascade of changing base etc.
 */

void display_change_option (int new_status, int opt_group)
{
	int					old_status;
	double				display_value=0;
	extern GladeXML		*main_window_xml;
	
	switch (opt_group) {
		case DISPLAY_OPT_NUMBER:
			update_active_buttons (main_window_xml, new_status, current_status.notation);
			if (current_status.number == new_status) return;
			display_value = display_result_get_double ();
			old_status = current_status.number;
			current_status.number = new_status;
			display_result_set_double (display_value);
			if (prefs.vis_number) {
				display_module_base_delete (DISPLAY_MARK_NUMBER, number_mod_labels);
				display_module_base_create (number_mod_labels, DISPLAY_MARK_NUMBER, current_status.number);
			}
			break;
		case DISPLAY_OPT_ANGLE:
			if (current_status.angle == new_status) return;
			old_status = current_status.angle;
			current_status.angle = new_status;
			if (prefs.vis_angle) {
				display_module_base_delete (DISPLAY_MARK_ANGLE, angle_mod_labels);
				display_module_base_create (angle_mod_labels, DISPLAY_MARK_ANGLE, current_status.angle);
			}
			break;
		case DISPLAY_OPT_NOTATION:
			update_active_buttons (main_window_xml, current_status.number, new_status);
			if (current_status.notation == new_status) return;
			old_status = current_status.notation;
			current_status.notation = new_status;
			if (prefs.vis_notation) {
				display_module_base_delete (DISPLAY_MARK_NOTATION, notation_mod_labels);
				display_module_base_create (notation_mod_labels, DISPLAY_MARK_NOTATION, current_status.notation);
			}
			break;
		default:
			fprintf (stderr, _("[%s] unknown display option in function \"display_change_option\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	calc_entry_start_new = TRUE;
}

/*
 * display_set_bkg_color - change the background color of the text view
 */

void display_set_bkg_color (char *color_string)
{
	GdkColor	color;
	
	gdk_color_parse (color_string, &color);
	gtk_widget_modify_base ((GtkWidget *)view, GTK_STATE_NORMAL, &color);
}

/*
 * if we change any settings for result/module fonts this function updates the
 * all the tags.
 */

void display_update_tags ()
{
	GtkTextIter		start, end;
	GtkTextTagTable		*tag_table;
	GtkTextTag 		*tag;
	
	// remove tag "result" from tag_table, so we can define a new tag named "result"
	tag_table = gtk_text_buffer_get_tag_table (buffer);
	tag = gtk_text_tag_table_lookup (tag_table, "result");
	gtk_text_tag_table_remove (tag_table, tag);
	tag = gtk_text_tag_table_lookup (tag_table, "inactive_module");
	gtk_text_tag_table_remove (tag_table, tag);
	tag = gtk_text_tag_table_lookup (tag_table, "active_module");
	gtk_text_tag_table_remove (tag_table, tag);
	
	display_create_text_tags ();
	
	display_get_line_iters (buffer, DISPLAY_RESULT_LINE, &start, &end);
	gtk_text_buffer_apply_tag_by_name (buffer, "result", &start, &end);
	
	display_update_modules ();
}

/*
 * END of display CONFIGuration code.
 * from here on display manipulation code.
 */

/*
 * display_result_add_digit. appends the given digit to the current entry, 
 * handles zeros, decimal points. call e.g. with *(gtk_button_get_label (button))
 */

void display_result_add_digit (char digit)
{
	char			digit_as_string[2];
	extern gboolean		rpn_have_result;
	GtkTextIter		end;
	
	/* put a ev. result onto the stack */
	/* an alternative idea would be to do this immediately after getting a result
	   in calc_basic's rpn routines or in on_function_button_clicked. but if we
	   use on_function_button_clicked twice, there are two results on the stack
	   where we don't expect the older one to be there. therefore doing it this way
	*/
	if ((current_status.notation == CS_RPN) && (rpn_have_result == TRUE)) {
		calc_rpn_stack_add (display_result_get_double ());
		rpn_have_result = FALSE;
	}
	
	digit_as_string[0] = digit;
	digit_as_string[1] = '\0';
	
	if (calc_entry_start_new == TRUE) {
		/* fool the following code */
		display_result_set ("0");
		calc_entry_start_new = FALSE;
		display_result_counter = 1;
	}
	if (digit == dec_point) {
		/* don't manipulate display_result_counter here! */
		if (strlen (display_result_get()) == 0) display_result_set ("0");
		else if ((strchr (display_result_get(), dec_point) == NULL) && \
					(strchr (display_result_get(), 'e') == NULL)) {
			display_get_line_end_iter (buffer, DISPLAY_RESULT_LINE, &end);
			gtk_text_buffer_insert_with_tags_by_name (buffer, &end, digit_as_string, \
				-1, "result", NULL);
		}
	} else {
		if (strcmp (display_result_get(), "0") == 0) display_result_set (digit_as_string);
		else if (display_result_counter < display_lengths[current_status.number]) {
			display_get_line_end_iter (buffer, DISPLAY_RESULT_LINE, &end);
			gtk_text_buffer_insert_with_tags_by_name (buffer, &end, digit_as_string, \
				-1, "result", NULL);
			/* increment counter only in this if directive as above the counter remains 1! */
			display_result_counter++;
		}
	}
}

/*
 * display_result_set_double. set text of calc_entry to string given by float value.
 *	the float value is manipulated (rounded, ...)
 */

void display_result_set_double (double value)
{
	GtkTextIter 		start;
	char			*string_value;
	extern gboolean		allow_arith_op;
	
	allow_arith_op = TRUE;
	display_module_arith_label_update (' ');
	
	/* at first clear the result field */
	
	display_delete_line (buffer, DISPLAY_RESULT_LINE, &start);
	
	switch (current_status.number) {
		case CS_DEC:
			string_value = g_strdup_printf ("%.*g", display_lengths[current_status.number], value);
			break;
		case CS_HEX:
			string_value = ftoax (value, 16, prefs.hex_bits, prefs.hex_signed);
			break;
		case CS_OCT:
			string_value = ftoax (value, 8, prefs.oct_bits, prefs.oct_signed);
			break;
		case CS_BIN:
			string_value = ftoax (value, 2, prefs.bin_bits, prefs.bin_signed);
			break;
		default:
			string_value = _("unknown number base");
			fprintf (stderr, _("[%s] unknown number base in function \"display_result_set_double\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	gtk_text_buffer_insert_with_tags_by_name (buffer, &start, string_value, -1, "result", NULL);
	display_result_counter = strlen (string_value);
	g_free (string_value);
}

/*
 * display_result_set_radiant - this function is used e.g. by trigonometric functions.
 *	pays attention to rad/deg/grad
 */

void display_result_set_radiant (double value)
{
	switch (current_status.angle) {
	case CS_DEG:
		display_result_set_double (rad2deg(value)); 
		break;
	case CS_RAD:
		display_result_set_double (value); 
		break;
	case CS_GRAD:
		display_result_set_double (rad2grad(value));
		break;
	default:
		fprintf (stderr, _("[%s] unknown angle base in function \"display_result_set_angle\". %s\n"), PROG_NAME, BUG_REPORT);
	}
}

void display_result_set (char *string_value)
{
	GtkTextIter 		end;
	extern gboolean		allow_arith_op;
	
	allow_arith_op = TRUE;
	display_module_arith_label_update (' ');
	
	/* at first clear the result field */
	
	display_delete_line (buffer, DISPLAY_RESULT_LINE, &end);
	
	/* here we call no kill_trailing_zeros. we set the result_field to what we entered */
	gtk_text_buffer_insert_with_tags_by_name (buffer, &end, string_value, \
		-1, "result", NULL);
	display_result_counter = strlen (string_value);
	
	/* this is some cosmetics. try to keep counter up2date */
	if (strchr (string_value, dec_point) != NULL) display_result_counter--;
	if (strchr (string_value, 'e') != NULL) {
		display_result_counter -= (strchr(string_value, 'e') + sizeof(char) - string_value)/sizeof(char);
		display_result_counter += display_lengths[current_status.number] - DISPLAY_RESULT_E_LENGTH - 1;
	}
}

/*
 * display_get_entry. returns a pointer to the current entry text. should be freed with g_free.
 */

char *display_result_get ()
{
	GtkTextIter 	start, end;
	
	display_get_line_iters (buffer, DISPLAY_RESULT_LINE, &start, &end);
	return gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
}

double display_result_get_double ()
{
	switch (current_status.number) {
		case CS_DEC:
			return atof(display_result_get());
			break;
		case CS_HEX:
			return axtof(display_result_get(), 16, prefs.hex_bits, prefs.hex_signed);
			break;
		case CS_OCT:
			return axtof(display_result_get(), 8, prefs.oct_bits, prefs.oct_signed);
			break;
		case CS_BIN:
			return axtof(display_result_get(), 2, prefs.bin_bits, prefs.bin_signed);
			break;
		default:
			fprintf (stderr, _("[%s] unknown number base in function \"display_result_get_double\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return 0;
}	


/*
 * display_result_get_angle - this function is used e.g. by trigonometric functions.
 *	pays attention to rad/deg/grad
 */

double display_result_get_rad_angle ()
{
	double		value;
	
	value = display_result_get_double();
	switch (current_status.angle)
	{
	case CS_DEG:
		return deg2rad(value); 
	case CS_RAD:
		return value;
	case CS_GRAD:
		return grad2rad (value);
	default:
		fprintf (stderr, _("[%s] unknown angle base in function \"display_result_get_rad_angle\". %s\n"), PROG_NAME, BUG_REPORT);
	}
	return -1;
}

void display_append_e ()
{
	GtkTextIter		end;
	
	if (current_status.number != CS_DEC) return;
	if (calc_entry_start_new == FALSE) {
		if (strstr (display_result_get(), "e+") == NULL) {
			display_get_line_end_iter (buffer, DISPLAY_RESULT_LINE, &end);
			gtk_text_buffer_insert_with_tags_by_name (buffer, &end, "e+", -1, "result", NULL);
		}
	} else {
		display_result_set ("0e+");
		calc_entry_start_new = FALSE;
	}
	display_result_counter = display_lengths[current_status.number] - DISPLAY_RESULT_E_LENGTH;
}

void display_result_toggle_sign ()
{
	GtkTextIter		start, end;
	char			*result_field, *e_pointer;
	
	if (current_status.number != CS_DEC) return;
	/* we could call display_result_get but we need start iterator later on anyway */
	display_get_line_iters (buffer, DISPLAY_RESULT_LINE, &start, &end);
	result_field = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	/* if there is no e? we toggle the leading sign, otherwise the sign after e */
	if ((e_pointer = strchr (result_field, 'e')) == NULL) {
		if (*result_field == '-') {
			gtk_text_buffer_get_iter_at_offset (buffer, &end, gtk_text_iter_get_offset (&start) + 1);
			gtk_text_buffer_delete (buffer, &start, &end);
		} else {
			if (strcmp (result_field, "0") != 0) \
				gtk_text_buffer_insert_with_tags_by_name (buffer, &start, "-", \
					-1, "result", NULL);
		}
	} else {
		if (*(++e_pointer) == '-') *e_pointer = '+';
		else *e_pointer = '-';
		display_result_set (result_field);
	}
	g_free (result_field);
}

/* display_result_backspace - deletes the tail of the display.
 *		sets the display with display_result_set => no additional manipulation of 
 *		display_result_counter
 *		necessary
 */

void display_result_backspace ()
{															
	char	*current_entry;
	
	if (calc_entry_start_new == TRUE) {
		calc_entry_start_new = FALSE;
		display_result_set ("0");
	} else {
		current_entry = display_result_get();
		/* to avoid an empty/senseless result field */
		if (strlen(current_entry) == 1) current_entry[0] = '0';
		else if ((strlen(current_entry) == 2) && (*current_entry == '-')) current_entry = "0\0";
		else if (current_entry[strlen(current_entry) - 2] == 'e') current_entry[strlen(current_entry) - 2] = '\0';
		else current_entry[strlen(current_entry) - 1] = '\0';
		display_result_set (current_entry);
		g_free (current_entry);
	}
}
