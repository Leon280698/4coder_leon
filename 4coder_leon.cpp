/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#include <cctype>
#include <4coder_default_include.cpp>
#include <generated/managed_id_metadata.cpp>

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
	// Cleaning trailing whitespace on save.
	// The original clean_all_lines function does not clean lines that only contain whitespace so this does.

	Scratch_Block scratch(app);
	Batch_Edit* batch_first = 0;
	Batch_Edit* batch_last = 0;

	i64 line_count = buffer_get_line_count(app, buffer_id);
	for(i64 line_number = 1; line_number <= line_count; line_number += 1){
		i64 line_start = get_line_side_pos(app, buffer_id, line_number, Side_Min);
		i64 line_end = get_line_side_pos(app, buffer_id, line_number, Side_Max);
		u8 prev = buffer_get_char(app, buffer_id, line_end - 1);
		b32 has_cr_character = false;
		b32 has_tail_whitespace = false;
		if(prev == '\r'){
			has_cr_character = true;
			if(line_end - 2 >= line_start){
				prev = buffer_get_char(app, buffer_id, line_end - 2);
				has_tail_whitespace = character_is_whitespace(prev);
			}
		}else{
			has_tail_whitespace = character_is_whitespace(prev);
		}

		if(has_tail_whitespace){
			String_Const_u8 line = push_buffer_range(app, scratch, buffer_id,
													 Ii64(line_start, line_end));
			if(line.size > 0){
				i64 end_offset = line.size;

				if(has_cr_character)
					end_offset -= 1;

				i64 start_offset = end_offset;

				while(start_offset > 0 && character_is_whitespace(line.str[start_offset - 1]))
					--start_offset;

				if(start_offset != end_offset){
					i64 start = start_offset + line_start;
					i64 end   = end_offset   + line_start;

					Batch_Edit* batch = push_array(scratch, Batch_Edit, 1);
					sll_queue_push(batch_first, batch_last, batch);
					batch->edit.text = SCu8();
					batch->edit.range = Ii64(start, end);
				}
			}
		}
	}

	if(batch_first != 0)
		buffer_batch_edit(app, buffer_id, batch_first);

	return default_file_save(app, buffer_id);
}

