/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#include <cctype>
#include "4coder_default_include.cpp"
#include "4coder_default_map.cpp"

#include "4coder_leon_whitespace.cpp"

global bool leon_trimWhitespaceOnFileSave = true;

CUSTOM_COMMAND_SIG(leon_enable_auto_trim_whitespace)
CUSTOM_DOC("Enables trimming of trailing whitespace on file save."){
	leon_trimWhitespaceOnFileSave = true;
}

CUSTOM_COMMAND_SIG(leon_disable_auto_trim_whitespace)
CUSTOM_DOC("Disables trimming of trailing whitespace on file save."){
	leon_trimWhitespaceOnFileSave = false;
}

CUSTOM_COMMAND_SIG(leon_write_text_input){
	write_text_input(app);
}

CUSTOM_COMMAND_SIG(leon_write_text_and_auto_indent){
	write_text_and_auto_indent(app);
}

CUSTOM_COMMAND_SIG(leon_backspace_char){
	backspace_char(app);
}

BUFFER_HOOK_SIG(leon_file_save){
	if(leon_trimWhitespaceOnFileSave)
		leon_trim_whitespace_impl(app, buffer_id);

	return default_file_save(app, buffer_id);
}

function void
leon_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
	ProfileScope(app, "leon render caller");
	View_ID active_view = get_active_view(app, Access_Always);
	b32 is_active_view = (active_view == view_id);

	Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
	Rect_f32 prev_clip = draw_set_clip(app, region);

	Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
	Face_ID face_id = get_face_id(app, buffer);
	Face_Metrics face_metrics = get_face_metrics(app, face_id);
	f32 line_height = face_metrics.line_height;
	f32 digit_advance = face_metrics.decimal_digit_advance;

	// NOTE(allen): file bar
	b64 showing_file_bar = false;
	if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
		Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
		draw_file_bar(app, view_id, buffer, face_id, pair.min);
		region = pair.max;
	}

	Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);

	Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
												  frame_info.animation_dt, scroll);
	if (!block_match_struct(&scroll.position, &delta.point)){
		block_copy_struct(&scroll.position, &delta.point);
		view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
	}
	if (delta.still_animating){
		animate_in_n_milliseconds(app, 0);
	}

	// NOTE(allen): query bars
	{
		Query_Bar *space[32];
		Query_Bar_Ptr_Array query_bars = {};
		query_bars.ptrs = space;
		if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
			for (i32 i = 0; i < query_bars.count; i += 1){
				Rect_f32_Pair pair = layout_query_bar_on_top(region, line_height, 1);
				draw_query_bar(app, query_bars.ptrs[i], face_id, pair.min);
				region = pair.max;
			}
		}
	}

	// NOTE(allen): FPS hud
	if (show_fps_hud){
		Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
		draw_fps_hud(app, frame_info, face_id, pair.max);
		region = pair.min;
		animate_in_n_milliseconds(app, 1000);
	}

	// NOTE(allen): layout line numbers
	Rect_f32 line_number_rect = {};
	if (global_config.show_line_number_margins){
		Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
		line_number_rect = pair.min;
		region = pair.max;
	}

	// NOTE(allen): begin buffer render
	Buffer_Point buffer_point = scroll.position;
	Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);

	// NOTE(allen): draw line numbers
	if (global_config.show_line_number_margins){
		draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
	}

	// NOTE(allen): draw the buffer
	default_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);

	// NOTE(Leon): Begin custom rendering
	// NOTE(Leon): End custom rendering

	text_layout_free(app, text_layout_id);
	draw_set_clip(app, prev_clip);
}

