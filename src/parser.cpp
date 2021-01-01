#include "parser.h"
#include "compiler.h"

Parser::Parser(){}

Parser::Parser(TokenList& tokens, CompilationUnit* unit){
	this->tokens = tokens.tokens;
	this->unit = unit;
	this->sym_table = std::make_shared<SymTable<Type>>();
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
	u8 requires_delim = 0; // e.g. if 1 {} doesn't require a delimiter
	do_newline();
	std::shared_ptr<AST> stmt = std::make_shared<ErrorAST>();
	switch (peek().type) {
		case Token::Type::HASH: {
			requires_delim = 0;
			stmt = parse_directive();
			break;
		}
		case Token::Type::RETURN:{
			requires_delim = 1;
			next();
			stmt = std::make_shared<StmtReturnAST>();
			break;
		}
		case Token::Type::CONTINUE: {
			requires_delim = 1;
			next();
			stmt = std::make_shared<StmtContinueAST>();
			break;
		}
		case Token::Type::BREAK: {
			requires_delim = 1;
			next();
			stmt = std::make_shared<StmtBreakAST>();
			break;
		}
		case Token::Type::IF: {
			stmt = parse_if();
			break;
		}
		case Token::Type::FOR: {
			stmt = parse_for();
			break;
		}
		case Token::Type::LCURLY: {
			stmt = parse_stmt_block();
			break;
		}
		case Token::Type::IDENTIFIER: {
			requires_delim = 1;
			// if we are dealing with an identifier, we could be doing various operations
			switch (peek(1).type) {
				case Token::Type::COLON:
				case Token::Type::QUICK_ASSIGN: stmt = parse_define(); break;
				default: stmt = parse_expression(); break;
			}
			break;
		}; // @TODO how do we know we are doing an assignment or expression?
		default: {
			requires_delim = 1;
			stmt = parse_expression();
			break;
		}
	}
	// @TODO it doesnt work if we dont have a newline at the end
	// we use ; when we want multiple statements on one line
	if (requires_delim && !(consume(Token::Type::NEWLINE) || consume(Token::Type::SEMI_COLON) || consume(Token::Type::END))) {
		// @TODO this prev() stuff should probably be done with a function
		unit->error_handler.error("expected ; or newline as statement delimiter",
			prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
		return std::make_shared<ErrorAST>();
	}
	do_newline();
	return stmt;
}

void Parser::do_newline() {
	while (expect(Token::NEWLINE)) next();
}

std::shared_ptr<AST> Parser::parse_directive() {
	consume(Token::HASH);
	switch (next().type) {
		case Token::Type::RUN: {
				// create a new compilation unit to JIT the next statement
				auto stmt_to_jit = parse_stmt();
				kng_log("JITing {}", stmt_to_jit->to_json());
				// tmp
				return std::make_shared<ErrorAST>();
				break;
		}
		case Token::Type::INCLUDE: {
			//// @TODO allow expressions to be the filename so we can support #include (#run win ? "windows.k" : "unix.k")
			//auto to_include = parse_expression();
			//if (!(to_include->type() == AST::Type::EXPR_LIT)){
			//	unit->error_handler.error("expected string as filename to include",
			//		prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
			//	return std::make_shared<ErrorAST>();
			//}

			if (!expect(Token::Type::STRING)) {
				unit->error_handler.error("expected string as filename to include",
					prev().index, prev().line, prev().index + prev().length, prev().line);
				return std::make_shared<ErrorAST>();
			}
			auto path = next();
			auto valid_include = unit->importer->valid_include_path(unit->compile_file.file_path, path.value);
			if (!(valid_include==Importer::DepStatus::OK)) {
				std::string problem;
				switch (valid_include) {
				case Importer::DepStatus::CYCLIC_DEP: problem = "include path is causing a cyclic dependency"; break;
				}
				unit->error_handler.error(problem,
					prev().index, prev().line, prev().index + prev().length, prev().line);
				return std::make_shared<ErrorAST>();
			}
			// finally if we have't already included this file then include it!
			if (!unit->importer->already_included(unit->compile_file.file_path, path.value)) {
				// @TODO_URGENT add a way for the compilers Importer to track which CompilationUnits exist so we can get some stats
				auto compilation_unit = unit->importer->include(unit->compile_file.file_path, path.value);
				auto ast = compilation_unit->compile_to_ast();
			}
			break;
		}
	}
	return parse_stmt();
}

std::shared_ptr<AST> Parser::parse_if(){
	StmtIfAST if_ast;
	consume(Token::Type::IF);
	auto if_cond = parse_expression();
	auto if_stmt = parse_stmt();

	kng_log("parsed if stmt, peeking {}", peek().to_json());
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


	auto stmt_block = StmtBlockAST();

	consume(Token::Type::LCURLY, "'{' expected");
	do_newline();
	sym_table->enter_scope();
	while (!end_of_block()) {
		stmt_block.stmts.push_back(parse_stmt());
	}
	do_newline();
	sym_table->pop_scope();
	consume(Token::Type::RCURLY, "'}' expected");

	return std::make_shared<StmtBlockAST>(stmt_block);

}

u8 Parser::end_of_block() {
	return end() || (peek().type == Token::RCURLY || peek().type == Token::END);
}

u8 Parser::expecting_type() {
	return 
		expect(Token::Type::U0)
		|| expect(Token::Type::U8)
		|| expect(Token::Type::U16)
		|| expect(Token::Type::U32)
		|| expect(Token::Type::S32)
		|| expect(Token::Type::S64)
		|| expect(Token::Type::F32)
		|| expect(Token::Type::F64)
		|| expect(Token::Type::CHAR)
		|| expect(Token::Type::IDENTIFIER);
}

Type Parser::parse_type(){
	switch (next().type) {
		case Token::Type::U0: return Type(Type::Types::U0);
		case Token::Type::U8: return Type(Type::Types::U8);
		case Token::Type::U16: return Type(Type::Types::U16);
		case Token::Type::U32: return Type(Type::Types::U32);
		case Token::Type::S32: return Type(Type::Types::S32);
		case Token::Type::S64: return Type(Type::Types::S64);
		case Token::Type::F32: return Type(Type::Types::F32);
		case Token::Type::F64: return Type(Type::Types::F64);
		case Token::Type::CHAR: return Type(Type::Types::CHAR);
		case Token::Type::IDENTIFIER: return Type(Type::Types::INTERFACE);
	}
}

std::shared_ptr<AST> Parser::parse_define() {
	StmtDefineAST define_ast;
	define_ast.identifier = next();
	if(consume(Token::Type::QUICK_ASSIGN)){
		define_ast.requires_type_inference = 1;
		// @TODO check an expression exists???
		define_ast.value = parse_expression();
		define_ast.is_initialised = 1;
	}
	else if(consume(Token::Type::COLON)){
		if (!expecting_type()) {
			unit->error_handler.error("expected type after :",
				prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
			return std::make_shared<ErrorAST>();
		}
		define_ast.value = NULL;
		define_ast.define_type = parse_type();
		if (consume(Token::Type::ASSIGN)){
			define_ast.value = parse_expression();
			define_ast.is_initialised = 1;
		}
	}
	return std::make_shared<StmtDefineAST>(define_ast);
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
			case AST::ASTType::EXPR_VAR: {
				auto variable_token = std::dynamic_pointer_cast<ExprVarAST>(higher_precedence);
				return std::make_shared<StmtAssignAST>(variable_token->identifier, assign_value);
			}
			case AST::ASTType::EXPR_INTER_GET: {
				auto member_get = std::dynamic_pointer_cast<ExprInterfaceGetAST>(higher_precedence);
				auto interface_value = member_get->value;
				auto member_token = member_get->member;
				return std::make_shared<StmtInterfaceAssignAST>(interface_value, member_token, assign_value);
			}
			default: {
				//@TODO use AST position information
				unit->error_handler.error("cannot assign to lhs", 0, 1, 0, 1);
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
			lit_ast.t = Type(Type::Types::S32, 0);
			lit_ast.v.v.as_s32 = stoi(t.value);
			return std::make_shared<ExprLiteralAST>(lit_ast);
			break;
		}
		case Token::Type::STRING: {
			// @TODO arrays
			return std::make_shared<ExprLiteralAST>();
			break;
		}
		case Token::Type::TRU: {
			ExprLiteralAST lit_ast;
			lit_ast.t = Type(Type::Types::U8, 0);
			lit_ast.v.v.as_u8 = 1;
			return std::make_shared<ExprLiteralAST>(lit_ast);
		}
		case Token::Type::FLSE: {
			ExprLiteralAST lit_ast;
			lit_ast.t = Type(Type::Types::U8, 0);
			lit_ast.v.v.as_u8 = 0;
			return std::make_shared<ExprLiteralAST>(lit_ast);
		}
		case Token::Type::LPAREN: {
			// parsing fn
			if (consume(Token::Type::RPAREN)) {
				kng_log("parsing fn!");
				ExprFnAST fn_ast;
				FnSignature fn_sig;
				fn_sig.anonymous_identifier = "test_anon_fn";
				if (expecting_type())
					fn_sig.operation_types.push_back(parse_type());
				else
					fn_sig.operation_types.push_back(Type(Type::Types::U0));
				fn_ast.full_type = Type(Type::Types::FN, fn_sig);
				// we need to check if the fn has a body
				if (!consume(Token::Type::SEMI_COLON)) {
					fn_ast.has_body = 1;
					fn_ast.body = parse_stmt();
				}
				return std::make_shared<ExprFnAST>(fn_ast);
			}
			else {
				// either parsing a group or a function
				auto expression = parse_expression();
				if (!consume(Token::Type::RPAREN)) {
					unit->error_handler.error("expected ) as statement delimiter",
						prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
					return std::make_shared<ErrorAST>();
				}
				auto group = ExprGroupAST(expression);
				return std::make_shared<ExprGroupAST>(group);
			}
		}
		case Token::Type::LBRACKET: return parse_interface();
	}
	// @TODO error here
	unit->error_handler.error("unexpected code?",
		prev().index + prev().length + 1, prev().line, prev().index + prev().length + 1, prev().line);
	return std::make_shared<ErrorAST>(); 
}


std::shared_ptr<AST> Parser::parse_fn(){
	return NULL;
}

std::shared_ptr<AST> Parser::parse_interface() {


	ExprInterfaceAST expr_interface;

	expr_interface.anonymous_name = "tmp_anon_interface_name";

	consume(Token::LBRACKET);

	// a list of definition
	while (!consume(Token::RBRACKET)) {
		parse_stmt();
	}
	return std::make_shared<ExprInterfaceAST>(expr_interface);
}