function void
leon_setup_default_mapping(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id){
#if OS_MAC
	#define leon_KeyCode_Control KeyCode_Command
#else
	#define leon_KeyCode_Control KeyCode_Control
#endif

	MappingScope();
	SelectMapping(mapping);

	SelectMap(global_id);
	BindCore(default_startup, CoreCode_Startup);
	BindCore(default_try_exit, CoreCode_TryExit);
	BindCore(clipboard_record_clip, CoreCode_NewClipboardContents);
	Bind(keyboard_macro_start_recording , KeyCode_U, leon_KeyCode_Control);
	Bind(keyboard_macro_finish_recording, KeyCode_U, leon_KeyCode_Control, KeyCode_Shift);
	Bind(keyboard_macro_replay,           KeyCode_U, KeyCode_Alt);
	Bind(change_active_panel,           KeyCode_Comma, leon_KeyCode_Control);
	Bind(change_active_panel_backwards, KeyCode_Comma, leon_KeyCode_Control, KeyCode_Shift);
	Bind(interactive_new,               KeyCode_N, leon_KeyCode_Control);
	Bind(interactive_open_or_new,       KeyCode_O, leon_KeyCode_Control);
	Bind(open_in_other,                 KeyCode_O, KeyCode_Alt);
	Bind(interactive_kill_buffer,       KeyCode_K, leon_KeyCode_Control, KeyCode_Shift);
	Bind(interactive_switch_buffer,     KeyCode_I, leon_KeyCode_Control);
	Bind(project_go_to_root_directory,  KeyCode_H, leon_KeyCode_Control);
	Bind(save_all_dirty_buffers,        KeyCode_S, leon_KeyCode_Control, KeyCode_Shift);
	Bind(change_to_build_panel,         KeyCode_Period, KeyCode_Alt);
	Bind(close_build_panel,             KeyCode_Comma, KeyCode_Alt);
	Bind(goto_next_jump,                KeyCode_N, KeyCode_Alt);
	Bind(goto_prev_jump,                KeyCode_N, KeyCode_Alt, KeyCode_Shift);
	Bind(build_in_build_panel,          KeyCode_B, KeyCode_Shift, leon_KeyCode_Control);
	Bind(goto_first_jump,               KeyCode_M, KeyCode_Alt, KeyCode_Shift);
	Bind(toggle_filebar,                KeyCode_B, KeyCode_Alt);
	Bind(execute_any_cli,               KeyCode_Z, KeyCode_Alt);
	Bind(execute_previous_cli,          KeyCode_Z, KeyCode_Alt, KeyCode_Shift);
	Bind(command_lister,                KeyCode_X, KeyCode_Alt);
	Bind(project_command_lister,        KeyCode_X, KeyCode_Alt, KeyCode_Shift);
	Bind(list_all_functions_current_buffer_lister, KeyCode_I, leon_KeyCode_Control, KeyCode_Shift);
#if OS_WINDOWS
	Bind(exit_4coder,          KeyCode_F4, KeyCode_Alt);
#endif
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
	BindMouseWheel(mouse_wheel_scroll);
	BindMouseWheel(mouse_wheel_change_face_size, leon_KeyCode_Control);

	SelectMap(file_id);
	ParentMap(global_id);
	BindTextInput(write_text_input);
	BindMouse(click_set_cursor_and_mark, MouseCode_Left);
	BindMouseRelease(click_set_cursor, MouseCode_Left);
	BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
	BindMouseMove(click_set_cursor_if_lbutton);
	Bind(delete_char,            KeyCode_Delete);
	Bind(backspace_char,         KeyCode_Backspace);
	Bind(move_up,                KeyCode_Up);
	Bind(move_down,              KeyCode_Down);
	Bind(move_left,              KeyCode_Left);
	Bind(move_right,             KeyCode_Right);
	Bind(seek_end_of_line,       KeyCode_End);
	Bind(seek_beginning_of_line, KeyCode_Home);
#if OS_MAC
	Bind(seek_end_of_line,       KeyCode_Right, KeyCode_Command);
	Bind(seek_beginning_of_line, KeyCode_Left, KeyCode_Command);
	Bind(goto_beginning_of_file, KeyCode_Up, KeyCode_Command);
	Bind(goto_end_of_file,       KeyCode_Down, KeyCode_Command);
#endif
	Bind(page_up,                KeyCode_PageUp);
	Bind(page_down,              KeyCode_PageDown);
	Bind(goto_beginning_of_file, KeyCode_PageUp, leon_KeyCode_Control);
	Bind(goto_end_of_file,       KeyCode_PageDown, leon_KeyCode_Control);
	Bind(move_up_10,             KeyCode_Up, leon_KeyCode_Control);
	Bind(move_down_10,           KeyCode_Down, leon_KeyCode_Control);
	Bind(move_up_to_blank_line,            KeyCode_Up, KeyCode_Shift, leon_KeyCode_Control);
	Bind(move_down_to_blank_line,          KeyCode_Down, KeyCode_Shift, leon_KeyCode_Control);
	Bind(move_left_whitespace_boundary,    KeyCode_Left, KeyCode_Control);
	Bind(move_right_whitespace_boundary,   KeyCode_Right, KeyCode_Control);
	Bind(move_line_up,                     KeyCode_Up, KeyCode_Alt);
	Bind(move_line_down,                   KeyCode_Down, KeyCode_Alt);
	Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, KeyCode_Control);
	Bind(delete_alpha_numeric_boundary,    KeyCode_Delete, KeyCode_Control);
	Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Alt);
	Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, KeyCode_Alt);
	Bind(set_mark,                    KeyCode_Space, KeyCode_Control);
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
	Bind(kill_buffer,                 KeyCode_K, leon_KeyCode_Control);
	Bind(duplicate_line,              KeyCode_L, leon_KeyCode_Control);
	Bind(cursor_mark_swap,            KeyCode_M, leon_KeyCode_Control);
	Bind(reopen,                      KeyCode_O, leon_KeyCode_Control, KeyCode_Shift);
	Bind(query_replace,               KeyCode_Q, leon_KeyCode_Control);
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
	Bind(redo,                        KeyCode_Z, leon_KeyCode_Control, KeyCode_Shift);
	Bind(undo,                        KeyCode_Z, leon_KeyCode_Control);
	Bind(view_buffer_other_panel,     KeyCode_1, leon_KeyCode_Control);
	Bind(swap_panels,                 KeyCode_2, leon_KeyCode_Control);
	Bind(if_read_only_goto_position,  KeyCode_Return);
	Bind(if_read_only_goto_position_same_panel, KeyCode_Return, KeyCode_Shift);
	Bind(view_jump_list_with_lister,  KeyCode_Period, leon_KeyCode_Control, KeyCode_Shift);
	SelectMap(code_id);
	ParentMap(file_id);
	BindTextInput(leon_write_text_and_auto_indent);
	Bind(move_left_alpha_numeric_boundary,           KeyCode_Left, KeyCode_Control);
	Bind(move_right_alpha_numeric_boundary,          KeyCode_Right, KeyCode_Control);
	Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_Left, KeyCode_Alt);
	Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_Right, KeyCode_Alt);
	Bind(comment_line_toggle,        KeyCode_Semicolon, leon_KeyCode_Control);
	Bind(auto_indent_range,          KeyCode_Tab, KeyCode_Shift, KeyCode_Control);
	Bind(auto_indent_line_at_cursor, KeyCode_Tab, KeyCode_Shift);
	Bind(word_complete,              KeyCode_Tab, KeyCode_Control);
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
	Bind(write_zero_struct,          KeyCode_0, leon_KeyCode_Control);
}

#if OS_MAC
void leon_mac_init();
#endif

void
custom_layer_init(Application_Links* app){
	Thread_Context* tctx = get_thread_context(app);

	// NOTE(allen): setup for default framework
	default_framework_init(app);

	// NOTE(allen): default hooks and command maps
	set_all_default_hooks(app);
	set_custom_hook(app, HookID_SaveFile, leon_file_save);
	mapping_init(tctx, &framework_mapping);
	leon_setup_default_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);

#if OS_MAC
	leon_mac_init();
#endif
}

// BOTTOM
