#include "cmdline.h"

#include "exec.h"

#define ERROR_INVALID_CHARACTER_FOUND(name, line) { \
		fprintf(stderr, "Invalid character found: %s (%zu)\n", (name), (line)); \
		exit(1); \
	}

void write_cmdline_buf(CommandLineBuffer *clb, const uint8_t *src, size_t size) {
	memcpy(&clb->buf[clb->cursor], src, size);
	clb->cursor += size;
}

void flush_token(CommandLineBuffer *clb) {
	// VALIDATION:
	if (clb->state != TS_IN_TOKEN) {
		fprintf(stderr, "Internal error: try to flush a token but in literal or not in token\n");
		exit(2);
	}

	clb->cmdline[clb->token_count] = &clb->buf[clb->start];
	clb->token_count++;
	clb->buf[clb->cursor] = '\0';
	clb->cursor++;
	clb->start = clb->cursor;
	clb->state = TS_NEUTORAL;
}

Cursor on_newline_found(Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	cur = advance_cursor(cur, &has_error);

	switch (clb->state) {
	case TS_IN_TOKEN:
		flush_token(clb);
		return cur;
	case TS_DOUBLE_QUOTED:
		clb->buf[clb->cursor++] = '\n';
		return cur;
	default:
		return cur;
	}
}

Cursor on_whitespace_found(Cursor cur, CommandLineBuffer *clb) {
	const uint8_t *start = cur.ptr;
	cur = skip_whitespaces(cur);

	switch (clb->state) {
	case TS_IN_TOKEN:
		flush_token(clb);
		return cur;
	case TS_DOUBLE_QUOTED:
		write_cmdline_buf(clb, start, cur.ptr - start);
		return cur;
	default:
		return cur;
	}
}

Cursor on_single_quote_found(FileInfo *fi, Cursor cur, CommandLineBuffer *clb) {
	if (clb->state == TS_DOUBLE_QUOTED) {
		cur.ptr++;
		clb->buf[clb->cursor++] = '\'';
		return cur;
	}

	clb->state = TS_IN_TOKEN;

	// Skip the current single quote.
	cur.ptr++;
	const Cursor start = cur;

	// Skip the contents.
	int has_error = 0;
	cur = skip_to_after_single_quote(cur, &has_error);
	if (has_error == 2) {
		fprintf(stderr, "Unclosed single quote found: %s (%zu)\n", fi->name, start.line);
		exit(1);
	}
	else if (has_error) ERROR_INVALID_CHARACTER_FOUND(fi->name, start.line) // TODO: show correct line.

	// Write the contents.
	write_cmdline_buf(clb, start.ptr, cur.ptr - 1 - start.ptr);
	return cur;
}

Cursor on_double_quote_found(FileInfo *fi, Cursor cur, CommandLineBuffer *clb) {
	if (clb->state == TS_DOUBLE_QUOTED) {
		clb->state = TS_IN_TOKEN;
		cur.ptr++;
		return cur;
	}

	clb->state = TS_DOUBLE_QUOTED;

	// Skip the current double quote.
	cur.ptr++;
	const Cursor start = cur;

	// Skip until any special character.
	int has_error = 0;
	cur = skip_until_special_char(cur, &has_error);
	if (cur.ptr == start.ptr) return cur;
	if (has_error) ERROR_INVALID_CHARACTER_FOUND(fi->name, start.line); // TODO: show correct line.

	// Write the contents.
	write_cmdline_buf(clb, start.ptr, cur.ptr - start.ptr);
	return cur;
}

Cursor on_general_char_found(FileInfo *fi, Cursor cur, CommandLineBuffer *clb) {
	if (clb->state == TS_NEUTORAL) clb->state = TS_IN_TOKEN;
	const Cursor start = cur;
	int has_error = 0;
	cur = skip_until_special_char(cur, &has_error);
	if (has_error) ERROR_INVALID_CHARACTER_FOUND(fi->name, start.line) // TODO: show correct line.
	write_cmdline_buf(clb, start.ptr, cur.ptr - start.ptr);
	return cur;
}

int execute_command_safely(FileInfo *fi, CommandLineBuffer *clb) {
	if (clb->state == TS_IN_TOKEN) flush_token(clb);
	else if (clb->state == TS_DOUBLE_QUOTED) {
		// TODO: show line number.
		fprintf(stderr, "Unclosed double quote found: %s\n", fi->name);
		exit(1);
	}

	const size_t count = clb->token_count;
	clb->cmdline[clb->token_count] = NULL;
	clb->start       = 0;
	clb->cursor      = 0;
	clb->token_count = 0;
	clb->state       = TS_NEUTORAL;
	return execute_command((const uint8_t *const *)clb->cmdline, count);
}

Cursor eval_cmdline(FileInfo *fi, Cursor cur, CommandLineBuffer *clb, int *exit_code) {
	while (*cur.ptr) {
		switch (*cur.ptr) {
		case '\n':
			cur = on_newline_found(cur, clb);
			if (clb->state == TS_NEUTORAL && !is_whitespace(*cur.ptr)) goto end_tokenizing;
			break;

		case ' ':
		case '\t':
			cur = on_whitespace_found(cur, clb);
			break;

		case '\'':
			cur = on_single_quote_found(fi, cur, clb);
			break;

		case '"':
			cur = on_double_quote_found(fi, cur, clb);
			break;

		default:
			cur = on_general_char_found(fi, cur, clb);
			break;
		}
	}
end_tokenizing:
	*exit_code = execute_command_safely(fi, clb);
	return cur;
}
