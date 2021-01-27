/*
James Clarke - 2021
*/

#include "parser.h"
#include "compiler.h"


Parser::Parser(){}

Parser::Parser(TokenList& tokens, CompilationUnit* unit){
	this->tokens = tokens.m_tokens;
	this->m_unit = unit;
}


std::shared_ptr<AST> Parser::parse() {
	this->m_root_ast = std::make_shared<AST>();
	auto program = ProgramAST();
	// @TODO ignore newlines when parsing stuff
	while (!end() && peek().m_type != Token::Type::END)	
		program.stmts.push_back(parse_stmt());
	this->m_root_ast = std::make_shared<ProgramAST>(program);
	return this->m_root_ast;
}

std::shared_ptr<AST> Parser::parse_stmt(){
	reset_flags();
	// consume empty newlines
	m_requiring_delimiter = 0; // e.g. if 1 {} doesn't require a delimiter
	std::shared_ptr<AST> stmt = std::make_shared<ErrorAST>(peek().m_position);
	switch (peek().m_type) {
		case Token::Type::DIRECTIVE: {
			stmt = parse_directive();
			m_requiring_delimiter = 1;
			break;
		}
		case Token::Type::RETURN:{
			StmtReturnAST r;
			m_requiring_delimiter = 1;
			next();
			if (expecting_expr())
				r.value = parse_expression();
			stmt = std::make_shared<StmtReturnAST>(r);
			break;
		}
		case Token::Type::CONTINUE: {
			m_requiring_delimiter = 1;
			next();
			stmt = std::make_shared<StmtContinueAST>();
			break;
		}
		case Token::Type::BREAK: {
			m_requiring_delimiter = 1;
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
		case Token::Type::DEFER: {
			m_requiring_delimiter = 1;
			stmt = parse_defer();
			break;
		}
		case Token::Type::LCURLY: {
			stmt = parse_stmt_block();
			break;
		}
		case Token::Type::IDENTIFIER: {
			m_requiring_delimiter = 1;
			// if we are dealing with an identifier, we could be doing various operations
			switch (peek(1).m_type) {
				case Token::Type::COLON:
				case Token::Type::QUICK_ASSIGN: stmt = parse_define(); break;
				default: stmt = parse_expression_stmt(); break;
			}
			break;
		}; // @TODO how do we know we are doing an assignment or expression?
		default: {
			parse_expression();
			m_requiring_delimiter = 1;
			break;
		}
	}
	// @TODO it doesnt work if we dont have a newline at the end
	// we use ; when we want multiple statements on one line
	if (m_requiring_delimiter && !(consume(Token::Type::SEMI_COLON) || consume(Token::Type::END))) {
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL,
			Error::Type::MISSING_DELIMITER,
			"expected ; at the end of the statment",
			prev().m_position
		);
		return std::make_shared<ErrorAST>(prev().m_position);

	}
	return stmt;
}

std::shared_ptr<AST> Parser::parse_directive() {
	consume(Token::DIRECTIVE);
	switch (next().m_type) {
	    case Token::Type::DOC: break;
		case Token::Type::RUN: {
				// create a new compilation unit to JIT the next statement
				auto stmt_to_jit = parse_stmt();
				kng_log("JITing {}", stmt_to_jit->to_json());
				// tmp
				return std::make_shared<ErrorAST>(prev().m_position);
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

			if (!expect(Token::Type::STRING_LIT)) {
				m_unit->m_error_handler.error(
					Error::Level::CRITICAL,
					Error::Type::MISSING_DELIMITER,
					"expected string as filename to include",
					prev().m_position
				);
				return std::make_shared<ErrorAST>(prev().m_position);
			}
			auto path = next();
			auto valid_include = m_unit->m_importer->valid_include_path(path.m_value);
			if (!valid_include){
				m_unit->m_error_handler.error(
					Error::Level::CRITICAL,
					Error::Type::MISSING_DELIMITER,
					"include path was invalid (TODO say why)",
					prev().m_position
				);
				return std::make_shared<ErrorAST>(prev().m_position);
			}
			auto compilation_unit = m_unit->m_importer->include_file(path.m_value);
			auto ast = compilation_unit->compile_to_ast();
			return std::make_shared<ExprIncludedAST>(prev().m_position, ast);
		}
	}
	return parse_stmt();
}

