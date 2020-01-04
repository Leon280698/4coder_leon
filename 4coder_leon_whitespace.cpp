function void
leon_free_batch_edits(Batch_Edit* e){
	if(e->next)
		leon_free_batch_edits(e->next);

	delete e;
}

function void
leon_trim_whitespace_impl(Application_Links* app, Buffer_ID buffer){
	auto bufferSize = buffer_get_size(app, buffer);

	if(bufferSize > 1){
		Batch_Edit* edits = nullptr;
		auto tmp = &edits;
		auto tempBuffer = new u8[bufferSize];

		if(buffer_read_range(app, buffer, {0, bufferSize}, tempBuffer)){
			auto textEnd = bufferSize;

			while(textEnd > 0 && std::isspace(tempBuffer[textEnd - 1]))
				--textEnd;

			// Removing trailing whitespace

			for(auto i = 0; i < textEnd; ++i){
				if(std::isspace(tempBuffer[i])){
					auto j = i;

					while(j < textEnd && std::isspace(tempBuffer[j]) && tempBuffer[j] != '\n')
						++j;

					if(j > i && std::isspace(tempBuffer[j])){
						*tmp = new Batch_Edit;
						(*tmp)->next = nullptr;
						(*tmp)->edit.text = string_u8_empty;
						(*tmp)->edit.range = {i, j};
						tmp = &(*tmp)->next;
					}

					i = j;
				}
			}

			// Inserting single final newline

			*tmp = new Batch_Edit;
			(*tmp)->next = nullptr;
			(*tmp)->edit.text = string_u8_litexpr("\n");
			(*tmp)->edit.range = {textEnd, bufferSize};
			tmp = &(*tmp)->next;
		}

		if(edits){
			buffer_batch_edit(app, buffer, edits);
			leon_free_batch_edits(edits);
		}

		delete[] tempBuffer;
	}
}

CUSTOM_COMMAND_SIG(leon_trim_whitespace)
CUSTOM_DOC("Discards trailing whitespace and inserts single final newline"){
	auto view = get_active_view(app, Access_ReadVisible);
	auto buffer = view_get_buffer(app, view, Access_ReadVisible);

	leon_trim_whitespace_impl(app, buffer);
}
