#include "cursor.h"

#include <stdio.h>
#include <string.h>

#define SUIT_NAME "test_cursor"

#define DEF_TEST(tn) \
	int tn(void) { \
		printf("%s: %s ... ", SUIT_NAME, #tn);
#define END_TEST \
		printf("ok\n"); \
		return 0; \
	}

#define ASSERT(b) if (!(b)) { printf("fail\n"); return 1; }

// ========================================================================= //
//     advance_cursor                                                        //
// ========================================================================= //

DEF_TEST(test_advance_ascii)
	const uint8_t *text = (const uint8_t *)"abc";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = advance_cursor(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 1)
	ASSERT(res.line == cur.line)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_advance_newline)
	const uint8_t *text = (const uint8_t *)"\nfoo";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = advance_cursor(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 1)
	ASSERT(res.line == cur.line + 1)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_advance_multi_byte)
	const uint8_t *text = (const uint8_t *)"あabc";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = advance_cursor(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 3)
	ASSERT(res.line == cur.line)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_advance_to_eof)
	const uint8_t *text = (const uint8_t *)"あ";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = advance_cursor(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 3)
	ASSERT(res.line == cur.line)
	ASSERT(has_error == 0)
	ASSERT(*res.ptr == '\0')
END_TEST

DEF_TEST(test_detect_invalid_char_when_advancing)
	const uint8_t *text = (const uint8_t *)"\xFF\xFE a";
	Cursor cur = {text, 1};

	int has_error = 0;
	advance_cursor(cur, &has_error);

	ASSERT(has_error == 1)
END_TEST

// ========================================================================= //
//     skip_whitespaces                                                      //
// ========================================================================= //

DEF_TEST(test_skip_whitespaces)
	const uint8_t *text = (const uint8_t *)"  \t foo";
	Cursor cur = {text, 1};

	Cursor res = skip_whitespaces(cur);

	ASSERT(res.ptr == cur.ptr + 4)
	ASSERT(res.line == 1)
END_TEST

DEF_TEST(test_skip_whitespaces_until_newline)
	const uint8_t *text = (const uint8_t *)"  \n foo";
	Cursor cur = {text, 1};

	Cursor res = skip_whitespaces(cur);

	ASSERT(res.ptr == cur.ptr + 2)
	ASSERT(res.line == 1)
END_TEST

DEF_TEST(test_skip_whitespaces_to_eof)
	const uint8_t *text = (const uint8_t *)"  ";
	Cursor cur = {text, 1};

	Cursor res = skip_whitespaces(cur);

	ASSERT(res.ptr == cur.ptr + 2)
	ASSERT(res.line == 1)
	ASSERT(*res.ptr == '\0')
END_TEST

DEF_TEST(test_skip_no_whitespace)
	const uint8_t *text = (const uint8_t *)"foo";
	Cursor cur = {text, 1};

	Cursor res = skip_whitespaces(cur);

	ASSERT(res.ptr == cur.ptr)
	ASSERT(res.line == 1)
END_TEST

// ========================================================================= //
//     skip_line                                                             //
// ========================================================================= //

DEF_TEST(test_skip_line)
	const uint8_t *text = (const uint8_t *)"foo\nbar";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_line(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 4)
	ASSERT(res.line == 2)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_skip_line_even_if_escaped)
	const uint8_t *text = (const uint8_t *)"foo\\\nbar";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_line(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 5)
	ASSERT(res.line == 2)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_skip_line_to_eof)
	const uint8_t *text = (const uint8_t *)"foo";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_line(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 3)
	ASSERT(res.line == 1)
	ASSERT(has_error == 0)
	ASSERT(*res.ptr == '\0')
END_TEST

// ========================================================================= //
//     skip_to_after_single_quote                                            //
// ========================================================================= //

DEF_TEST(test_skip_to_after_single_quote)
	const uint8_t *text = (const uint8_t *)"foo 'bar";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_to_after_single_quote(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 5)
	ASSERT(res.line == 1)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_skip_escaped_singlequote_and_newline)
	const uint8_t *text = (const uint8_t *)"foo\\'bar\nbaz'fuga";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_to_after_single_quote(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 13)
	ASSERT(res.line == 2)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_skip_to_after_single_quote_eof)
	const uint8_t *text = (const uint8_t *)"foo'";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_to_after_single_quote(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 4)
	ASSERT(res.line == 1)
	ASSERT(has_error == 0)
	ASSERT(*res.ptr == '\0')
END_TEST

DEF_TEST(test_detect_unclosed_singlequote)
	const uint8_t *text = (const uint8_t *)"foo bar";
	Cursor cur = {text, 1};

	int has_error = 0;
	skip_to_after_single_quote(cur, &has_error);

	ASSERT(has_error == 2)
END_TEST

// ========================================================================= //
//     skip_until_special_char                                               //
// ========================================================================= //

DEF_TEST(test_skip_until_special_char)
	const uint8_t *text = (const uint8_t *)"foo bar";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_until_special_char(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 3)
	ASSERT(res.line == 1)
	ASSERT(has_error == 0)
END_TEST

DEF_TEST(test_skip_to_eof)
	const uint8_t *text = (const uint8_t *)"foobar";
	Cursor cur = {text, 1};

	int has_error = 0;
	Cursor res = skip_until_special_char(cur, &has_error);

	ASSERT(res.ptr == cur.ptr + 6)
	ASSERT(res.line == 1)
	ASSERT(has_error == 0)
	ASSERT(*res.ptr == '\0')
END_TEST

// ========================================================================= //
//     main                                                                  //
// ========================================================================= //

#define TEST(tn) error = tn() || error;
int main(void) {
	int error = 0;
	TEST(test_advance_ascii)
	TEST(test_advance_newline)
	TEST(test_advance_multi_byte)
	TEST(test_advance_to_eof)
	TEST(test_detect_invalid_char_when_advancing)
	TEST(test_skip_whitespaces)
	TEST(test_skip_whitespaces_until_newline)
	TEST(test_skip_whitespaces_to_eof)
	TEST(test_skip_no_whitespace)
	TEST(test_skip_line)
	TEST(test_skip_line_even_if_escaped)
	TEST(test_skip_line_to_eof)
	TEST(test_skip_to_after_single_quote)
	TEST(test_skip_escaped_singlequote_and_newline)
	TEST(test_skip_to_after_single_quote_eof)
	TEST(test_detect_unclosed_singlequote)
	TEST(test_skip_until_special_char)
	TEST(test_skip_to_eof)
	return error;
}