std::shared_ptr<AST> Parser::parse_if(){
	StmtIfAST if_ast;
	consume(Token::Type::IF);
	auto if_cond = parse_expression();
	auto if_stmt = parse_stmt();
	if(consume(Token::Type::ELSE)){
		if_ast.has_else = 1;
		if_ast.else_stmt = parse_stmt();
	}
	if_ast.if_cond = if_cond;
	if_ast.if_stmt = if_stmt;
	m_requiring_delimiter = 0;
	return std::make_shared<StmtIfAST>(if_ast);
}

std::shared_ptr<AST> Parser::parse_for(){
	StmtLoopAST loop_ast;
	consume(Token::Type::FOR);
	// if there is no condition, then the for loop is infinite
	if (expecting_expr()) {
		loop_ast.loop_type = StmtLoopAST::LoopType::INF;
	}
	else {
		kng_assert(false,  "not implemented yet!");
	}
	loop_ast.body = parse_stmt();
	m_requiring_delimiter = 0;
	return std::make_shared<StmtLoopAST>(loop_ast);
}

std::shared_ptr<AST> Parser::parse_defer() {
	consume(Token::Type::DEFER);
	auto expression = parse_expression();
	return std::make_shared<StmtDeferAST>(prev().m_position.merge(expression->m_position), expression);
}

// a stmt block is simply a list of statements.
// enter with { and exit with }
std::shared_ptr<AST> Parser::parse_stmt_block() {

	auto stmt_block = StmtBlockAST();

	consume(Token::Type::LCURLY, "'{' expected");
	if (!consume(Token::Type::RCURLY)) {
		while (!end_of_block()) {
			stmt_block.stmts.push_back(parse_stmt());
		}
		consume(Token::Type::RCURLY, "'}' expected");
	}

	m_requiring_delimiter = 0;
	return std::make_shared<StmtBlockAST>(stmt_block);

}

u8 Parser::end_of_block() {
	return end() || (peek().m_type == Token::RCURLY || peek().m_type == Token::END);
}

u8 Parser::expecting_type() {
	// @TODO this should be done as a numeric check e.g. (u32)expect() > 12 etc
	return 
		expect(Token::Type::U0)
		|| expect(Token::Type::U8)
		|| expect(Token::Type::S8)
		|| expect(Token::Type::U16)
		|| expect(Token::Type::S16)
		|| expect(Token::Type::U32)
		|| expect(Token::Type::S32)
		|| expect(Token::Type::S64)
		|| expect(Token::Type::F32)
		|| expect(Token::Type::F64)
		|| expect(Token::Type::CHAR)
		|| expect(Token::Type::INTERFACE)
		|| expect(Token::Type::IDENTIFIER)
		|| expect(Token::Type::STRING)
		|| expect(Token::Type::POINTER)
		|| expect(Token::Type::FN)
		|| expect(Token::Type::TYPE);
}

u8 Parser::expecting_expr(){
	// an expression can be the following (same as parsing a single!)
	// string, number, identifier, group, true, false, {
	return
		expect(Token::Type::STRING_LIT)
		|| expect(Token::Type::TYPE)
		|| expect(Token::Type::NUMBER)
		|| expect(Token::Type::IDENTIFIER)
		|| expect(Token::Type::LPAREN)
		|| expect(Token::Type::TRU)
		|| expect(Token::Type::FLSE)
		|| expect(Token::Type::LBRACKET) // the problem is { can start a stmt and an expression...
		|| expect(Token::Type::LCURLY);
}

