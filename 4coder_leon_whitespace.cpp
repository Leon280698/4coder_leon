internal void
leon_trim_whitespace_impl(Application_Links* app, Buffer_ID buffer){
	auto bufferSize = buffer_get_size(app, buffer);

	if(bufferSize > 1){
		auto edits = new Batch_Edit[buffer_get_line_count(app, buffer)]; // Maximum: one edit per line
		auto numEdits = 0;
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
						edits[numEdits].next = nullptr;
						edits[numEdits].edit.text = string_u8_empty;
						edits[numEdits].edit.range = {i, j};
						++numEdits;
					}

					i = j;
				}
			}

			// Inserting single final newline

			if(textEnd < bufferSize - 1 || !std::isspace(tempBuffer[textEnd])){
				edits[numEdits].next = nullptr;
				edits[numEdits].edit.text = string_u8_litexpr("\n");
				edits[numEdits].edit.range = {textEnd, bufferSize};
				++numEdits;
			}
		}

		if(numEdits > 0){
			for(auto i = 0; i < numEdits - 1; ++i)
				edits[i].next = &edits[i + 1];

			buffer_batch_edit(app, buffer, edits);
		}

		delete[] tempBuffer;
		delete[] edits;
	}
}

CUSTOM_COMMAND_SIG(leon_trim_whitespace)
CUSTOM_DOC("Discards trailing whitespace and inserts single final newline"){
	auto view = get_active_view(app, Access_ReadWriteVisible);
	auto buffer = view_get_buffer(app, view, Access_ReadWriteVisible);

	leon_trim_whitespace_impl(app, buffer);
}
