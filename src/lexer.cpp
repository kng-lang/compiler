#include "lexer.h"
#include "compiler.h"

Lexer::Lexer(std::string& file_contents, CompilationUnit* unit){
	this->unit = unit;
	this->src = file_contents;
}

TokenList Lexer::scan() {
	while (!end()) {
		// do stuff here
		skip_whitespace();
		auto current = next();
		switch (current) {
			// @TODO these are sequential i.e. \r\n is a newline
			case '\n': token(Token::Type::NEWLINE); index = 1; line++; break;
			case '\r': token(Token::Type::NEWLINE); index = 1; line++; break;
			case '#': token(Token::Type::HASH); break;
			case '+': token(Token::Type::PLUS); break;
			case '-': token(Token::Type::MINUS); break;
			case '*': token(Token::Type::STAR); break;
			case '(': token(Token::Type::LPAREN); break;
			case ')': token(Token::Type::RPAREN); break;
			case '[': token(Token::Type::LBRACKET); break;
			case ']': token(Token::Type::RBRACKET); break;
			case '{': token(Token::Type::LCURLY); break;
			case '}': token(Token::Type::RCURLY); break;
			case '_': token(Token::Type::UNDERSCORE); break;
			case ';': token(Token::Type::SEMI_COLON); break;
			case '^': token(Token::Type::REFERENCE); break;
			case ',': token(Token::Type::COMMA); break;

			case '&': decide(Token::Type::BAND, Token::Type::LAND); break;
			case '|': decide(Token::Type::BOR, Token::Type::LOR); break;
			case '!': decide(Token::Type::BANG, Token::Type::NEQ); break;
			case '=': decide(Token::Type::ASSIGN, Token::Type::EQUALS); break;
			case ':': decide(Token::Type::COLON, Token::Type::QUICK_ASSIGN); break;

			case '.': decide(Token::Type::DOT, Token::Type::DOUBLEDOT, Token::Type::TRIPLEDOT); break;
			case '>': decide(Token::Type::GREATER, Token::Type::GEQ, Token::Type::RSHIFT); break;
			case '<': decide(Token::Type::LESS, Token::Type::LEQ, Token::Type::LSHIFT); break;

			case '/': do_comment(); break;

			default: {
				if (is_letter(current)) {
					do_word(current);
				}
				else if (is_digit(current)) {
					do_number(current);
				}else if (is_string(current)){
					do_string(current);
				}
			}
		}
	}

	token(Token::Type::END);
	return TokenList(tokens);
}

void Lexer::token(Token::Type type) {
	token(type, "");
}

void Lexer::token(Token::Type type, std::string value) {
	Token tok;
	tok.index = this->indexSavePoint;
	tok.line = this->lineSavePoint;
	tok.length = this->index - this->indexSavePoint;
	tok.type = type;
	tok.value = value;
	tokens.push_back(tok);
	resetSavePoint();
}

void Lexer::decide(Token::Type t1, Token::Type t2){
	// all double tokens end with '=' which is convenient!
	if (!end() && peek() == '=') {
		next();
		return token(t2);
	}
	if (!end() && prev()==peek()) {
		next();
		return token(t2);
	}
	else 
		token(t1);

}

// @TODO this doesn't work
void Lexer::decide(Token::Type t1, Token::Type t2, Token::Type t3){
	if (!end() && peek() == '=') {
		advance(1);
		return token(t2);
	}
	// e.g. >> or ||
	if (!end() && prev() == peek()) {
		advance(1);
		return token(t3);
	}
}

void Lexer::skip_whitespace(){
	while (peek() == ' ' || peek() == '\t') {
		next();
		resetSavePoint();
	}
}

void Lexer::resetSavePoint() {
	indexSavePoint = index;
	lineSavePoint = line;
}

u8 Lexer::consume(char c) {
	if (peek() == c) {
		next();
		return 1;
	}
	return 0;
}

char Lexer::prev(){
	return peek(-1);
}

char Lexer::peek(){
	return src.at(current);
}

char Lexer::peek(u32 amount) {
	return src.at(current + amount);
}

char Lexer::peek_ahead() {
	return src.at(current + 1);
}

char Lexer::next(){
	return advance(1);
}

char Lexer::advance(u32 amount){
	index +=amount;
	char c = src.at(current);
	current += amount;
	return c;
}

u8 Lexer::end() {
	return current > src.size()-1 || src.size()==0;
}

u8 Lexer::is_letter(char c){
	return isalpha(c);
}

u8 Lexer::is_digit(char c){
	return isdigit(c);
}

u8 Lexer::is_string(char c) {
	return c == '"' || c == '\'';
}


u8 Lexer::check_keyword(std::string rest, Token::Type t) {
	log("checking keyword src len {}", src.length());
	int i = 0;
	for (i; i < rest.size() && !end(); i++) {
		log("checking: {}, current: {}", peek(i), current+i);
		if (peek(i) != rest.at(i))
			return 0;
	}
	if (!(current + i > src.length() - 1) && (is_letter(peek(i+1)) || is_digit(peek(i+1)) || peek(i+1) == '_'))
		return 0;
	advance((u32)rest.size());
	token(t);
	return 1;
}

void Lexer::do_word(char start){

	u8 found_keyword = 0;

	switch (start) {
		case 'a': {
			found_keyword = check_keyword("s", Token::Type::AS); break;
			if (!found_keyword) found_keyword = check_keyword("nd", Token::Type::LAND);
		}
		case 'b': found_keyword = check_keyword("reak", Token::Type::BREAK); break;
		case 'c': found_keyword = check_keyword("ontinue", Token::Type::CONTINUE); break;
		case 'f': {
			found_keyword = check_keyword("or", Token::Type::FOR);
			if(!found_keyword) found_keyword = check_keyword("alse", Token::Type::FLSE);
			break;
		}
		case 'i': {
			found_keyword = check_keyword("f", Token::Type::IF); 
			if (!found_keyword) found_keyword = check_keyword("nterface", Token::Type::INTERFACE);
			if (!found_keyword) found_keyword = check_keyword("n", Token::Type::INN);
			if (!found_keyword) found_keyword = check_keyword("nclude", Token::Type::INCLUDE);
			if (!found_keyword) found_keyword = check_keyword("mport", Token::Type::IMPORT);
			break;
		}
		case 'o': {
			found_keyword = check_keyword("r", Token::Type::LOR); break;
		}
		case 'r': {
			found_keyword = check_keyword("eturn", Token::Type::RETURN);
			if (!found_keyword) found_keyword = check_keyword("un", Token::Type::RUN); 
			break;
		}
		case 't': found_keyword = check_keyword("rue", Token::Type::TRU); break;
		case 'x': found_keyword = check_keyword("or", Token::Type::LAND); break;
	}

	if (!found_keyword) {
		// get the first word character
		std::stringstream ss;
		ss << start;
		while (!end() && (is_letter(peek()) || is_digit(peek()) || peek() == '_'))
			ss << next();
		token(Token::Type::IDENTIFIER, ss.str());
	}
}

void Lexer::do_number(char start){
	std::stringstream ss;
	ss << start;
	while (!end() && (is_digit(peek()) || peek() == '.' || peek() == '_'))
		ss << next();
	token(Token::Type::NUMBER, ss.str());
}

void Lexer::do_string(char start) {
	std::stringstream ss;
	while (!end() && peek() != start)
		ss << next();
	// consume past the delimiter
	next();
	token(Token::Type::STRING, ss.str());
}

void Lexer::do_comment(){
}

void Lexer::do_documentation(){
}