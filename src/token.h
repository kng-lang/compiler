#pragma once

#include <vector>
#include <variant>
#include <sstream>
#include "common.h"

struct Token{

	const static char* debug_types[];

	enum Type : u32 {

		END,
		COMMENT,
		DOC, // documentation
		UNDERSCORE,
		NEWLINE,


		IDENTIFIER,

		STRING,
		NUMBER,
		TRU,
		FLSE,

		DOT,
		DOUBLEDOT,
		TRIPLEDOT,

		SEMI_COLON, // inserted automatically
		COLON,
		QUESTION,
		BANG,
		COMMA,
		LBRACKET,
		RBRACKET,
		LCURLY,
		RCURLY,
		LPAREN,
		RPAREN,

		// maths
		PLUS,
		MINUS,
		STAR,
		DIV,
		MOD,

		// logical operators
		EQUALS,
		NEQ,
		GREATER,
		GEQ,
		LESS,
		LEQ,
		LOR,
		LAND,

		// bitwise operators
		LSHIFT,
		RSHIFT,
		BXOR,
		BOR,
		BAND,

		INTERFACE,
		IF,
		INN,
		FOR,
		RETURN,
		BREAK,
		CONTINUE,

		ASSIGN,
		QUICK_ASSIGN

	};

	Type type;
	u32 index;
	u32 line;
	u32 length;

	std::string value;

	std::string to_json();

	void debug() {
		log("token: {}", debug_types[type]);
	}
};

struct TokenList {
	std::vector<Token> tokens;
	// used for debugging, linting etc
	std::string to_json();

	TokenList() {}
	TokenList(std::vector<Token> tokens) : tokens(tokens) {}
};