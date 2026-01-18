#include "cmdline.h"
#include "context.h"
#include "file.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if CHAR_BIT != 8
#error "This application requires CHAR_BIT == 8"
#endif

typedef enum TopLevelItem_t {
	TLI_END_OF_FILE = 0,
	TLI_COMMAND_LINE,
} TopLevelItem;

TopLevelItem find_next_top_level_item(Context *ctx) {
	while (is_continue(ctx)) {
		switch (ctx->data[ctx->cursor]) {
		// Skip a whiteline.
		case '\n':
			advance_cursor(ctx);
			break;

		// Skip a comment line.
		case '#':
			skip_to_next_newline(ctx);
			break;

		// Disallow whitespace at top level.
		case ' ':
		case '\t':
			fprintf(stderr, "Whitespace not allowed at top level: %s (%zu)\n", ctx->file_name, ctx->line_number);
			exit(1);
			break;

		// Command line found.
		default:
			return TLI_COMMAND_LINE;
		}
	}
	return TLI_END_OF_FILE;
}

int main(int argc, const char *const argv[]) {
	if (argc < 2) {
		printf("Usage: trish <file-path>\n");
		return 0;
	}

	FILE *const fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "No such file: %s\n", argv[1]);
		return 1;
	}

	const size_t file_size = get_file_size(fp);
	if (file_size == LONG_MAX) {
		fprintf(stderr, "Failed to seek in '%s'\n", argv[1]);
		fclose(fp);
		return 1;
	}

	uint8_t *const data = read_file(fp, file_size);
	if (!data) {
		fprintf(stderr, "Failed to read '%s'\n", argv[1]);
		fclose(fp);
		return 1;
	}

	fclose(fp);

	FileInfo fi = {
		argv[1],   // name
		data,      // data
		file_size, // size
	};
	Cursor ctx = {
		data, // ptr
		1,    // line
	};
	static uint8_t *cmdline[1024];
	static uint8_t  buf[2 * 1024 * 1024];
	CommandLineBuffer clb = {
		buf,         // buf
		0,           // start
		0,           // cursor
		cmdline,     // cmdline
		0,           // token_count
		TS_NEUTORAL, // state
	};

	while (1) {
		int cmdline_res;
		switch (find_next_top_level_item(&ctx)) {
		case TLI_END_OF_FILE:
			goto end_process;
		case TLI_COMMAND_LINE:
			cmdline_res = eval_cmdline(&ctx, &clb);
			if (cmdline_res != 0) {
				// TODO: show command and line number.
				fprintf(stderr, "Command exited with %d: %s\n", cmdline_res, ctx.file_name);
				exit(1);
			}
			break;
		}
	}

end_process:
	free((void *)data);
	return 0;
}
