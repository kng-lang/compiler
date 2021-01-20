/*
James Clarke - 2021
*/

#pragma once

#include <sstream>
#include <string>
#include "common.h"
#include "types.h"
#include "token.h"


struct ASTVisitor;

struct StatementAST;
struct ExpressionAST;
struct ErrorAST;
struct ProgramAST;
struct StmtBlockAST;
struct StmtExpressionAST;
struct StmtDefineAST;
struct StmtInterfaceDefineAST; // e.g. app.main := () io.println "hello world"
struct StmtAssignAST;
struct StmtInterfaceAssignAST;
struct StmtReturnAST;
struct StmtContinueAST;
struct StmtBreakAST;
struct StmtIfAST;
struct StmtLoopAST;
struct ExprCastAST;
struct ExprCallAST;
struct ExprVarAST;
struct ExprPatternAST;
struct ExprGetAST; // this is used for interface gets and namespace gets, bc at parse time we can't tell the difference.
struct ExprBinAST;
struct ExprUnAST;
struct ExprGroupAST;
struct ExprLiteralAST;


struct AST {
	
	enum class ASTType{
		BASE,
		STMT,
		EXPR,
		ERR,
		PROG,
		STMT_BLOCK,
		STMT_EXPR,
		STMT_DEF,
		STMT_ASSIGN,
		STMT_INTER_ASSIGN,
		STMT_RET,
		STMT_CONT,
		STMT_BRK,
		STMT_IF,
		STMT_LOOP,
		EXPR_INTER,
		EXPR_FN,
		EXPR_CAST,
		EXPR_CALL,
		EXPR_VAR,
		EXPR_PATTERN,
		EXPR_GET,
		EXPR_BIN,
		EXPR_UN,
		EXPR_GROUP,
		EXPR_LIT,
		EXPR_LIT_ARRAY
	};
	

	AST(){}
	AST(Token::Position position) : m_position(position){}

	// this is so we can perform error handling whenever there is no token in the context
	Token::Position m_position;
	virtual void debug();
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::BASE; }
	virtual void* visit(ASTVisitor* visitor);
};


