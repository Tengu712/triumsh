#include "eval.h"

#include "cursor.h"
#include "exec.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK_INVALID_CHARACTER_FOUND(fn, ln) \
	if (has_error == 1) { \
		fprintf(stderr, "Invalid character found: %s (%zu)\n", (fn), (ln)); \
		exit(1); \
	}
#define CHECK_SINGLE_QUOTE_NOT_FOUND(fn, ln) \
	if (has_error == 2) { \
		fprintf(stderr, "Unclosed single quote found: %s (%zu)\n", (fn), (ln)); \
		exit(1); \
	}

typedef struct CommandLineBuffer_t {
	uint8_t        *start;
	uint8_t        *ptr;
	uint8_t **const cmdline;
	size_t          token_count;
} CommandLineBuffer;

void write_cmdline_buf(CommandLineBuffer *clb, const uint8_t *src, size_t size) {
	memcpy(clb->ptr, src, size);
	clb->ptr += size;
}

Cursor consume_until_special_char(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	Cursor new_cur = skip_until_special_char(cur, &has_error);
	CHECK_INVALID_CHARACTER_FOUND(file_name, cur.line)
	write_cmdline_buf(clb, cur.ptr, new_cur.ptr - cur.ptr);
	return new_cur;
}

Cursor pr_escape(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	switch (*cur.ptr) {
	case '\'':
	case '"':
	case '\\':
		write_cmdline_buf(clb, cur.ptr, 1);
		cur.ptr++;
		return cur;
	default:
		fprintf(stderr, "Invalid escape found: %s (%zu)\n", file_name, cur.line);
		exit(1);
	}
}

Cursor pr_variable(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	const uint8_t *start = cur.ptr;
	if (is_letter(*cur.ptr) || *cur.ptr == '_') cur.ptr++;
	else {
		fprintf(stderr, "Invalid variable name: %s (%zu)\n", file_name, cur.line);
		exit(1);
	}
	while (*cur.ptr && (is_letter(*cur.ptr) || is_digit(*cur.ptr) || *cur.ptr == '_')) cur.ptr++;

	static char var_name[256];
	const size_t name_len = cur.ptr - start;
	if (name_len >= sizeof(char) * 256) {
		fprintf(stderr, "Variable name too long: %s (%zu)\n", file_name, cur.line);
		exit(1);
	}
	memcpy(var_name, start, name_len);
	var_name[name_len] = '\0';

	const char *value = getenv(var_name);
	if (value) write_cmdline_buf(clb, (const uint8_t *)value, strlen(value));
	return cur;
}

Cursor pr_expansion(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	return pr_variable(file_name, cur, clb);
}

Cursor pr_single_quoted(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	Cursor new_cur = skip_to_after_single_quote(cur, &has_error);
	CHECK_INVALID_CHARACTER_FOUND(file_name, cur.line) // TODO: correct line.
	CHECK_SINGLE_QUOTE_NOT_FOUND(file_name, cur.line)
	write_cmdline_buf(clb, cur.ptr, new_cur.ptr - 1 - cur.ptr);
	return new_cur;
}

Cursor pr_double_quoted(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	const size_t start_line = cur.line;
	while (*cur.ptr) {
		int has_error = 0;
		switch (*cur.ptr) {
		case '"':
			cur.ptr++;
			return cur;
		case ' ':
		case '\t':
			{
				const Cursor new_cur = skip_whitespaces(cur);
				write_cmdline_buf(clb, cur.ptr, new_cur.ptr - cur.ptr);
				cur = new_cur;
			}
			break;
		case '\'':
		case '\n':
			write_cmdline_buf(clb, cur.ptr, 1);
			cur = advance_cursor(cur, &has_error);
			break;
		case '$':
			cur.ptr++;
			cur = pr_expansion(file_name, cur, clb);
			break;
		case '\\':
			cur.ptr++;
			cur = pr_escape(file_name, cur, clb);
			break;
		default:
			cur = consume_until_special_char(file_name, cur, clb);
			break;
		}
	}
	fprintf(stderr, "Unclosed double quote found: %s (%zu)\n", file_name, start_line);
	exit(1);
}

Cursor pr_token(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	while (*cur.ptr) {
		switch (*cur.ptr) {
		case ' ':
		case '\t':
		case '\n':
			goto end_token;
		case '\'':
			cur.ptr++;
			cur = pr_single_quoted(file_name, cur, clb);
			break;
		case '"':
			cur.ptr++;
			cur = pr_double_quoted(file_name, cur, clb);
			break;
		case '$':
			cur.ptr++;
			cur = pr_expansion(file_name, cur, clb);
			break;
		case '\\':
			cur.ptr++;
			cur = pr_escape(file_name, cur, clb);
			break;
		default:
			cur = consume_until_special_char(file_name, cur, clb);
			break;
		}
	}
end_token:
	clb->cmdline[clb->token_count] = clb->start;
	clb->token_count++;
	*clb->ptr++ = '\0';
	clb->start = clb->ptr;
	return cur;
}

// NOTE: This function consume a newline even if the command line should be ended.
Cursor pr_token_sep(Cursor cur, int *ended) {
	if (*cur.ptr == '\n') {
		int has_error = 0;
		cur = advance_cursor(cur, &has_error);
		if (!is_whitespace(*cur.ptr)) {
			*ended = 1;
			return cur;
		}
	}
	return skip_whitespaces(cur);
}

Cursor pr_cmdline(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	CommandLineBuffer new_clb = {
		clb->ptr,                        // start
		clb->ptr,                        // ptr
		&clb->cmdline[clb->token_count], // cmdline
		0,                               // token_count
	};

	const size_t start_line = cur.line;
	cur = pr_token(file_name, cur, &new_clb);

	while (*cur.ptr) {
		int ended = 0;
		switch (*cur.ptr) {
		case ' ':
		case '\t':
		case '\n':
			cur = pr_token_sep(cur, &ended);
			break;
		default:
			cur = pr_token(file_name, cur, &new_clb);
			break;
		}
		if (ended) break;
	}

	int exit_code = execute_command((const uint8_t *const *)new_clb.cmdline, new_clb.token_count);
	if (exit_code) {
		fprintf(stderr, "Command exited with %d: %s (%zu)\n", exit_code, file_name, start_line);
		exit(1);
	}
	return cur;
}

Cursor pr_top_level(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	switch (*cur.ptr) {
	case ' ':
	case '\t':
		fprintf(stderr, "Whitespace not allowed at top level: %s (%zu)\n", file_name, cur.line);
		exit(1);
	case '\n':
		cur = advance_cursor(cur, &has_error);
		CHECK_INVALID_CHARACTER_FOUND(file_name, cur.line) // TODO: correct line.
		return cur;
	case '#':
		cur = skip_line(cur, &has_error);
		CHECK_INVALID_CHARACTER_FOUND(file_name, cur.line) // TODO: correct line.
		return cur;
	default:
		return pr_cmdline(file_name, cur, clb);
	}
}

void eval(const char *file_name, const uint8_t *data) {
	static uint8_t  buf[2 * 1024 * 1024];
	static uint8_t *cmdline[1024];

	Cursor cur = {
		data, // ptr
		1,    // line
	};
	CommandLineBuffer clb = {
		buf,     // start
		buf,     // ptr
		cmdline, // cmdline
		0,       // token_count
	};

	while (*cur.ptr) {
		cur = pr_top_level(file_name, cur, &clb);
	}
}
