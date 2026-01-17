#include "cmdline.h"

#include "exec.h"

void write_cmdline_buf(CommandLineBuffer *clb, Context *ctx, size_t start, size_t end) {
	// VALIDATION:
	if (end <= start) {
		fprintf(stderr, "Internal error: try to write 0 length token to cmdline buf\n");
		exit(2);
	}

	memcpy(&clb->buf[clb->cursor], &ctx->data[start], end - start);
	clb->cursor += end - start;
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

void on_newline_found(Context *ctx, CommandLineBuffer *clb) {
	advance_cursor(ctx);

	switch (clb->state) {
	case TS_IN_TOKEN:
		flush_token(clb);
		break;
	case TS_DOUBLE_QUOTED:
		clb->buf[clb->cursor++] = '\n';
		break;
	default:
		break;
	}
}

void on_whitespace_found(Context *ctx, CommandLineBuffer *clb) {
	const size_t start = ctx->cursor;
	skip_whitespaces(ctx);

	switch (clb->state) {
	case TS_IN_TOKEN:
		flush_token(clb);
		break;
	case TS_DOUBLE_QUOTED:
		write_cmdline_buf(clb, ctx, start, ctx->cursor);
		break;
	default:
		break;
	}
}

void on_single_quote_found(Context *ctx, CommandLineBuffer *clb) {
	const size_t start       = ++ctx->cursor;
	const size_t line_number = ctx->line_number;

	if (clb->state == TS_DOUBLE_QUOTED) {
		clb->buf[clb->cursor++] = '\'';
		return;
	}

	clb->state = TS_IN_TOKEN;
	if (!skip_to_next_single_quote(ctx)) {
		fprintf(stderr, "Unclosed single quote found: %s (%zu)\n", ctx->file_name, line_number);
		exit(1);
	}
	write_cmdline_buf(clb, ctx, start, ctx->cursor - 1);
}

void on_double_quote_found(Context *ctx, CommandLineBuffer *clb) {
	const size_t start = ++ctx->cursor;

	if (clb->state == TS_DOUBLE_QUOTED) {
		clb->state = TS_IN_TOKEN;
		return;
	}

	clb->state = TS_DOUBLE_QUOTED;
	skip_until_special_char(ctx);
	if (ctx->cursor == start) return;
	write_cmdline_buf(clb, ctx, start, ctx->cursor);
}

void on_general_char_found(Context *ctx, CommandLineBuffer *clb) {
	if (clb->state == TS_NEUTORAL) clb->state = TS_IN_TOKEN;
	const size_t start = ctx->cursor;
	skip_until_special_char(ctx);
	write_cmdline_buf(clb, ctx, start, ctx->cursor);
}

int execute_command_safely(Context *ctx, CommandLineBuffer *clb) {
	if (clb->state == TS_IN_TOKEN) flush_token(clb);
	else if (clb->state == TS_DOUBLE_QUOTED) {
		// TODO: show line number.
		fprintf(stderr, "Unclosed double quote found: %s\n", ctx->file_name);
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

int eval_cmdline(Context *ctx, CommandLineBuffer *clb) {
	while (is_continue(ctx)) {
		switch (ctx->data[ctx->cursor]) {
		case '\n':
			on_newline_found(ctx, clb);
			if (clb->state == TS_NEUTORAL && !is_whitespace(ctx)) goto end_tokenizing;
			break;

		case ' ':
		case '\t':
			on_whitespace_found(ctx, clb);
			break;

		case '\'':
			on_single_quote_found(ctx, clb);
			break;

		case '"':
			on_double_quote_found(ctx, clb);
			break;

		default:
			on_general_char_found(ctx, clb);
			break;
		}
	}
end_tokenizing:
	return execute_command_safely(ctx, clb);
}
