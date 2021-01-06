/*
James Clarke - 2021
*/

#include "token.h"

// @TODO implement
std::string TokenList::to_json(){

	std::stringstream ss;
	ss << "{[\n";

	for (int i = 0; i < tokens.size(); i++)
		ss << tokens.at(i).to_json() << ",\n";
	ss << "]}";
	return ss.str();
}

std::string Token::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\":" << "\"" << debug_types[type] << "\",\n\"index\": " << index << ",\"line\": " << line << ",\n\"length\":"<<length<<",\n\"value\":\"" << value << "\"\n}";
	return ss.str();
}

const char* Token::debug_types[] = {
		"END",
		"COMMENT",
		"DOC",
		"UNDERSCORE",
		"NEWLINE",
		"HASH",
		"INCLUDE",

		"IMPORT",
		"RUN",
		"LINK",
		"HIDE",
		"GUARD",
		"EXPORT",
		"ASM",

		"IDENTIFIER",

		"STRING",
		"NUMBER",
		"TRUE",
		"FALSE",

		"U0",
		"U8",
		"U16",
		"U32",
		"U64",
		"S32",
		"S64",
		"F32",
		"F64",
		"CHAR",

		"POINTER",
		"DOT",
		"DOUBLEDOT",
		"TRIPLEDOT",

		"SEMI_COLON", // inserted automatically
		"COLON",
		"QUESTION",
		"BANG",
		"COMMA",
		"LBRACKET",
		"RBRACKET",
		"LCURLY",
		"RCURLY",
		"LPAREN",
		"RPAREN",

		// maths
		"PLUS",
		"MINUS",
		"STAR",
		"DIV",
		"MOD",

		// logical operators
		"EQUALS",
		"NEQ",
		"GREATER",
		"GEQ",
		"LESS",
		"LEQ",
		"LOR",
		"LAND",
		"LXOR",

		// bitwise operators
		"LSHIFT",
		"RSHIFT",
		"BXOR",
		"BOR",
		"BAND",

		"AS",
		"INTERFACE",
		"IF",
		"ELSE",
		"IN",
		"FOR",
		"RETURN",
		"BREAK",
		"CONTINUE",

		"ASSIGN",
		"QUICK_ASSIGN"
};

u8 TokenConsumer::end() {
	return current >= tokens.size();
}

Token TokenConsumer::consume(Token::Type type, const std::string err_msg) {
	if (peek().type != type)
		// we need to insert an error AST here
		kng_log(err_msg);
	return next();
}


u8 TokenConsumer::consume(Token::Type type) {
	if (expect(type)) {
		next();
		return 1;
	}
	return 0;
}

u8 TokenConsumer::expect(Token::Type type) {
	return !end() && peek().type == type;
}

Token TokenConsumer::prev() {
	return peek(-1);
}

Token TokenConsumer::peek() {
	return tokens.at(current);
}

Token TokenConsumer::peek(u32 amount) {
	return tokens.at(current + amount);
}

Token TokenConsumer::peek_ahead() {
	return tokens.at(current + 1);
}

Token TokenConsumer::next() {
	return advance(1);
}

Token TokenConsumer::advance(u32 amount) {
	Token t = tokens.at(current);
	current += amount;
	return t;
}