#include "parser.h"


std::shared_ptr<AST> Parser::parse() {
	auto program = ProgramAST();
	// @TODO ignore newlines when parsing stuff
	while (peek().type != Token::Type::END)	
		program.stmts.push_back(parse_stmt());
	this->root_ast = std::make_shared<ProgramAST>(program);
	return this->root_ast;
}

std::shared_ptr<AST> Parser::parse_stmt(){
	std::shared_ptr<AST> stmt = std::make_shared<ErrorAST>();
	switch (peek().type) {
		case Token::Type::LCURLY: {
			stmt = parse_stmt_block();
			break;
		}
		case Token::Type::IDENTIFIER: {
			// if we are dealing with an identifier, we could be doing various operations
			switch (peek(1).type) {
				case Token::Type::COLON: stmt = parse_define(); break;
				default: break;
			}
			break;
		}; // @TODO how do we know we are doing an assignment or expression?
	default: break;
	}
	
	
	// we use ; when we want multiple statements on one line
	if (peek().type == Token::Type::SEMI_COLON)
		next();
	else
		consume(Token::Type::NEWLINE, "newline required as statement delimiter");
	return stmt;
}

// a stmt block is simply a list of statements.
// enter with { and exit with }
std::shared_ptr<AST> Parser::parse_stmt_block() {
	log("parsing statement block");

	auto stmt_block = StmtBlockAST();

	consume(Token::Type::LCURLY, "'{' expected");

	while (!end_of_block()) {
		stmt_block.stmts.push_back(parse_stmt());
	}

	consume(Token::Type::RCURLY, "'}' expected");

	return std::make_shared<StmtBlockAST>(stmt_block);

}

u8 Parser::end_of_block() {
	return end() || (peek().type == Token::RCURLY || peek().type == Token::END);
}

std::shared_ptr<AST> Parser::parse_expression() {
	return std::make_shared<ExpressionAST>();
}

std::shared_ptr<AST> Parser::parse_define() {
	log("parsing define");
	auto identifier = next().value;
	consume(Token::COLON, ": expected after identifier for definition");
	// @TODO get the type
	switch (peek().type) {
		case Token::Type::INTERFACE: break;
		case Token::Type::IDENTIFIER: break; // @TODO on pass 2 we need to check this type is actually valid and has been defined
		default: break; // @TODO this is invalid return an error
	}
	return std::make_shared<AST>();
}

std::shared_ptr<AST> Parser::parse_quick_define() {
	log("parsing define");
	auto identifier = next().value;
	consume(Token::QUICK_ASSIGN, ":= expected after identifier for quick define");
	// @TODO infer the type
	auto assignment_expression = parse_expression();
	auto type = infer_type(assignment_expression);
	return std::make_shared<AST>();
}

u8 Parser::end() {
	return current >= tokens.size();
}

Token Parser::consume(Token::Type type, const std::string err_msg){
	if (peek().type != type)
		// we need to insert an error AST here
		log(err_msg);
	return next();
}

Token Parser::prev() {
	return peek(-1);
}

Token Parser::peek() {
	return tokens.at(current);
}

Token Parser::peek(u32 amount) {
	return tokens.at(current + amount);
}

Token Parser::peek_ahead() {
	return tokens.at(current + 1);
}

Token Parser::next() {
	return advance(1);
}

Token Parser::advance(u32 amount) {
	Token t = tokens.at(current);
	current += amount;
	return t;
}