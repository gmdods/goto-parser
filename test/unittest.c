#ifndef UNITTEST_MAIN
#include "unittest.h"
#include <stdbool.h>
#include <stdlib.h>

#include "../include/lexer.h"
#include "../include/macros.h"
#include "../include/parser.h"

#endif /* ifndef UNITTEST_MAIN */

static const char8_t * str =
    cast(const char8_t *, "let num = 30;\n"
			  "let letter = 'a';\n"
			  "if (num <= 32)\n"
			  "then num + (letter - 'a') * 3\n"
			  "else 30");

unittest("lexer") {
	struct token expected[] = {
	    {tokenof(KEYWORD_LET), str + 0, 3},
	    {TOKEN_LABEL, str + 4, 3},
	    {'=', str + 8, 1},
	    {TOKEN_DECIMAL, str + 10, 2},
	    {';', str + 12, 1},
	    {tokenof(KEYWORD_LET), str + 14, 3},
	    {TOKEN_LABEL, str + 18, 6},
	    {'=', str + 25, 1},
	    {'\'', str + 28, 1},
	    {';', str + 30, 1},
	    {tokenof(KEYWORD_IF), str + 32, 2},
	    {'(', str + 35, 1},
	    {TOKEN_LABEL, str + 36, 3},
	    {digraph('<'), str + 40, 2},
	    {TOKEN_DECIMAL, str + 43, 2},
	    {')', str + 45, 1},
	    {tokenof(KEYWORD_THEN), str + 47, 4},
	    {TOKEN_LABEL, str + 52, 3},
	    {'+', str + 56, 1},
	    {'(', str + 58, 1},
	    {TOKEN_LABEL, str + 59, 6},
	    {'-', str + 66, 1},
	    {'\'', str + 69, 1},
	    {')', str + 71, 1},
	    {'*', str + 73, 1},
	    {TOKEN_DECIMAL, str + 75, 1},
	    {tokenof(KEYWORD_ELSE), str + 77, 4},
	    {TOKEN_DECIMAL, str + 82, 2},
	};

	struct lexer lexer = {.ptr = str};
	struct token token = {0};
	for (size_t index = 0; (token = lex(&lexer)).type != TOKEN_EOF;
	     ++index) {
		struct token expect_token = expected[index];
		ensure(expect_token.type == token.type),
		    ensure(expect_token.ptr == token.ptr),
		    ensure(expect_token.span == token.span);
	}
}

unittest("parser") {

	struct node expected[] = {
	    {0, {tokenof(KEYWORD_LET), str + 0, 3}},
	    {1, {TOKEN_LABEL, str + 4, 3}},
	    {1, {'=', str + 8, 1}},
	    {1, {TOKEN_DECIMAL, str + 10, 2}},
	    {0, {tokenof(KEYWORD_LET), str + 14, 3}},
	    {5, {TOKEN_LABEL, str + 18, 6}},
	    {5, {'=', str + 25, 1}},
	    {5, {'\'', str + 28, 1}},
	    {0, {tokenof(KEYWORD_IF), str + 32, 2}},
	    {9, {'(', str + 35, 1}},
	    {10, {TOKEN_LABEL, str + 36, 3}},
	    {10, {digraph('<'), str + 40, 2}},
	    {12, {TOKEN_DECIMAL, str + 43, 2}},
	    {9, {tokenof(KEYWORD_THEN), str + 47, 4}},
	    {14, {TOKEN_LABEL, str + 52, 3}},
	    {14, {'+', str + 56, 1}},
	    {16, {'(', str + 58, 1}},
	    {17, {TOKEN_LABEL, str + 59, 6}},
	    {17, {'-', str + 66, 1}},
	    {19, {'\'', str + 69, 1}},
	    {19, {'*', str + 73, 1}},
	    {21, {TOKEN_DECIMAL, str + 75, 1}},
	    {9, {tokenof(KEYWORD_ELSE), str + 77, 4}},
	    {23, {TOKEN_DECIMAL, str + 82, 2}},
	};

	struct node node = {0};
	struct parser parser = {.lexer = (struct lexer){.ptr = str},
				.arena = alloc(256U * sizeof(struct node))};
	error_t error = parse(&parser);
	ensure(error == ERROR_EOF);

	for (size_t index = 0; index != sizeof(expected) / sizeof(struct node);
	     ++index) {
		struct node node = ((struct node *) parser.arena.ptr)[index];
		struct node expect = expected[index];
		ensure(expect.token.type == node.token.type),
		    ensure(expect.token.ptr == node.token.ptr),
		    ensure(expect.token.span == node.token.span),
		    ensure(expect.index == node.index);
	}

	dealloc(&parser.arena);
}

unittest("parser errors") {
	static const char8_t * issue =
	    cast(const char8_t *, "while (1) { break; } != 1");

	struct parser parser = {.lexer = (struct lexer){.ptr = issue},
				.arena = alloc(256U * sizeof(struct node))};
	error_t error = parse(&parser);
	ensure(error == ERROR_BAD), ensure(parser.token.type == digraph('!')),
	    ensure(parser.token.span == 2),
	    ensure(parser.token.ptr == issue + 21);

	dealloc(&parser.arena);
}
