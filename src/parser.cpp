#include "parser.h"
#include "compiler.h"

Parser::Parser(){}

Parser::Parser(TokenList& tokens, Compiler* compiler){
	this->tokens = tokens.tokens;
	this->compiler = compiler;
}


std::shared_ptr<AST> Parser::parse() {
	this->root_ast = std::make_shared<AST>();
	auto program = ProgramAST();
	// @TODO ignore newlines when parsing stuff
	while (!end() && peek().type != Token::Type::END)	
		program.stmts.push_back(parse_stmt());
	this->root_ast = std::make_shared<ProgramAST>(program);
	return this->root_ast;
}

std::shared_ptr<AST> Parser::parse_stmt(){
	std::shared_ptr<AST> stmt = std::make_shared<ErrorAST>();
	switch (peek().type) {
		case Token::Type::RETURN:{
			next();
			stmt = std::make_shared<StmtReturnAST>();
			break;
		}
		case Token::Type::CONTINUE: {
			next();
			stmt = std::make_shared<StmtContinueAST>();
			break;
		}
		case Token::Type::BREAK: {
			next();
			stmt = std::make_shared<StmtBreakAST>();
			break;
		}
		case Token::Type::IF: {
			stmt = parse_if();
			break;
		}
		case Token::Type::FOR: {
			stmt = parse_if();
			break;
		}
		case Token::Type::LCURLY: {
			stmt = parse_stmt_block();
			break;
		}
		case Token::Type::IDENTIFIER: {
			// if we are dealing with an identifier, we could be doing various operations
			switch (peek(1).type) {
				case Token::Type::COLON: stmt = parse_define(); break;
				default: stmt = parse_expression(); break;
			}
			break;
		}; // @TODO how do we know we are doing an assignment or expression?
		default: stmt = parse_expression(); break;
	}
	
	// we use ; when we want multiple statements on one line
	if (!(consume(Token::Type::NEWLINE) || consume(Token::Type::SEMI_COLON)))
		// @TODO this prev() stuff should probably be done with a function
		compiler->error_handler.error("expected ; or newline as statement delimiter", 
			prev().index+prev().length+1, prev().line, prev().index + prev().length + 1, prev().line);

	return stmt;
}

std::shared_ptr<AST> Parser::parse_if(){
	StmtIfAST if_ast;
	consume(Token::Type::IF);
	auto if_cond = parse_expression();
	auto if_stmt = parse_stmt();
	if_ast.if_cond = if_cond;
	if_ast.if_stmt = if_stmt;
	return std::make_shared<StmtIfAST>(if_ast);
}

std::shared_ptr<AST> Parser::parse_for(){
	StmtLoopAST loop_ast;
	consume(Token::Type::FOR);
	return std::make_shared<StmtLoopAST>(loop_ast);
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




std::shared_ptr<AST> Parser::parse_expression() { 
	return parse_assign();
}

std::shared_ptr<AST> Parser::parse_assign() { 
	auto higher_precedence = parse_lor();
	if (consume(Token::ASSIGN)) {
		auto assign_value = parse_lor();
		// we need to check if we are setting a variable, or an interface member
		switch (higher_precedence->type()) {
			case AST::Type::EXPR_VAR: {
				auto variable_token = std::dynamic_pointer_cast<ExprVarAST>(higher_precedence);
				return std::make_shared<StmtAssignAST>(variable_token->identifier, assign_value);
			}
			case AST::Type::EXPR_INTER_GET: {
				auto member_get = std::dynamic_pointer_cast<ExprInterfaceGetAST>(higher_precedence);
				auto interface_value = member_get->value;
				auto member_token = member_get->member;
				return std::make_shared<StmtInterfaceAssignAST>(interface_value, member_token, assign_value);
			}
			default: {
				//@TODO use AST position information
				compiler->error_handler.error("cannot assign to lhs", 0, 1, 0, 1);
				break;
			}
		}
	}
	return higher_precedence;
}

std::shared_ptr<AST> Parser::parse_lor() { 
	auto higher_precedence = parse_land();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_land() {
	auto higher_precedence = parse_bor();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_bor() {
	auto higher_precedence = parse_band();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_band() {
	auto higher_precedence = parse_eq();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_eq() {
	auto higher_precedence = parse_comp();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_comp() {
	auto higher_precedence = parse_shift();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_shift() {
	auto higher_precedence = parse_pm();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_pm() {
	auto higher_precedence = parse_mdmr();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_mdmr() {
	auto higher_precedence = parse_un();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_un() {
	auto higher_precedence = parse_cast();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_cast() {
	auto higher_precedence = parse_call();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_call() {
	auto higher_precedence = parse_single();
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_single(){ 


	// @TODO implement groups e.g. (1+2) + (1+3)
	log("parsing single!");
	auto t = next();
	switch (t.type) {
		case Token::Type::IDENTIFIER: {
			return std::make_shared<ExprVarAST>(t);
		}
		case Token::Type::NUMBER: {
			ExprLiteralAST lit_ast;
			return std::make_shared<ExprLiteralAST>();
			break;
		}
		case Token::Type::STRING: {
			break;
		}
		case Token::Type::TRU: {
			break;
		}
		case Token::Type::FLSE: {
			break;
		}
	}
	// @TODO error here
	return nullptr; 

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

u8 Parser::consume(Token::Type type) {
	if (peek().type != type)
		return 0;
	next();
	return 1;
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