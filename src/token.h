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
		HASH,
		INCLUDE,
		IMPORT,
		RUN,
		LINK,
		HIDE,
		GUARD,
		EXPORT,


		IDENTIFIER,

		STRING,
		NUMBER,
		TRU,
		FLSE,

		REFERENCE,
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
		LXOR,

		// bitwise operators
		LSHIFT,
		RSHIFT,
		BXOR,
		BOR,
		BAND,

		AS,
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
	u32 index  = 0;
	u32 line   = 0;
	u32 length = 0;

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

struct TokenConsumer {
	std::vector<Token> tokens;
	u32 current = 0;

	virtual u8 end();
	virtual Token consume(Token::Type type, const std::string err_msg);
	virtual u8 consume(Token::Type type);
	virtual u8 expect(Token::Type type);
	virtual Token prev();
	virtual Token peek();
	virtual Token peek(u32 amount);
	virtual Token peek_ahead();
	virtual Token next();
	virtual Token advance(u32 amount);
};