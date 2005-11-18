/*
 *  callbacks.h
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

#include <gtk/gtk.h>

/* FILE */

void
on_number_button_clicked               (GtkToggleButton	*button,
                                        gpointer         user_data);
void
on_operation_button_clicked            (GtkToggleButton	*button,
                                        gpointer         user_data);
void
on_function_button_clicked             (GtkToggleButton	*button,
                                        gpointer         user_data);
void
on_constant_button_clicked             (GtkToggleButton	*button,
                                        gpointer         user_data);
void
on_tbutton_fmod_clicked                (GtkButton       *button,
					gpointer	 user_data);
void
on_gfunc_button_clicked                (GtkToggleButton	*button,
                                        gpointer         user_data);
void
on_about_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data);
void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_dec_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_hex_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_oct_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_bin_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_deg_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_rad_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_grad_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_ordinary_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_rpn_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);					
void 
on_form_activate 			(GtkMenuItem     *menuitem,
					gpointer         user_data);
void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_prefs_result_font_clicked           (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_result_color_clicked          (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_mod_font_clicked              (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_act_mod_color_clicked         (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_inact_mod_color_clicked       (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_bkg_color_clicked             (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_button_font_clicked           (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_close_clicked                (GtkButton       *button,
                                        gpointer         user_data);
void
on_color_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data);
void
on_color_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);
void
on_font_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data);
void
on_font_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);
void
on_prefs_cancel_clicked                (GtkButton       *button,
                                        gpointer         user_data);
void
on_color_cancel_button_clicked         (GtkButton       *button,
                                        gpointer         user_data);
void
on_font_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);
void
on_show_menubar1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_prefs_custom_button_font_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
						
gboolean on_button_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

void on_prefs_vis_number_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);
					
void on_prefs_vis_angle_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_vis_notation_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_vis_arith_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_vis_bracket_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);
				
void on_prefs_show_menu_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_rem_display_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);
					
void on_prefs_button_width_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data);

void on_prefs_button_height_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data);

void on_prefs_hex_bits_value_changed (GtkSpinButton *spinbutton, 
					GtkScrollType arg1, 
					gpointer user_data);
					
void on_prefs_hex_signed_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);
					
void on_prefs_oct_bits_value_changed (GtkSpinButton *spinbutton, 
					GtkScrollType arg1, 
					gpointer user_data);
					
void on_prefs_oct_signed_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);
					
void on_prefs_bin_bits_value_changed (GtkSpinButton *spinbutton, 
					GtkScrollType arg1, 
					gpointer user_data);
					
void on_prefs_bin_signed_toggled (GtkToggleButton *togglebutton, 
				gpointer user_data);

void on_prefs_dec_sep_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_hex_sep_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_oct_sep_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_bin_sep_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_bin_sep_toggled (GtkToggleButton *togglebutton, 
					gpointer user_data);

void on_prefs_dec_sep_length_value_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data);
					
void on_prefs_hex_sep_length_value_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data);

void on_prefs_oct_sep_length_value_changed (GtkSpinButton *spinbutton,
					GtkScrollType arg1,
					gpointer user_data);

void on_prefs_dec_sep_char_changed (GtkEditable *editable,
                                            gpointer user_data);

void on_prefs_hex_sep_char_changed (GtkEditable *editable,
                                            gpointer user_data);
					    
void on_prefs_oct_sep_char_changed (GtkEditable *editable,
                                            gpointer user_data);

void on_prefs_bin_sep_char_changed (GtkEditable *editable,
                                            gpointer user_data);

void on_prefs_menu_dec_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);
			
void on_prefs_menu_hex_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);
			
void on_prefs_menu_oct_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);
			
void on_prefs_menu_bin_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void const_list_selection_changed_cb (GtkTreeSelection *selection, 
				gpointer data);
					
void on_togglebutton_released (GtkToggleButton *togglebutton, 
					gpointer user_data);
					
void on_display_control_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void on_logical_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void on_copy_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void on_basic_mode_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);
			
void on_cut_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);
			
void on_scientific_mode_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void on_paste_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void on_functions_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void on_standard_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);

void on_main_window_check_resize (GtkContainer *container,
			gpointer user_data);

void on_finite_stack_size_clicked (GtkRadioButton *rb,
			gpointer user_data);
			
void on_infinite_stack_size_clicked (GtkRadioButton *rb,
			gpointer user_data);

gboolean on_button_press_event (GtkWidget *widget,
						GdkEventButton *event,
						gpointer user_data);

void on_formula_entry_activate (GtkEntry *entry,
                                            gpointer user_data);
					    
void on_formula_entry_changed (GtkEditable *editable, gpointer user_data);

void
on_user_function_button_clicked (GtkToggleButton       *button,
                                        gpointer         user_data);

void user_function_list_selection_changed_cb (GtkTreeSelection *selection, 
					gpointer data);

void on_ng_mode_activate (GtkMenuItem     *menuitem,
			gpointer         user_data);
