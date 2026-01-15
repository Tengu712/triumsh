#include "exec.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum State {
	TOP_LEVEL = 0,
	COMMENT,
	COMMAND_LINE,
};

// Returns file size in bytes, or LONG_MAX on failure.
long get_file_size(FILE *fp) {
	if (fseek(fp, 0, SEEK_END)) return LONG_MAX;
	const long file_size = ftell(fp);
	if (fseek(fp, 0, SEEK_SET)) return LONG_MAX;
	return file_size;
}

// Allocates and reads file contents with null termination.
// Returns NULL on failure.
uint8_t *read_file(FILE *fp, long file_size) {
	uint8_t *const data = (uint8_t *)malloc(file_size + 1);
	if (!data) return NULL;
	size_t readed_size = fread((void *)data, 1, file_size, fp);
	if (readed_size < file_size) return NULL;
	data[readed_size] = '\0';
	return data;
}

// Returns UTF-8 character length in bytes from the first byte, or 0 if invalid.
unsigned int get_utf8_char_length(uint8_t first_byte) {
	if ((first_byte & 0x80) == 0x00) return 1; // 0xxxxxxx
	if ((first_byte & 0xE0) == 0xC0) return 2; // 110xxxxx
	if ((first_byte & 0xF0) == 0xE0) return 3; // 1110xxxx
	if ((first_byte & 0xF8) == 0xF0) return 4; // 11110xxx
	return 0;
}

// Transitions state based on the first byte of a character.
// Returns 0 if an error is detected.
int top_level(uint8_t c, unsigned int *state, unsigned int *prev_state) {
	switch (c) {
	// Skip whiteline at top level.
	case '\n':
		break;

	// Disallow whitespace at top level.
	case '\t':
	case ' ':
		fprintf(stderr, "Whitespace not allowed at top level");
		return 0;

	// Enter a comment.
	case '#':
		*prev_state = *state;
		*state      = COMMENT;
		break;

	// Otherwise, start command line.
	default:
		*state = COMMAND_LINE;
		break;
	}
	return 1;
}

// Constructs and executes a command line.
// Transitions to TOP_LEVEL state when executed.
// Returns 0 on success, or the command's exit code if executed.
int command_line(
	const uint8_t *data, long file_size,
	unsigned int cursor, unsigned int char_len,
	unsigned int *state
) {
	static uint8_t g_cmdline_buf[2 * 1024 * 1024];
	static size_t  g_cmdline_cursor = 0;

	if (data[cursor] != '\n') {
		memcpy(&g_cmdline_buf[g_cmdline_cursor], &data[cursor], char_len);
		g_cmdline_cursor += char_len;
	}

	int should_run =
		cursor + char_len >= file_size
		|| data[cursor] == '\n' && data[cursor + 1] != ' ' && data[cursor + 1] != '\t';

	if (!should_run) return 0;

	*state = TOP_LEVEL;
	g_cmdline_buf[g_cmdline_cursor] = '\0';
	g_cmdline_cursor = 0;
	int result = execute_command((char *)g_cmdline_buf);

	if (result != 0) fprintf(stderr, "Command exited with code %d: %s", result, g_cmdline_buf);

	return result;
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

	const long file_size = get_file_size(fp);
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

	unsigned int cursor      = 0;
	unsigned int line_number = 1;
	unsigned int state      = TOP_LEVEL;
	unsigned int prev_state = TOP_LEVEL;

	while (cursor < file_size) {
		const unsigned int char_len = get_utf8_char_length(data[cursor]);
		if (!char_len) {
			fprintf(stderr, "Invalid character found: %s (%u)\n", argv[1], line_number);
			free((void *)data);
			return 1;
		}

		if (state == TOP_LEVEL && !top_level(data[cursor], &state, &prev_state)) {
			fprintf(stderr, ": %s (%u)\n", argv[1], line_number);
			free((void *)data);
			return 1;
		}

		switch (state) {
		case COMMAND_LINE:
			if (command_line(data, file_size, cursor, char_len, &state) != 0) {
				fprintf(stderr, ": %s (%u)\n", argv[1], line_number);
				free((void *)data);
				return 1;
			}
			break;

		case COMMENT:
			if (data[cursor] == '\n') state = prev_state;
			break;

		default:
			break;
		}

		if (data[cursor] == '\n') line_number++;
		cursor += char_len;
	}

	free((void *)data);
	return 0;
}