u8 Parser::expecting_def() {
	return expect(Token::Type::IDENTIFIER) && expect(Token::Type::COLON, 1);
}

Type Parser::parse_type() {
	Type t;
	u8 ptr_indirection=0;
	while (consume(Token::Type::POINTER)){
		ptr_indirection += 1;
	}

	auto token = next();
	switch (token.m_type) {
	case Token::Type::U0: t = Type::create_basic(Type::Types::U0); break;
	case Token::Type::U8: t = Type::create_basic(Type::Types::U8); break;
	case Token::Type::S8: t = Type::create_basic(Type::Types::S8); break;
	case Token::Type::U16: t = Type::create_basic(Type::Types::U16); break;
	case Token::Type::S16: t = Type::create_basic(Type::Types::S16); break;
	case Token::Type::U32: t = Type::create_basic(Type::Types::U32); break;
	case Token::Type::S32: t = Type::create_basic(Type::Types::S32); break;
	case Token::Type::S64: t = Type::create_basic(Type::Types::S64); break;
	case Token::Type::F32: t = Type::create_basic(Type::Types::F32); break;
	case Token::Type::F64: t = Type::create_basic(Type::Types::F64); break;
	case Token::Type::CHAR: t = Type::create_basic(Type::Types::CHAR); break;
	case Token::Type::INTERFACE: t = Type::create_basic(Type::Types::INTERFACE); break;
	case Token::Type::IDENTIFIER: {
		// !@TODO here, we need the typechecker to resolve the full type of the interface
		t = Type::create_interface(token);
		//.t.m_interface_identifier = prev();
		break;
	}
	case Token::Type::STRING: t = Type(Type::Types::STRING); break;
	case Token::Type::TYPE: t = Type(Type::Types::TYPE); break;
	// TODO parsing fns here
	case Token::Type::FN: {
		u8 has_return = 0;
		std::vector<Type> op_types;
		op_types.push_back(Type::create_basic(Type::Types::U0));
		if(!consume(Token::Type::LPAREN))
			m_unit->m_error_handler.error(
				Error::Level::CRITICAL,
				Error::Type::MISSING_DELIMITER,
				"expected ( after fn type decleration",
				prev().m_position
			);
		while (!expect(Token::Type::RPAREN)) {
			if(expecting_type())
				op_types.push_back(parse_type());
		}
		consume(Token::Type::RPAREN);
		if (expecting_type()) {
			has_return = 1;
			op_types.at(0) = parse_type();
		}
		t = Type::create_fn(has_return, op_types);
		break;
	}
	default: {
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL,
			Error::Type::UNKNOWN_TYPE,
			"unknown type when parsing",
			prev().m_position
		);
		return Type::create_basic(Type::Types::UNKNOWN);
	}
	}
	// check if we are dealing with an array
	if (consume(Token::Type::LBRACKET)){
		if (expect(Token::Type::NUMBER)) {
			// known size and stack allocated
			Token size = next();
			t.m_is_arr = 1;
			t.m_arr_length = std::stoi(size.m_value);
		}
		else {
			// unknown size, we need to check during type checking if the array is initialised otherwise it is a pointer
			t.m_is_arr = 1;
			ptr_indirection++;
		}

		if(!consume(Token::Type::RBRACKET)){
			m_unit->m_error_handler.error(
				Error::Level::CRITICAL,
				Error::Type::MISSING_DELIMITER,
				"expected ] after array decleration",
				prev().m_position
			);
			return Type(Type::Types::UNKNOWN);
		}
	}
	t.m_ptr_indirection = ptr_indirection;
	t.m_is_constant = m_parsing_constant_assignment;
	return t;
}

void Parser::reset_flags() {
	m_parsing_constant_assignment = 0;
	m_parsing_variable_assignment = 0;
}

