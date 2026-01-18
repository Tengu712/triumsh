#include "cmdline.h"
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

TopLevelItem find_next_top_level_item(FileInfo *fi, Cursor *cur) {
	while (*cur->ptr) {
		int has_error = 0;
		switch (*cur->ptr) {
		// Skip a whiteline.
		case '\n':
			*cur = advance_cursor(*cur, &has_error);
			break;

		// Skip a comment line.
		case '#':
			*cur = skip_line(*cur, &has_error);
			break;

		// Disallow whitespace at top level.
		case ' ':
		case '\t':
			fprintf(stderr, "Whitespace not allowed at top level: %s (%zu)\n", fi->name, cur->line);
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
	Cursor cur = {
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
		int exit_code = 0;
		switch (find_next_top_level_item(&fi, &cur)) {
		case TLI_END_OF_FILE:
			goto end_process;
		case TLI_COMMAND_LINE:
			cur = eval_cmdline(&fi, cur, &clb, &exit_code);
			if (exit_code != 0) {
				// TODO: show command and line number.
				fprintf(stderr, "Command exited with %d: %s\n", exit_code, fi.name);
				exit(1);
			}
			break;
		}
	}

end_process:
	free((void *)data);
	return 0;
}