void
custom_layer_init(Application_Links *app){
	Thread_Context* tctx = get_thread_context(app);

	// NOTE(allen): setup for default framework
	async_task_handler_init(app, &global_async_system);
	code_index_init();
	buffer_modified_set_init();
	Profile_Global_List* list = get_core_profile_list(app);
	ProfileThreadName(tctx, list, string_u8_litexpr("main"));
	initialize_managed_id_metadata(app);
	set_default_color_scheme(app);

	// NOTE(allen): default hooks and command maps
	set_all_default_hooks(app);
	set_custom_hook(app, HookID_SaveFile, leon_file_save);
	set_custom_hook(app, HookID_RenderCaller, leon_render_caller);
	mapping_init(tctx, &framework_mapping);

	// Setting up default mapping

	MappingScope();
	SelectMapping(&framework_mapping);

	SelectMap(mapid_global);
	BindCore(default_startup, CoreCode_Startup);
	BindCore(default_try_exit, CoreCode_TryExit);
	Bind(keyboard_macro_start_recording , KeyCode_U, KeyCode_Control);
	Bind(keyboard_macro_finish_recording, KeyCode_U, KeyCode_Control, KeyCode_Shift);
	Bind(keyboard_macro_replay,           KeyCode_U, KeyCode_Alt);
	Bind(change_active_panel,           KeyCode_Comma, KeyCode_Control);
	Bind(change_active_panel_backwards, KeyCode_Comma, KeyCode_Control, KeyCode_Shift);
	Bind(interactive_new,               KeyCode_N, KeyCode_Control);
	Bind(interactive_open_or_new,       KeyCode_O, KeyCode_Control);
	Bind(open_in_other,                 KeyCode_O, KeyCode_Alt);
	Bind(interactive_kill_buffer,       KeyCode_K, KeyCode_Control);
	Bind(interactive_switch_buffer,     KeyCode_I, KeyCode_Control);
	Bind(project_go_to_root_directory,  KeyCode_H, KeyCode_Control);
	Bind(save_all_dirty_buffers,        KeyCode_S, KeyCode_Control, KeyCode_Shift);
	Bind(change_to_build_panel,         KeyCode_Period, KeyCode_Alt);
	Bind(close_build_panel,             KeyCode_Comma, KeyCode_Alt);
	Bind(goto_next_jump,                KeyCode_N, KeyCode_Alt);
	Bind(goto_prev_jump,                KeyCode_N, KeyCode_Alt, KeyCode_Shift);
	Bind(build_in_build_panel,          KeyCode_B, KeyCode_Control, KeyCode_Shift); // NOTE(Leon): Override
	Bind(goto_first_jump,               KeyCode_M, KeyCode_Alt, KeyCode_Shift);
	Bind(toggle_filebar,                KeyCode_B, KeyCode_Alt);
	Bind(execute_any_cli,               KeyCode_Z, KeyCode_Alt);
	Bind(execute_previous_cli,          KeyCode_Z, KeyCode_Alt, KeyCode_Shift);
	Bind(command_lister,                KeyCode_X, KeyCode_Alt);
	Bind(project_command_lister,        KeyCode_X, KeyCode_Alt, KeyCode_Shift);
	Bind(list_all_functions_current_buffer, KeyCode_I, KeyCode_Control, KeyCode_Shift);
	Bind(project_fkey_command, KeyCode_F1);
	Bind(project_fkey_command, KeyCode_F2);
	Bind(project_fkey_command, KeyCode_F3);
	Bind(project_fkey_command, KeyCode_F4);
	Bind(project_fkey_command, KeyCode_F5);
	Bind(project_fkey_command, KeyCode_F6);
	Bind(project_fkey_command, KeyCode_F7);
	Bind(project_fkey_command, KeyCode_F8);
	Bind(project_fkey_command, KeyCode_F9);
	Bind(project_fkey_command, KeyCode_F10);
	Bind(project_fkey_command, KeyCode_F11);
	Bind(project_fkey_command, KeyCode_F12);
	Bind(project_fkey_command, KeyCode_F13);
	Bind(project_fkey_command, KeyCode_F14);
	Bind(project_fkey_command, KeyCode_F15);
	Bind(project_fkey_command, KeyCode_F16);
	Bind(exit_4coder,          KeyCode_F4, KeyCode_Alt);
	BindMouseWheel(mouse_wheel_scroll);
	BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);

	SelectMap(mapid_file);
	ParentMap(mapid_global);
	BindTextInput(leon_write_text_input); // NOTE(Leon): Override
	BindMouse(click_set_cursor_and_mark, MouseCode_Left);
	BindMouseRelease(click_set_cursor, MouseCode_Left);
	BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
	BindMouseMove(click_set_cursor_if_lbutton);
	Bind(delete_char,            KeyCode_Delete);
	Bind(leon_backspace_char,    KeyCode_Backspace); // NOTE(Leon): Override
	Bind(move_up,                KeyCode_Up);
	Bind(move_down,              KeyCode_Down);
	Bind(move_left,              KeyCode_Left);
	Bind(move_right,             KeyCode_Right);
	Bind(seek_end_of_line,       KeyCode_End);
	Bind(seek_beginning_of_line, KeyCode_Home);
	Bind(page_up,                KeyCode_PageUp);
	Bind(page_down,              KeyCode_PageDown);
	Bind(goto_beginning_of_file, KeyCode_PageUp, KeyCode_Control);
	Bind(goto_end_of_file,       KeyCode_PageDown, KeyCode_Control);
	Bind(move_up_10,             KeyCode_Up, KeyCode_Control); // NOTE(Leon): Override
	Bind(move_down_10,           KeyCode_Down, KeyCode_Control); // NOTE(Leon): Override
	Bind(move_up_to_blank_line_end,        KeyCode_Up, KeyCode_Control, KeyCode_Shift); // NOTE(Leon): Custom Function
	Bind(move_down_to_blank_line_end,      KeyCode_Down, KeyCode_Control, KeyCode_Shift); // NOTE(Leon): Custom Function
	Bind(move_left_whitespace_boundary,    KeyCode_Left, KeyCode_Control);
	Bind(move_right_whitespace_boundary,   KeyCode_Right, KeyCode_Control);
	Bind(move_line_up,                     KeyCode_Up, KeyCode_Alt);
	Bind(move_line_down,                   KeyCode_Down, KeyCode_Alt);
	Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, KeyCode_Control);
	Bind(delete_alpha_numeric_boundary,    KeyCode_Delete, KeyCode_Control);
	Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Alt);
	Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, KeyCode_Alt);
	Bind(set_mark,                    KeyCode_Space, KeyCode_Control);
	Bind(replace_in_range,            KeyCode_A, KeyCode_Control);
	Bind(copy,                        KeyCode_C, KeyCode_Control);
	Bind(delete_range,                KeyCode_D, KeyCode_Control);
	Bind(delete_line,                 KeyCode_D, KeyCode_Control, KeyCode_Shift);
	Bind(center_view,                 KeyCode_E, KeyCode_Control);
	Bind(left_adjust_view,            KeyCode_E, KeyCode_Control, KeyCode_Shift);
	Bind(search,                      KeyCode_F, KeyCode_Control);
	Bind(list_all_locations,          KeyCode_F, KeyCode_Control, KeyCode_Shift);
	Bind(list_all_substring_locations_case_insensitive, KeyCode_F, KeyCode_Alt);
	Bind(goto_line,                   KeyCode_G, KeyCode_Control);
	Bind(list_all_locations_of_selection,  KeyCode_G, KeyCode_Control, KeyCode_Shift);
	Bind(snippet_lister,              KeyCode_J, KeyCode_Control);
	Bind(kill_buffer,                 KeyCode_K, KeyCode_Control, KeyCode_Shift);
	Bind(duplicate_line,              KeyCode_L, KeyCode_Control);
	Bind(cursor_mark_swap,            KeyCode_M, KeyCode_Control);
	Bind(reopen,                      KeyCode_O, KeyCode_Control, KeyCode_Shift);
	Bind(query_replace,               KeyCode_Q, KeyCode_Control);
	Bind(query_replace_identifier,    KeyCode_Q, KeyCode_Control, KeyCode_Shift);
	Bind(query_replace_selection,     KeyCode_Q, KeyCode_Alt);
	Bind(reverse_search,              KeyCode_R, KeyCode_Control);
	Bind(save,                        KeyCode_S, KeyCode_Control);
	Bind(save_all_dirty_buffers,      KeyCode_S, KeyCode_Control, KeyCode_Shift);
	Bind(search_identifier,           KeyCode_T, KeyCode_Control);
	Bind(list_all_locations_of_identifier, KeyCode_T, KeyCode_Control, KeyCode_Shift);
	Bind(paste_and_indent,            KeyCode_V, KeyCode_Control);
	Bind(paste_next_and_indent,       KeyCode_V, KeyCode_Control, KeyCode_Shift);
	Bind(cut,                         KeyCode_X, KeyCode_Control);
	Bind(redo,                        KeyCode_Y, KeyCode_Control);
	Bind(undo,                        KeyCode_Z, KeyCode_Control);
	Bind(view_buffer_other_panel,     KeyCode_1, KeyCode_Control);
	Bind(swap_panels,                 KeyCode_2, KeyCode_Control);
	Bind(if_read_only_goto_position,  KeyCode_Return);
	Bind(if_read_only_goto_position_same_panel, KeyCode_Return, KeyCode_Shift);
	Bind(view_jump_list_with_lister,  KeyCode_Period, KeyCode_Control, KeyCode_Shift);

	SelectMap(mapid_code);
	ParentMap(mapid_file);
	BindTextInput(leon_write_text_and_auto_indent); // NOTE(Leon): Override
	Bind(move_left_alpha_numeric_boundary,           KeyCode_Left, KeyCode_Control);
	Bind(move_right_alpha_numeric_boundary,          KeyCode_Right, KeyCode_Control);
	Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_Left, KeyCode_Alt);
	Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_Right, KeyCode_Alt);
	Bind(comment_line_toggle,        KeyCode_Semicolon, KeyCode_Control);
	Bind(word_complete,              KeyCode_Tab);
	Bind(auto_indent_range,          KeyCode_Tab, KeyCode_Control);
	Bind(auto_indent_line_at_cursor, KeyCode_Tab, KeyCode_Shift);
	Bind(word_complete_drop_down,    KeyCode_Tab, KeyCode_Shift, KeyCode_Control);
	Bind(write_block,                KeyCode_R, KeyCode_Alt);
	Bind(write_todo,                 KeyCode_T, KeyCode_Alt);
	Bind(write_note,                 KeyCode_Y, KeyCode_Alt);
	Bind(list_all_locations_of_type_definition,               KeyCode_D, KeyCode_Alt);
	Bind(list_all_locations_of_type_definition_of_identifier, KeyCode_T, KeyCode_Alt, KeyCode_Shift);
	Bind(open_long_braces,           KeyCode_LeftBracket, KeyCode_Control);
	Bind(open_long_braces_semicolon, KeyCode_LeftBracket, KeyCode_Control, KeyCode_Shift);
	Bind(open_long_braces_break,     KeyCode_RightBracket, KeyCode_Control, KeyCode_Shift);
	Bind(select_surrounding_scope,   KeyCode_LeftBracket, KeyCode_Alt);
	Bind(select_surrounding_scope_maximal, KeyCode_LeftBracket, KeyCode_Alt, KeyCode_Shift);
	Bind(select_prev_scope_absolute, KeyCode_RightBracket, KeyCode_Alt);
	Bind(select_prev_top_most_scope, KeyCode_RightBracket, KeyCode_Alt, KeyCode_Shift);
	Bind(select_next_scope_absolute, KeyCode_Quote, KeyCode_Alt);
	Bind(select_next_scope_after_current, KeyCode_Quote, KeyCode_Alt, KeyCode_Shift);
	Bind(place_in_scope,             KeyCode_ForwardSlash, KeyCode_Alt);
	Bind(delete_current_scope,       KeyCode_Minus, KeyCode_Alt);
	Bind(if0_off,                    KeyCode_I, KeyCode_Alt);
	Bind(open_file_in_quotes,        KeyCode_1, KeyCode_Alt);
	Bind(open_matching_file_cpp,     KeyCode_2, KeyCode_Alt);
	Bind(write_zero_struct,          KeyCode_0, KeyCode_Control);
}

// BOTTOM