std::shared_ptr<AST> Parser::parse_define() {
	StmtDefineAST define_ast;
	define_ast.identifier = next();
	define_ast.m_position = prev().m_position;
	if (define_ast.identifier.m_value.compare("_") == 0) {
		define_ast.m_is_underscore = 1;
	}
	if(consume(Token::Type::QUICK_ASSIGN)){
		m_parsing_variable_assignment = 1;
		// at this point, we are expecting to parse an expression as without an expression the syntax is invalid

		define_ast.m_requires_type_inference = 1;
		// @TODO check an expression exists???
		define_ast.value = parse_expression();
		define_ast.m_is_initialised = 1;
		m_parsing_variable_assignment = 0;
	}
	else if(consume(Token::Type::COLON)){
		if (expecting_type()) {
			// if we have reached here, we are either dealing with x : u8, x : u8 = 1, x : u8 1
			// variable decleration, variable decleration & assignment or constant decleration
			define_ast.define_type = parse_type();
			define_ast.value = NULL;

			u8 expecting_expression = expecting_expr();
			u8 parsing_constant = expecting_expression;
			define_ast.m_is_constant = parsing_constant;
			m_parsing_constant_assignment = parsing_constant;
			if (expecting_expression || consume(Token::Type::ASSIGN)) {
				m_parsing_variable_assignment = 1;
				define_ast.m_is_initialised = 1;
				define_ast.value = parse_expression();
				m_parsing_variable_assignment = 0;
			}
		}
		else {
			// if we reached here we MUST expect an inferred constant e.g. x : 1
			if (!expecting_expr()) {
				m_unit->m_error_handler.error(
					Error::Level::CRITICAL,
					Error::Type::MISSING_DELIMITER,
					"expected type after :",
					prev().m_position
				);
				return std::make_shared<ErrorAST>(prev().m_position);
			}
			m_parsing_constant_assignment = 1;
			define_ast.m_requires_type_inference = 1;
			define_ast.value = parse_expression();
			define_ast.m_is_initialised = 1;
			define_ast.m_is_constant = 1;
		}
	}
	m_parsing_constant_assignment = 0;
	return std::make_shared<StmtDefineAST>(define_ast);
}


std::shared_ptr<AST> Parser::parse_expression_stmt() {
	return parse_assign();
}

std::shared_ptr<AST> Parser::parse_expression() { 
	return parse_pattern();
}


std::shared_ptr<AST> Parser::parse_assign() { 
	auto higher_precedence = parse_expression();
	if (consume(Token::ASSIGN)) {
		auto assign_value = parse_expression();
		return std::make_shared<StmtAssignAST>(prev().m_position, higher_precedence, assign_value);
	}
	return higher_precedence;
}

// e.g. x = 1, a, 2, b
// @TODO we need to be able to assign to patterns e.g. a, b, c = 1, 2, 3 so we need to swap the precedence
std::shared_ptr<AST> Parser::parse_pattern() {
	auto higher_precedence = parse_lor();
	// @TODO figure out how to check a pattern when no comma is used
	if (expect(Token::Type::COMMA)) {
		std::vector<std::shared_ptr<AST>> values;
		values.push_back(higher_precedence);
		while(consume(Token::Type::COMMA))
			values.push_back(parse_expression());
		auto pattern = ExprPatternAST(higher_precedence->m_position, values);
		return std::make_shared<ExprPatternAST>(pattern);
	}
	return higher_precedence;
}

