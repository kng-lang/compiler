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
	ss << "{\n\"type\":" << "\"" << debug_types[type] << "\",\n\"value\":\"" << value << "\"\n}";
	return ss.str();
}

const char* Token::debug_types[] = {
		"END",
		"COMMENT",
		"DOC",
		"UNDERSCORE",
		"NEWLINE",

		"IDENTIFIER",

		"STRING",
		"NUMBER",
		"TRUE",
		"FALSE",


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

		// bitwise operators
		"LSHIFT",
		"RSHIFT",
		"BXOR",
		"BOR",
		"BAND",

		"INTERFACE",
		"IF",
		"IN",
		"FOR",
		"RETURN",
		"BREAK",
		"CONTINUE",

		"ASSIGN",
		"QUICK_ASSIGN"
};