struct StatementAST : public AST {
	StatementAST() {}
	StatementAST(Token::Position position) : AST(position) {}
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::STMT; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ExpressionAST : public AST {
	ExpressionAST(){}
	ExpressionAST(Token::Position position) : AST(position){}
	virtual ASTType type() { return ASTType::EXPR; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ErrorAST : public StatementAST {
	std::string error_msg;
	ErrorAST(){}
	ErrorAST(Token::Position position) { m_position = position; }
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::ERR; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ProgramAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::PROG; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtBlockAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::STMT_BLOCK; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtExpressionAST : public StatementAST {
	std::shared_ptr<AST> expression;
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::STMT_EXPR; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtDefineAST : public StatementAST {
	u8 is_global = 0;
	u8 is_constant = 0;
	u8 is_initialised = 0;
	u8 requires_type_inference = 0;
	u8 m_is_underscore = 0;
	Token identifier;
	Type define_type;
	std::shared_ptr<AST> value;
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_DEF; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtAssignAST : public StatementAST {
	//Token variable;
	std::shared_ptr<AST> assignee;
	std::shared_ptr<AST> value;

	StmtAssignAST(Token::Position position, std::shared_ptr<AST> assignee, std::shared_ptr<AST> value) : StatementAST(position), assignee(assignee), value(value){}

	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_ASSIGN; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtInterfaceAssignAST : public StatementAST {
	std::shared_ptr<AST> variable; // the actual interface
	Token member;				   // the interface member
	std::shared_ptr<AST> value;    // the value to assign

	StmtInterfaceAssignAST(
						Token::Position position,
						   std::shared_ptr<AST> variable,
						   Token member,
						   std::shared_ptr<AST> value
		) : StatementAST(position), variable(variable), member(member), value(value) {}

	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_INTER_ASSIGN; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtReturnAST : public StatementAST {
	std::shared_ptr<AST> value;
	StmtReturnAST(){}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_RET; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtContinueAST: public StatementAST {
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_CONT; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtBreakAST : public StatementAST {
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_BRK; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtIfAST : public StatementAST {
	std::shared_ptr<AST> if_cond;
	std::shared_ptr<AST> if_stmt;
	std::shared_ptr<AST> else_stmt;
	u8 has_else = 0;
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_IF; }
	virtual void* visit(ASTVisitor* visitor);
};

struct StmtLoopAST : public StatementAST {
	
	enum class LoopType {
		INF,
	};

	StmtLoopAST(){}

	LoopType loop_type;
	std::shared_ptr<AST> body;
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::STMT_LOOP; }
	virtual void* visit(ASTVisitor* visitor);
};

// e.g. x : interface = { y : s32 }
struct ExprInterfaceAST : public ExpressionAST {
	// even though they are first class, they still need a name
	std::string anonymous_name;
	ExprInterfaceAST(){}
	ExprInterfaceAST(std::string anonymous_name) : anonymous_name(anonymous_name) {}
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::EXPR_INTER; }
	virtual void* visit(ASTVisitor* visitor);
};

// this is a lambda e.g. () io.println "hello world!", it can be assigned to variables e.g. x := () io.println "hello world!"
struct ExprFnAST : public ExpressionAST {
	std::shared_ptr<AST> body;
	std::vector<std::shared_ptr<AST>> params;
	u8 has_body = 0; // e.g. if we are declaring an external fn
	u8 is_lambda = 0; // this is used for name resolution
	// the full type signature
	Type full_type;
	ExprFnAST() {}
	ExprFnAST(std::shared_ptr<AST> body, Type full_type) : body(body), full_type(full_type) {}
	virtual std::string to_json();
	virtual ASTType type() { return ASTType::EXPR_FN; }
	virtual void* visit(ASTVisitor* visitor);
};


struct ExprCastAST : public ExpressionAST {
	std::shared_ptr<AST> value;
	Type from_type;
	Type to_type;
	u8 niavely_resolved = 0;
	ExprCastAST(){}
	ExprCastAST(std::shared_ptr<AST> value, Type from_type, Type to_type) : 
		value(value), 
		from_type(from_type),
		to_type(to_type){}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_CAST; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ExprCallAST : public ExpressionAST {
	std::shared_ptr<AST> callee;
	std::vector<std::shared_ptr<AST>> args;
	u8 has_args = 0;
	ExprCallAST() {}
	ExprCallAST(std::shared_ptr<AST> callee, std::vector<std::shared_ptr<AST>> args, u8 has_args) :
		callee(callee),
		args(args),
		has_args(has_args){}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_CALL; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ExprVarAST : public ExpressionAST {
	Token identifier;
	ExprVarAST(Token::Position position, Token identifier) : ExpressionAST(position), identifier(identifier){}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_VAR; }
	virtual void* visit(ASTVisitor* visitor);
};


struct ExprPatternAST : public ExpressionAST {
	std::vector<std::shared_ptr<AST>> asts;
	ExprPatternAST(){}
	ExprPatternAST(std::vector<std::shared_ptr<AST>> asts) : asts(asts) {}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_PATTERN; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ExprGetAST : public ExpressionAST {

	// this represents what the lhs is
	enum class GetType {
		INTERFACE,
		FILE,
		MODULE
	};

	GetType get_type;
	std::shared_ptr<AST> value;
	Token member;
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_GET; }
	virtual void* visit(ASTVisitor* visitor);
};


struct ExprBinAST : public ExpressionAST {
	std::shared_ptr<AST> lhs;
	std::shared_ptr<AST> rhs;
	Token op;
	// the types of the operation that is performed (signed, floating point, interface)
	Type::Types m_value_type;
	ExprBinAST(){}
	ExprBinAST(Token::Position position, std::shared_ptr<AST> lhs, std::shared_ptr<AST> rhs, Token op) : ExpressionAST(position), lhs(lhs), rhs(rhs), op(op){}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_BIN; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ExprUnAST : public ExpressionAST {

	enum class Side {
		LEFT,
		RIGHT
	};

	Token op;
	std::shared_ptr<AST> ast;
	Side side;

	ExprUnAST(){}
	ExprUnAST(Token::Position position, Token op, std::shared_ptr<AST> ast, Side side) : ExpressionAST(position), op(op), ast(ast), side(side){}

	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_UN; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ExprGroupAST : public ExpressionAST {
	std::shared_ptr<AST> expression;
	ExprGroupAST(){}
	ExprGroupAST(Token::Position position, std::shared_ptr<AST> expression) : ExpressionAST(position), expression(expression) {}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_GROUP; }
	virtual void* visit(ASTVisitor* visitor);
};

// @TODO technically a function is a literal?
struct ExprLiteralAST : public ExpressionAST {
	Type t;
	struct Value {
		std::string value;
		u8 as_u8() { return std::atoi(value.c_str()); }
		s8 as_s8() { return std::atoi(value.c_str()); }
		u16 as_u16() { return std::atoi(value.c_str()); }
		s16 as_s16() { return std::atoi(value.c_str()); }
		u32 as_u32() { return std::atoi(value.c_str()); }
		s32 as_s32() { return std::atoi(value.c_str()); }
		s64 as_s64() { return std::atoi(value.c_str()); }
		f32 as_f32() { return std::atof(value.c_str()); }
		f64 as_f64() { return std::atof(value.c_str()); }
		char as_char() { return value.at(0); }
		std::string as_string() { return value; }
	};
	Value v;
	ExprLiteralAST(){}
	ExprLiteralAST(Token::Position position, Type t, Value v) : ExpressionAST(position), t(t) { this->v = v; }
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_LIT; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ExprLiteralArrayAST : public ExpressionAST {
	Type array_type;
	Type contained_type;
	u32 size;
	std::vector<std::shared_ptr<AST>> values;
	ExprLiteralArrayAST() {}
	ExprLiteralArrayAST(Token::Position position, Type array_type, Type contained_type, u32 size, std::vector<std::shared_ptr<AST>> values) 
		: ExpressionAST(position), array_type(array_type), contained_type(contained_type), size(size), values(values) {}
	virtual std::string to_json();
	virtual ASTType  type() { return ASTType::EXPR_LIT_ARRAY; }
	virtual void* visit(ASTVisitor* visitor);
};

struct ASTVisitor {
	std::shared_ptr<AST> m_ast;

	ASTVisitor(){}
	ASTVisitor(std::shared_ptr<AST> ast) : m_ast(ast){}

	virtual void* visit_program(ProgramAST* program_ast) = 0;
	virtual void* visit_stmt_block(StmtBlockAST* stmt_block_ast) = 0;
	virtual void* visit_stmt_expression(StmtExpressionAST* stmt_expression_ast) = 0;
	virtual void* visit_stmt_define(StmtDefineAST* stmt_define_ast) = 0;
	virtual void* visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) = 0;
	virtual void* visit_stmt_assign(StmtAssignAST* stmt_assign_ast) = 0;
	virtual void* visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast) = 0;
	virtual void* visit_stmt_return(StmtReturnAST* stmt_return_ast) = 0;
	virtual void* visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast) = 0;
	virtual void* visit_stmt_break_ast(StmtBreakAST* stmt_break_ast) = 0;
	virtual void* visit_stmt_if_ast(StmtIfAST* stmt_if_ast) = 0;
	virtual void* visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast) = 0;
	virtual void* visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast) = 0;
	virtual void* visit_expr_fn_ast(ExprFnAST* expr_fn_ast) = 0;
	virtual void* visit_expr_cast_ast(ExprCastAST* expr_cast_ast) = 0;
	virtual void* visit_expr_call_ast(ExprCallAST* expr_call_ast) = 0;
	virtual void* visit_expr_var_ast(ExprVarAST* expr_var_ast) = 0;
	virtual void* visit_expr_interface_get_ast(ExprGetAST* expr_interface_get_ast) = 0;
	virtual void* visit_expr_bin_ast(ExprBinAST* expr_bin_ast) = 0;
	virtual void* visit_expr_un_ast(ExprUnAST* expr_un_ast) = 0;
	virtual void* visit_expr_group_ast(ExprGroupAST* expr_group_ast) = 0;
	virtual void* visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) = 0;
	virtual void* visit_expr_literal_array_ast(ExprLiteralArrayAST* expr_literal_array_ast) = 0;
};