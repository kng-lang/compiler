/*
James Clarke - 2021
*/

#include "token.h"

// @TODO implement
std::string TokenList::to_json(){

	std::stringstream ss;
	ss << "{[\n";

	for (int i = 0; i < m_tokens.size(); i++)
		ss << m_tokens.at(i).to_json() << ",\n";
	ss << "]}";
	return ss.str();
}

std::string Token::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\":" << "\"" << debug_types[m_type] << "\",\n\"index\": " << m_index << ",\"line\": " << m_line << ",\n\"length\":"<<m_length<<",\n\"value\":\"" << m_value << "\"\n}";
	return ss.str();
}

const char* Token::debug_types[] = {
		"END",
		"COMMENT",
		"DOC", // documentation
		"UNDERSCORE",
		"NEWLINE",

		"HASH",
		"DIRECTIVE",

		"INCLUDE",
		"IMPORT",
		"RUN",
		"LINK",
		"HIDE",
		"GUARD",
		"EXPORT",
		"ASM",


		"IDENTIFIER",

		"STRING_LIT",
		"NUMBER",
		"TRU",
		"FLSE",

		"TYPE",
		"U0",
		"U8",
		"S8",
		"U16",
		"S16",
		"U32",
		"U64",
		"S32",
		"S64",
		"F32",
		"F64",
		"CHAR",
		"STRING",
		"FN",

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

		"TYPEOF",
		"AS",
		"INTERFACE",
		"IF",
		"ELSE",
		"INN",
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

u8 TokenConsumer::end(u32 ahead) {
	return current+ahead >= tokens.size();
}

Token TokenConsumer::consume(Token::Type type, const std::string err_msg) {
	if (peek().m_type != type)
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
	return !end() && peek().m_type == type;
}

u8 TokenConsumer::expect(Token::Type type, u32 ahead) {
	return !end(ahead) && peek(ahead).m_type == type;
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