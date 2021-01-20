/*
James Clarke - 2021
*/

#pragma once

#include <vector>

#include "common.h"
#include "error.h"
#include "token.h"
#include <string>
#include <sstream>

struct Compiler;

struct Lexer {


	enum class LexResult {
		SUCCESS,
		FAIL,
	};

	LexResult m_result;
	CompilationUnit* m_unit;
	// the actual source file
	std::string m_src; // @TODO this should be a ref
	// current char being scanned
	u32 m_current = 0;
	// index and line along with offsets used for error handling
	u32 m_index = 0;
	u32 m_line = 0;
	u32 m_index_save_point = 0;
	u32 m_line_save_point = 0;

	std::vector<Token> m_tokens;

	Lexer(){}
	Lexer(std::string& file_contents, CompilationUnit* unit);


	void token(Token::Type type);
	void token(Token::Type type, std::string value);
	void skip_whitespace();
	void decide(Token::Type t1, Token::Type t2);
	void decide(Token::Type t1, Token::Type t2, Token::Type t3);
	inline void resetSavePoint();
	inline u8 consume(char c);
	inline char prev();
	inline char peek();
	inline char peek(u32 amount);
	inline char peek_ahead();
	inline char next();
	inline char advance(u32 amount);
	inline u8 end();
	inline u8 check_keyword(std::string rest, Token::Type t);
	inline u8 is_letter(char c);
	inline u8 is_digit(char c);
	inline u8 is_string(char c);
	void do_word(char start);
	void do_number(char start);
	void do_string(char start);
	void do_comment();
	void do_documentation();
	void do_escape();
	TokenList scan();
};