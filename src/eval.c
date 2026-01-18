#include "eval.h"
#include "eval_internal.h"

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

Cursor eat_until_special_char(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	Cursor new_cur = skip_until_special_char(cur, &has_error);
	CHECK_INVALID_CHARACTER_FOUND(file_name, cur.line)
	write_cmdline_buf(clb, cur.ptr, new_cur.ptr - cur.ptr);
	return new_cur;
}

Cursor pr_top_level(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	switch (*cur.ptr) {
	case ' ':
	case '\t':
		fprintf(stderr, "Whitespace not allowed at top level: %s (%zu)\n", file_name, cur.line);
		exit(1);
		return cur;
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

Cursor pr_cmdline(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	CommandLineBuffer new_clb = {
		clb->ptr,     // start
		clb->ptr,     // ptr
		cmdline, // cmdline
		0,       // token_count
	};

	int has_error = 0;
	switch (*cur.ptr) {
	case ' ':
	case '\t':
		
	default:
	}
}

Cursor pr_token(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	// TODO: handle escape character.
	// TODO: handle env or emb.
	switch (*cur.ptr) {
	case '\'':
		cur.ptr++;
		return pr_single_quoted(file_name, cur, clb);
	case '"':
		cur.ptr++;
	default:
		cur = eat_until_special_char(file_name, cur, clb);
		return pr_token(file_name, cur, clb);
	}
}

Cursor pr_single_quoted(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	Cursor new_cur = skip_to_after_single_quote(cur, &has_error);
	CHECK_INVALID_CHARACTER_FOUND(file_name, cur.line) // TODO: correct line.
	CHECK_SINGLE_QUOTE_NOT_FOUND(file_name, cur.line)
	write_cmdline_buf(clb, cur.ptr, new_cur.ptr - 1 - cur.ptr);
	return pr_token(file_name, new_cur, clb);
}

int eval(const char *file_name, Cursor cur) {
	static uint8_t  buf[2 * 1024 * 1024];
	static uint8_t *cmdline[1024];

	CommandLineBuffer clb = {
		buf,     // start
		buf,     // ptr
		cmdline, // cmdline
		0,       // token_count
	};
}
