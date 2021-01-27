/*
James Clarke - 2021
*/

#pragma once

#include <vector>
#include <variant>
#include <sstream>
#include "common.h"

struct Token{


	struct Position {
		u32 m_index_start;
		u32 m_index_end;
		u32 m_line_start;
		u32 m_line_end;
		Position() {}
		Position(u32 index_start, u32 index_end, u32 line_start, u32 line_end)
			: m_index_start(index_start), m_index_end(index_end), m_line_start(line_start), m_line_end(line_end) {}

		// we merge the positions by taking the lower and upper most values
		Position merge(Position other) {
			Position p;
			p.m_index_start = (other.m_index_start < m_index_start) ? other.m_index_start : m_index_start;
			p.m_index_end = (other.m_index_end > m_index_end) ? other.m_index_end : m_index_end;
			p.m_line_start = (other.m_line_start < m_index_start) ? other.m_line_start : m_line_start;
			p.m_line_end = (other.m_line_end > m_line_end) ? other.m_line_end : m_line_end;
			return p;
		}


		Position add_index_start(u32 offset) {
			m_index_start += offset;
			return *this;
		}

		Position add_index_end(u32 offset) {
			m_index_end += offset;
			return *this;
		}

		Position add_line_start(u32 offset) {
			m_line_start += offset;
			return *this;
		}

		Position add_line_end(u32 offset) {
			m_line_end += offset;
			return *this;
		}
	};

	const static char* debug_types[];

	enum Type : u32 {

		END,
		COMMENT,
		DOC, // documentation
		UNDERSCORE,
		NEWLINE,

		HASH,
		DIRECTIVE,

		INCLUDE,
		IMPORT,
		MODULE,
		USE,
		RUN,
		LINK,
		HIDE,
		GUARD,
		EXPORT,
		ASM,


		IDENTIFIER,

		STRING_LIT,
		NUMBER,
		TRU,
		FLSE,
		TYPE,

		U0,
		U8,
		S8,
		U16,
		S16,
		U32,
		U64,
		S32,
		S64,
		F32,
		F64,
		CHAR,
		STRING,
		FN,

		POINTER,
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

		TYPEOF,
		AS,
		INTERFACE,
		IF,
		ELSE,
		INN,
		FOR,
		RETURN,
		BREAK,
		CONTINUE,

		ASSIGN,
		QUICK_ASSIGN

	};

	Type m_type;
	u32 m_index  = 0;
	u32 m_line   = 0;
	u32 m_length = 0;
	Position m_position;

	std::string m_value;

	std::string to_json();

	bool operator<(const Token& o)  const {
		return m_value < o.m_value;
	}

	void debug() {
		kng_log("token: {}", debug_types[m_type]);
	}
};

struct TokenList {
	std::vector<Token> m_tokens;
	// used for debugging, linting etc
	std::string to_json();

	TokenList() {}
	TokenList(std::vector<Token> tokens) : m_tokens(tokens) {}
};

struct TokenConsumer {
	std::vector<Token> tokens;
	u32 current = 0;

	virtual u8 end();
	virtual u8 end(u32 ahead);
	virtual Token consume(Token::Type type, const std::string err_msg);
	virtual u8 consume(Token::Type type);
	virtual u8 expect(Token::Type type);
	virtual u8 expect(Token::Type type, u32 ahead);
	virtual Token prev();
	virtual Token peek();
	virtual Token peek(u32 amount);
	virtual Token peek_ahead();
	virtual Token next();
	virtual Token advance(u32 amount);
};