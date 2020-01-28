/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#include <cctype>
#include <4coder_default_include.cpp>
#include <generated/managed_id_metadata.cpp>

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
	User_Input in = get_current_input(app);
	String_Const_u8 insert = to_writable(&in);

	if(insert.str != 0 && insert.size > 0){
		b32 do_auto_indent = false;

		for(u64 i = 0; !do_auto_indent && i < insert.size; ++i){
			switch (insert.str[i]){
				case ';': case ':':
				case '{': case '}':
				case '(': case ')':
				case '[': case ']':
				case '#':
				case '\n': /*case '\t':*/ // Don't auto indent when typing tab since it is really annoying...
					do_auto_indent = true;
					break;
			}
		}

		if(do_auto_indent){
			View_ID view = get_active_view(app, Access_ReadWriteVisible);
			Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
			Range_i64 pos = {};
			pos.min = view_get_cursor_pos(app, view);
			write_text_input(app);
			pos.max= view_get_cursor_pos(app, view);
			auto_indent_buffer(app, buffer, pos, 0);
			move_past_lead_whitespace(app, view, buffer);
		}else{
			write_text_input(app);
		}
	}
}

CUSTOM_COMMAND_SIG(leon_backspace_char){
	backspace_char(app);
}

BUFFER_HOOK_SIG(leon_file_save){
	if(leon_trimWhitespaceOnFileSave)
		leon_trim_whitespace_impl(app, buffer_id);

	return default_file_save(app, buffer_id);
}

#if OS_MAC
void leon_mac_init();
#endif

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
	mapping_init(tctx, &framework_mapping);

	// Setting up default mapping

#ifdef __APPLE__
	#define leon_KeyCode_Control KeyCode_Command
#else
	#define leon_KeyCode_Control KeyCode_Control
