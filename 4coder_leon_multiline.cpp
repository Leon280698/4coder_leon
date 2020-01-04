

CUSTOM_COMMAND_SIG(leon_add_cursor_up){
	auto view = get_active_view(app, Access_ReadWriteVisible);
	auto pos = view_get_cursor_pos(app, view);

	move_vertical_lines(app, view, -1);

	auto newPos = view_get_cursor_pos(app, view);

	if(newPos != pos)
		print_message(app, string_u8_litexpr("ADD CURSOR UP\n"));
}

CUSTOM_COMMAND_SIG(leon_add_cursor_down){
	auto view = get_active_view(app, Access_ReadWriteVisible);
	auto pos = view_get_cursor_pos(app, view);

	move_vertical_lines(app, view, 1);

	auto newPos = view_get_cursor_pos(app, view);

	if(newPos != pos)
		print_message(app, string_u8_litexpr("ADD CURSOR DOWN\n"));
}
