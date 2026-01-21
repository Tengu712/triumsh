#include "eval.h"
#include "eval_internal.h"

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

void write_cmdline_buf(CommandLineBuffer *clb, const uint8_t *src, size_t size) {
	memcpy(clb->ptr, src, size);
	clb->ptr += size;
}

Cursor consume_until_special_char(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	int has_error = 0;
	Cursor new_cur = skip_until_special_char(cur, &has_error);
	CHECK_INVALID_CHARACTER_FOUND(file_name, cur.line)
	if (new_cur.ptr == cur.ptr) {
		fprintf(stderr, "Internal error: Consume nothing: %s\n", file_name);
		exit(2);
	}
	write_cmdline_buf(clb, cur.ptr, new_cur.ptr - cur.ptr);
	return new_cur;
}

// NOTE: The current character must a whitespace.
Cursor consume_whitespaces(Cursor cur, CommandLineBuffer *clb) {
	const Cursor new_cur = skip_whitespaces(cur);
	write_cmdline_buf(clb, cur.ptr, new_cur.ptr - cur.ptr);
	return new_cur;
}

Cursor pr_escape(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
	switch (*cur.ptr) {
	case '\'':
	case '"':
	case '\\':
	case '$':
	case '(':
	case ')':
		write_cmdline_buf(clb, cur.ptr, 1);
		cur.ptr++;
		return cur;
	default:
		fprintf(stderr, "Invalid escape found: %s (%zu)\n", file_name, cur.line);
		exit(1);
	}
}

Cursor pr_expansion_variable(const char *file_name, Cursor cur, CommandLineBuffer *clb) {
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
	switch (*cur.ptr) {
	case '(':
		cur.ptr++;
		return pr_cmdline(file_name, cur, clb, ')', 1);
	default:
		return pr_expansion_variable(file_name, cur, clb);
	}
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
		switch (*cur.ptr) {
		case '"':
			cur.ptr++;
			return cur;
		case ' ':
		case '\t':
			cur = consume_whitespaces(cur, clb);
			break;
		case '\\':
			cur.ptr++;
			cur = pr_escape(file_name, cur, clb);
			break;
		case '$':
			cur.ptr++;
			cur = pr_expansion(file_name, cur, clb);
			break;
		case '\n':
		case '\'':
		case '(':
		case ')':
			write_cmdline_buf(clb, cur.ptr, 1);
			cur = advance_cursor(cur, NULL);
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
	const uint8_t *const start = cur.ptr;
	while (*cur.ptr) {
		switch (*cur.ptr) {
		case ' ':
		case '\t':
		case '\n':
		case '(':
		case ')':
			goto end_token;
		case '\\':
			cur.ptr++;
			cur = pr_escape(file_name, cur, clb);
			break;
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
		default:
			cur = consume_until_special_char(file_name, cur, clb);
			break;
		}
	}
end_token:
	if (cur.ptr == start) {
		fprintf(stderr, "Internal error: Token without length found: %s\n", file_name);
		exit(2);
	}
	clb->cmdline[clb->token_count] = clb->start;
	clb->token_count++;
	*clb->ptr++ = '\0';
	clb->start = clb->ptr;
	return cur;
}

Cursor pr_cmdline(const char *file_name, Cursor cur, CommandLineBuffer *clb, uint8_t end_char, int piped) {
	CommandLineBuffer new_clb = {
		clb->ptr,                        // start
		clb->ptr,                        // ptr
		&clb->cmdline[clb->token_count], // cmdline
		0,                               // token_count
	};
	const size_t start_line = cur.line;

	if (*cur.ptr == end_char) {
		fprintf(stderr, "The command line is empty: %s (%zu)", file_name, cur.line);
		exit(1);
	}
	if (is_whitespace(*cur.ptr) || *cur.ptr == '\n') {
		fprintf(stderr, "Command line must not start with whitespace or newline: %s (%zu)", file_name, cur.line);
		exit(1);
	}

	cur = pr_token(file_name, cur, &new_clb);
	while (*cur.ptr) {
		if (*cur.ptr == end_char) break;
		int ended = 0;
		switch (*cur.ptr) {
		case ' ':
		case '\t':
			cur = skip_whitespaces(cur);
			break;
		case '\n':
			cur = advance_cursor(cur, NULL);
			if (!is_whitespace(*cur.ptr)) ended = 1;
			break;
		case '(':
		case ')':
			fprintf(stderr, "Unexpected character '%c' found: %s (%zu)\n", *cur.ptr, file_name, cur.line);
			exit(1);
		default:
			cur = pr_token(file_name, cur, &new_clb);
			break;
		}
		if (ended) break;
	}

	if (end_char != '\0') {
		if (*cur.ptr == end_char) cur.ptr++;
		else {
			fprintf(stderr, "The command line not ended with '%c': %s (%zu-%zu)\n", end_char, file_name, start_line, cur.line);
			exit(1);
		}
	}

	size_t output_len = 0;
	int exit_code = execute_command(
		(const uint8_t *const *)new_clb.cmdline,
		new_clb.token_count,
		piped ? clb->ptr : NULL,
		piped ? &output_len : NULL
	);
	if (exit_code) {
		fprintf(stderr, "Command exited with %d: %s (%zu)\n", exit_code, file_name, start_line);
		exit(1);
	}

	if (piped) {
		while (output_len > 1 && clb->ptr[output_len - 1] == '\n') output_len--;
		clb->ptr += output_len;
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
		return pr_cmdline(file_name, cur, clb, '\0', 0);
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