#endif

	MappingScope();
	SelectMapping(&framework_mapping);

	SelectMap(mapid_global);
	BindCore(default_startup, CoreCode_Startup);
	BindCore(default_try_exit, CoreCode_TryExit);
	Bind(keyboard_macro_start_recording , KeyCode_U, leon_KeyCode_Control);
	Bind(keyboard_macro_finish_recording, KeyCode_U, leon_KeyCode_Control, KeyCode_Shift);
	Bind(keyboard_macro_replay,           KeyCode_U, KeyCode_Alt);
	Bind(change_active_panel,           KeyCode_Comma, leon_KeyCode_Control);
	Bind(change_active_panel_backwards, KeyCode_Comma, leon_KeyCode_Control, KeyCode_Shift);
	Bind(interactive_new,               KeyCode_N, leon_KeyCode_Control);
	Bind(interactive_open_or_new,       KeyCode_O, leon_KeyCode_Control);
	Bind(open_in_other,                 KeyCode_O, KeyCode_Alt);
	Bind(interactive_kill_buffer,       KeyCode_K, leon_KeyCode_Control);
	Bind(interactive_switch_buffer,     KeyCode_I, leon_KeyCode_Control);
	Bind(project_go_to_root_directory,  KeyCode_H, leon_KeyCode_Control);
	Bind(save_all_dirty_buffers,        KeyCode_S, leon_KeyCode_Control, KeyCode_Shift);
	Bind(change_to_build_panel,         KeyCode_Period, KeyCode_Alt);
	Bind(close_build_panel,             KeyCode_Comma, KeyCode_Alt);
	Bind(goto_next_jump,                KeyCode_N, KeyCode_Alt);
	Bind(goto_prev_jump,                KeyCode_N, KeyCode_Alt, KeyCode_Shift);
	Bind(build_in_build_panel,          KeyCode_B, leon_KeyCode_Control, KeyCode_Shift); // NOTE(Leon): Override
	Bind(goto_first_jump,               KeyCode_M, KeyCode_Alt, KeyCode_Shift);
	Bind(toggle_filebar,                KeyCode_B, KeyCode_Alt);
	Bind(execute_any_cli,               KeyCode_Z, KeyCode_Alt);
	Bind(execute_previous_cli,          KeyCode_Z, KeyCode_Alt, KeyCode_Shift);
	Bind(command_lister,                KeyCode_X, KeyCode_Alt);
	Bind(project_command_lister,        KeyCode_X, KeyCode_Alt, KeyCode_Shift);
	Bind(list_all_functions_current_buffer, KeyCode_I, leon_KeyCode_Control, KeyCode_Shift);
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
	BindMouseWheel(mouse_wheel_change_face_size, leon_KeyCode_Control);

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
	Bind(goto_beginning_of_file, KeyCode_PageUp, leon_KeyCode_Control);
	Bind(goto_end_of_file,       KeyCode_PageDown, leon_KeyCode_Control);
	Bind(move_up_10,             KeyCode_Up, leon_KeyCode_Control); // NOTE(Leon): Override
	Bind(move_down_10,           KeyCode_Down, leon_KeyCode_Control); // NOTE(Leon): Override
	Bind(move_up_to_blank_line_end,        KeyCode_Up, leon_KeyCode_Control, KeyCode_Shift); // NOTE(Leon): Custom Function
	Bind(move_down_to_blank_line_end,      KeyCode_Down, leon_KeyCode_Control, KeyCode_Shift); // NOTE(Leon): Custom Function
	Bind(move_left_whitespace_boundary,    KeyCode_Left, leon_KeyCode_Control);
	Bind(move_right_whitespace_boundary,   KeyCode_Right, leon_KeyCode_Control);
	Bind(move_line_up,                     KeyCode_Up, KeyCode_Alt);
	Bind(move_line_down,                   KeyCode_Down, KeyCode_Alt);
	Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, leon_KeyCode_Control);
	Bind(delete_alpha_numeric_boundary,    KeyCode_Delete, leon_KeyCode_Control);
	Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Alt);
	Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, KeyCode_Alt);
	Bind(set_mark,                    KeyCode_Space, KeyCode_Control); // NOTE(Leon): Not using leon_KeyCode_Control because this should be the same on macOS
	Bind(replace_in_range,            KeyCode_A, leon_KeyCode_Control);
	Bind(copy,                        KeyCode_C, leon_KeyCode_Control);
	Bind(delete_range,                KeyCode_D, leon_KeyCode_Control);
	Bind(delete_line,                 KeyCode_D, leon_KeyCode_Control, KeyCode_Shift);
	Bind(center_view,                 KeyCode_E, leon_KeyCode_Control);
	Bind(left_adjust_view,            KeyCode_E, leon_KeyCode_Control, KeyCode_Shift);
	Bind(search,                      KeyCode_F, leon_KeyCode_Control);
	Bind(list_all_locations,          KeyCode_F, leon_KeyCode_Control, KeyCode_Shift);
	Bind(list_all_substring_locations_case_insensitive, KeyCode_F, KeyCode_Alt);
	Bind(goto_line,                   KeyCode_G, leon_KeyCode_Control);
	Bind(list_all_locations_of_selection,  KeyCode_G, leon_KeyCode_Control, KeyCode_Shift);
	Bind(snippet_lister,              KeyCode_J, leon_KeyCode_Control);
	Bind(kill_buffer,                 KeyCode_K, leon_KeyCode_Control, KeyCode_Shift);
	Bind(duplicate_line,              KeyCode_L, leon_KeyCode_Control);
	Bind(cursor_mark_swap,            KeyCode_M, leon_KeyCode_Control);
	Bind(reopen,                      KeyCode_O, leon_KeyCode_Control, KeyCode_Shift);
	Bind(query_replace_identifier,    KeyCode_Q, leon_KeyCode_Control, KeyCode_Shift);
	Bind(query_replace_selection,     KeyCode_Q, KeyCode_Alt);
	Bind(reverse_search,              KeyCode_R, leon_KeyCode_Control);
	Bind(save,                        KeyCode_S, leon_KeyCode_Control);
	Bind(save_all_dirty_buffers,      KeyCode_S, leon_KeyCode_Control, KeyCode_Shift);
	Bind(search_identifier,           KeyCode_T, leon_KeyCode_Control);
	Bind(list_all_locations_of_identifier, KeyCode_T, leon_KeyCode_Control, KeyCode_Shift);
	Bind(paste_and_indent,            KeyCode_V, leon_KeyCode_Control);
	Bind(paste_next_and_indent,       KeyCode_V, leon_KeyCode_Control, KeyCode_Shift);
	Bind(cut,                         KeyCode_X, leon_KeyCode_Control);
	Bind(redo,                        KeyCode_Y, leon_KeyCode_Control);
	Bind(undo,                        KeyCode_Z, leon_KeyCode_Control);
	Bind(view_buffer_other_panel,     KeyCode_1, leon_KeyCode_Control);
	Bind(swap_panels,                 KeyCode_2, leon_KeyCode_Control);
	Bind(if_read_only_goto_position,  KeyCode_Return);
	Bind(if_read_only_goto_position_same_panel, KeyCode_Return, KeyCode_Shift);
	Bind(view_jump_list_with_lister,  KeyCode_Period, leon_KeyCode_Control, KeyCode_Shift);

	SelectMap(mapid_code);
	ParentMap(mapid_file);
	BindTextInput(leon_write_text_and_auto_indent); // NOTE(Leon): Override
	Bind(move_left_alpha_numeric_boundary,           KeyCode_Left, leon_KeyCode_Control);
	Bind(move_right_alpha_numeric_boundary,          KeyCode_Right, leon_KeyCode_Control);
	Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_Left, KeyCode_Alt);
	Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_Right, KeyCode_Alt);
	Bind(comment_line_toggle,        KeyCode_Semicolon, leon_KeyCode_Control);
	Bind(word_complete,  	        KeyCode_Tab, leon_KeyCode_Control); // NOTE(Leon): Override
	Bind(auto_indent_line_at_cursor, KeyCode_Tab, KeyCode_Shift);
	Bind(word_complete_drop_down,    KeyCode_Tab, KeyCode_Shift, leon_KeyCode_Control);
	Bind(write_block,                KeyCode_R, KeyCode_Alt);
	Bind(write_todo,                 KeyCode_T, KeyCode_Alt);
	Bind(write_note,                 KeyCode_Y, KeyCode_Alt);
	Bind(list_all_locations_of_type_definition,               KeyCode_D, KeyCode_Alt);
	Bind(list_all_locations_of_type_definition_of_identifier, KeyCode_T, KeyCode_Alt, KeyCode_Shift);
	Bind(open_long_braces,           KeyCode_LeftBracket, leon_KeyCode_Control);
	Bind(open_long_braces_semicolon, KeyCode_LeftBracket, leon_KeyCode_Control, KeyCode_Shift);
	Bind(open_long_braces_break,     KeyCode_RightBracket, leon_KeyCode_Control, KeyCode_Shift);
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

#if OS_MAC
	leon_mac_init();
#endif
}

// BOTTOM
