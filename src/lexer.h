#pragma once

#include <vector>

#include "common.h"
#include "error.h"
#include "token.h"
#include <string>
#include <sstream>

struct Compiler;

struct Lexer {


	CompilationUnit* unit;
	// the actual source file
	std::string src;
	// current char being scanned
	u32 current = 0;
	// index and line along with offsets used for error handling
	u32 index = 1;
	u32 line = 1;
	u32 indexSavePoint = index;
	u32 lineSavePoint = line;

	std::vector<Token> tokens;

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
	TokenList scan();
};