std::shared_ptr<AST> Parser::parse_lor() { 
	auto higher_precedence = parse_land();
	if (expect(Token::Type::LOR)) {
		auto op = next();
		auto rhs = parse_lor();
		auto pm = ExprBinAST(higher_precedence->m_position.merge(prev().m_position), higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(pm);
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_land() {
	auto higher_precedence = parse_bor();
	if (expect(Token::Type::LAND)) {
		auto op = next();
		auto rhs = parse_land();
		auto pm = ExprBinAST(higher_precedence->m_position.merge(prev().m_position), higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(pm);
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_bor() {
	auto higher_precedence = parse_band();
	if (expect(Token::Type::BOR)) {
		auto op = next();
		auto rhs = parse_bor();
		auto pm = ExprBinAST(higher_precedence->m_position.merge(prev().m_position), higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(pm);
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_band() {
	auto higher_precedence = parse_eq();
	if (expect(Token::Type::BAND)) {
		auto op = next();
		auto rhs = parse_band();
		auto pm = ExprBinAST(higher_precedence->m_position.merge(prev().m_position), higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(pm);
	}
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
	if (expect(Token::Type::LSHIFT) || expect(Token::Type::RSHIFT)) {
		auto op = next();
		auto rhs = parse_shift();
		auto pm = ExprBinAST(higher_precedence->m_position.merge(prev().m_position), higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(pm);
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_pm() {
	auto higher_precedence = parse_mdmr();
	if(expect(Token::Type::PLUS) || expect(Token::Type::MINUS)){
		auto op = next();
		auto rhs = parse_pm();
		auto pm = ExprBinAST(higher_precedence->m_position.merge(prev().m_position), higher_precedence, rhs, op);
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
		auto mdmr = ExprBinAST(higher_precedence->m_position.merge(prev().m_position), higher_precedence, rhs, op);
		return std::make_shared<ExprBinAST>(mdmr);
	}
	return higher_precedence;
}

std::shared_ptr<AST> Parser::parse_un() {
	if (expect(Token::Type::POINTER) || expect(Token::Type::BANG) || expect(Token::Type::BAND) || expect(Token::Type::TYPEOF)) {


		// @TODO optimise by having a for consume(POINTER) for example so we can chain operations to reduce recursiveness

		auto op = next();
		auto ast = parse_un();
		auto un = ExprUnAST(prev().m_position, op, ast, ExprUnAST::Side::LEFT);
		return std::make_shared<ExprUnAST>(un);
	}
	return parse_cast();
}
std::shared_ptr<AST> Parser::parse_cast() {
	auto higher_precedence = parse_interface_get();
	if (consume(Token::AS)) {
		// expecting type here
		if (!expecting_type()) {
			m_unit->m_error_handler.error(
				Error::Level::CRITICAL,
				Error::Type::MISSING_DELIMITER,
				"expected type after 'as'",
				prev().m_position
			);
			return std::make_shared<ErrorAST>(prev().m_position);
		}
		ExprCastAST c;
		c.m_position = higher_precedence->m_position.merge(prev().m_position);
		c.to_type = parse_type();
		c.value = higher_precedence;
		return std::make_shared<ExprCastAST>(c);
	}
	return higher_precedence;
}

std::shared_ptr<AST> Parser::parse_interface_get() {
	auto higher_precedence = parse_call();
	if (consume(Token::Type::DOT)) {
		// parsing an expression get
		if (!(consume(Token::Type::IDENTIFIER) || consume(Token::Type::NUMBER))) {
			m_unit->m_error_handler.error(
				Error::Level::CRITICAL,
				Error::Type::NOT_MEMBER,
				"expected identifier or number as interface member",
				peek().m_position
			);
		}
		return std::make_shared<ExprGetAST>(higher_precedence->m_position.merge(prev().m_position), higher_precedence, prev());
	}
	return higher_precedence;
}

std::shared_ptr<AST> Parser::parse_call() {
	auto higher_precedence = parse_single();
	// check if next is var args
	if (consume(Token::Type::LPAREN)) {
		ExprCallAST call;
		call.m_position = prev().m_position;
		call.callee = higher_precedence;
		if (!consume(Token::Type::RPAREN)) {

			// @TODO this should parse a pattern e.g. 1, 2, 3 rather than like this!
			while(!expect(Token::Type::RPAREN)){
				call.args.push_back(parse_expression());
				if (!expect(Token::Type::RPAREN))
					consume(Token::Type::COMMA);
			}
			call.has_args = 1;
			if (!consume(Token::Type::RPAREN)) {
				m_unit->m_error_handler.error(
					Error::Level::CRITICAL,
					Error::Type::MISSING_DELIMITER,
					"expected closing ) after fn call",
					prev().m_position
				);
				return std::make_shared<ErrorAST>(prev().m_position);
			}
		}
		call.m_position = call.m_position.merge(prev().m_position);
		return std::make_shared<ExprCallAST>(call);
	}
	return higher_precedence;
}
std::shared_ptr<AST> Parser::parse_single(){ 

	//@TODO this breaks because if you do y := x(); it thinks x is a type because x could be the name of an interface
	//if (expecting_type()){
	//	auto t = parse_type();
	//	return std::make_shared<ExprTypeAST>(prev().m_position, t);
	//}

	// @TODO implement casts
	// @TODO implement groups e.g. (1+2) + (1+3)
	auto t = next();


	switch (t.m_type) {
		// array literal for now
		case Token::Type::LCURLY: {


			/*
			
			
			{ can be used for multiple singular expressions

			1. array				    { 1, 2, 3  }
				note the array can also be used for interface initialisation
			2. interface definition     { x : u32; }

			
			
			*/


			// this is an interface definition :)
			if(expecting_def()){
				return parse_interface();
			}

			std::vector<std::shared_ptr<AST>> array_values;
			while (!expect(Token::Type::RCURLY)) {
				array_values.push_back(parse_single());
				if(!expect(Token::Type::RCURLY))
					consume(Token::Type::COMMA);
			}
			consume(Token::Type::RCURLY);
			return std::make_shared<ExprLiteralArrayAST>(prev().m_position, Type::create_basic(Type::Types::UNKNOWN), Type::create_basic(Type::Types::UNKNOWN), array_values.size(), array_values);
		}
		case Token::Type::IDENTIFIER: {
			return std::make_shared<ExprVarAST>(prev().m_position, t);
		}
		case Token::Type::NUMBER: {
			ExprLiteralAST lit_ast;
			lit_ast.m_position = prev().m_position;
			if(t.m_value.find('.') != std::string::npos)
				lit_ast.t = Type::create_basic(Type::Types::F32);
			else
				lit_ast.t = Type::create_basic(Type::Types::S32);
			lit_ast.v.value = t.m_value;
			return std::make_shared<ExprLiteralAST>(lit_ast);
			break;
		}
		case Token::Type::STRING_LIT: {
			// @TODO in retrospect, this is a terrible idea. we want the assembly file to have this as a global in .data section
			// such as .text_const: "hello world"
			// treat this as an array
			//std::vector<std::shared_ptr<AST>> array_values;
			//
			//ExprLiteralAST lit_ast;
			//lit_ast.t = Type(Type::Types::U8);
			//auto& s = t.value;
			//for (const auto& c : s) {
			//	lit_ast.v.v.as_u8 = c;
			//	array_values.push_back(std::make_shared<ExprLiteralAST>(lit_ast));
			//}
			//lit_ast.v.v.as_u8 = 0;
			//array_values.push_back(std::make_shared<ExprLiteralAST>(lit_ast));
			//
			//return std::make_shared<ExprLiteralArrayAST>(Type(Type::Types::UNKNOWN), Type(Type::Types::UNKNOWN), array_values.size(), array_values);
			ExprLiteralAST string_lit;
			string_lit.m_position = prev().m_position;
			string_lit.v.value = t.m_value;
			string_lit.t = Type(Type::Types::STRING);
			return std::make_shared<ExprLiteralAST>(string_lit);
			break;
		}
		case Token::Type::TRU: {
			ExprLiteralAST lit_ast;
			lit_ast.m_position = prev().m_position;
			lit_ast.t = Type::create_basic(Type::Types::U8);
			lit_ast.v.value = "1";
			return std::make_shared<ExprLiteralAST>(lit_ast);
		}
		case Token::Type::FLSE: {
			ExprLiteralAST lit_ast;
			lit_ast.m_position = prev().m_position;
			lit_ast.t = Type::create_basic(Type::Types::U8);
			lit_ast.v.value = "0";
			return std::make_shared<ExprLiteralAST>(lit_ast);
		}
		case Token::Type::LPAREN: {

			// at this point, we are either parsing a group or a fn.
			// to tell the difference, if we consume a rparen we are obviously parsing a fn.
			// if not, we need to check what comes next, if it is a definition stmt, then we are parsing a fn,
			// if not then its a group

			if(expect(Token::Type::RPAREN) || expecting_def()){
				return parse_fn();
			}else {
				auto expression = parse_expression();
				if (!consume(Token::Type::RPAREN)) {
					m_unit->m_error_handler.error(
						Error::Level::CRITICAL,
						Error::Type::MISSING_DELIMITER,
						"expected ) as statement delimiter",
						prev().m_position
					);
					return std::make_shared<ErrorAST>(prev().m_position);
				}
				auto group = ExprGroupAST(prev().m_position, expression);
				return std::make_shared<ExprGroupAST>(group);
			}
		}
		//case Token::Type::LBRACKET: return parse_interface();
		case Token::Type::INTERFACE: {
			return parse_interface();
		}
	}
	m_unit->m_error_handler.error(
		Error::Level::CRITICAL,
		Error::Type::MISSING_DELIMITER,
		"unexpected code?",
		prev().m_position
	);
	return std::make_shared<ErrorAST>(prev().m_position); 
}


std::shared_ptr<AST> Parser::parse_fn(){
	// doing fn
	ExprFnAST fn_ast;
	fn_ast.m_position = prev().m_position;
	std::vector<Type> operation_types;
	u8 has_return = 0;
	fn_ast.is_lambda = 1;
	// the fn can only be a lambda if it isn't being assigned to a constant
	// e.g. x : (){} is the only way a fn can be a lambda
	fn_ast.is_lambda = !m_parsing_constant_assignment;
	// !@TODO this needs to be resolved to a token



	operation_types.push_back(Type(Type::Types::U0));
	std::vector<std::shared_ptr<AST>> params;
	while (!expect(Token::Type::RPAREN)) {
		auto define = std::static_pointer_cast<StmtDefineAST>(parse_define());
		define->define_type.m_is_arg = 1;
		operation_types.push_back(define->define_type);
		params.push_back(define);
		if (!expect(Token::Type::RPAREN))
			consume(Token::Type::COMMA);
	}
	consume(Token::Type::RPAREN);
	fn_ast.params = params;
	// TODO move return type to the start
	if (expecting_type()) {
		operation_types.at(0) = parse_type();
		has_return = 1;
	}
	fn_ast.m_type = Type::create_fn(has_return, operation_types);
	// we need to check if the fn has a body
	if (!expect(Token::Type::SEMI_COLON)) {
		fn_ast.has_body = 1;
		fn_ast.body = parse_stmt();
	}
	fn_ast.m_position = fn_ast.m_position.merge(prev().m_position);
	return std::make_shared<ExprFnAST>(fn_ast);
}

std::shared_ptr<AST> Parser::parse_interface() {
	ExprInterfaceAST expr_interface;
	expr_interface.m_position = prev().m_position;

	std::vector<Type> members;

	consume(Token::LCURLY);

	// a list of definition
	while (!consume(Token::RCURLY)) {
		expr_interface.m_definitions.push_back(parse_stmt());
	}
	expr_interface.m_type = Type::create_interface(members);
	expr_interface.m_position = expr_interface.m_position.merge(prev().m_position);
	m_requiring_delimiter = 0;
	return std::make_shared<ExprInterfaceAST>(expr_interface);
}

