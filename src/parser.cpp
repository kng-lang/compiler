#include "parser.h"
#include "compiler.h"

Parser::Parser(){}

Parser::Parser(TokenList& tokens, Compiler* compiler){
	this->tokens = tokens.tokens;
	this->compiler = compiler;
	this->sym_table = std::make_shared<SymTable>();
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

	// consume empty newlines
	do_newline();

	std::shared_ptr<AST> stmt = std::make_shared<ErrorAST>();
	switch (peek().type) {
		case Token::Type::HASH: {
			stmt = parse_directive();
			break;
		}
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
	
	// @TODO it doesnt work if we dont have a newline at the end
	// we use ; when we want multiple statements on one line
	if (!(consume(Token::Type::NEWLINE) || consume(Token::Type::SEMI_COLON) || consume(Token::Type::END))) {
		// @TODO this prev() stuff should probably be done with a function
		compiler->error_handler.error("expected ; or newline as statement delimiter",
			prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
		return std::make_shared<ErrorAST>();
	}
	return stmt;
}

void Parser::do_newline() {
	while (expect(Token::Type::NEWLINE))
		next();
}

std::shared_ptr<AST> Parser::parse_directive() {
	consume(Token::HASH);
	// all directives need an identifier
	if (!expect(Token::Type::IDENTIFIER)) {
		// @TODO err here
		return NULL;
	}
	std::string s = next().value;
	if(!s.compare("run")){
		log("#run {}", peek().to_json());
		// @TODO do
	}else if (!s.compare("import")) {
		if (expect(Token::Type::STRING)) {
			auto s = next();
			log("importing {}", s.value);
		}
		else {
			compiler->error_handler.error("expected string as filename to import",
				prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
			return std::make_shared<ErrorAST>();
		}
	}
	else if (!s.compare("include")) {
		log("#include");
		// @TODO do
	}
	return parse_stmt();
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

	sym_table = sym_table->enter_scope(sym_table);
	
	while (!end_of_block()) {
		stmt_block.stmts.push_back(parse_stmt());
	}
	
	sym_table = sym_table->pop_scope();

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
	auto higher_precedence = parse_pattern();
	if (consume(Token::ASSIGN)) {
		auto assign_value = parse_pattern();
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
				return std::make_shared<ErrorAST>();
			}
		}
	}
	return higher_precedence;
}

// e.g. x = 1, a, 2, b
// @TODO we need to be able to assign to patterns e.g. a, b, c = 1, 2, 3 so we need to swap the precedence
std::shared_ptr<AST> Parser::parse_pattern() {
	auto higher_precedence = parse_lor();
	// @TODO figure out how to check a pattern when no comma is used
	if (consume(Token::Type::COMMA)) {
		std::vector<std::shared_ptr<AST>> asts;
		asts.push_back(higher_precedence);
		while (!expect(Token::Type::END)) {
			asts.push_back(parse_lor());
			// consume comma if it exists e.g. a, b, c or a b c
			consume(Token::Type::COMMA);
		}
		auto pattern = ExprPatternAST(asts);
		return std::make_shared<ExprPatternAST>(pattern);
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
	if(expect(Token::Type::PLUS) || expect(Token::Type::MINUS)){
		auto op = next();
		auto rhs = parse_pm();
		auto pm = ExprBinAST(higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(pm);
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_mdmr() {
	auto higher_precedence = parse_un();
	// @TODO need to lex /, // and ///
	if (expect(Token::Type::STAR) || expect(Token::Type::DIV) || expect(Token::Type::MOD)) {
		auto op = next();
		auto rhs = parse_mdmr();
		auto mdmr = ExprBinAST(higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(mdmr);
	}
	return higher_precedence;
}

std::shared_ptr<AST> Parser::parse_un() {
	auto higher_precedence = parse_cast();
	if (expect(Token::Type::REFERENCE) || expect(Token::Type::BANG)) {
		auto op = next();
		auto ast = parse_un();
#define LEFT 0
#define RIGHT 1
		auto un = ExprUnAST(op, ast, LEFT);
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_cast() {
	auto higher_precedence = parse_call();
	if (consume(Token::AS)) {
		auto op = next();
		// @TODO do type stuff?
		auto typ = next();
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_call() {
	auto higher_precedence = parse_single();
	// check if next is var args
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_single(){ 
	// @TODO implement groups e.g. (1+2) + (1+3)
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
			return std::make_shared<ExprLiteralAST>();
			break;
		}
		case Token::Type::TRU: {
			return std::make_shared<ExprLiteralAST>();
			break;
		}
		case Token::Type::FLSE: {
			return std::make_shared<ExprLiteralAST>();
			break;
		}
		case Token::Type::LPAREN: {
			auto expression = parse_expression();
			if (!consume(Token::Type::RPAREN)) {
				compiler->error_handler.error("expected ) as statement delimiter",
					prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
				return std::make_shared<ErrorAST>();
			}
			auto group = ExprGroupAST(expression);
			return std::make_shared<ExprGroupAST>(group);
		}
	}
	// @TODO error here
	compiler->error_handler.error("unexpected code?",
		prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
	return std::make_shared<ErrorAST>(); 
}