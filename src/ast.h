#pragma once

#include <sstream>
#include <string>
#include "common.h"
#include "types.h"
#include "token.h"

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
struct ExprVarAST;
struct ExprPatternAST;
struct ExprInterfaceGetAST;
struct ExprBinAST;
struct ExprUnAST;
struct ExprGroupAST;
struct ExprLiteralAST;

struct AST {
	
	enum class Type{
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
		EXPR_VAR,
		EXPR_PATTERN,
		EXPR_INTER_GET,
		EXPR_BIN,
		EXPR_UN,
		EXPR_GROUP,
		EXPR_LIT,
	};
	
	u32 index;
	u32 line;
	u32 end_index;
	u32 end_line;

	virtual void debug();
	virtual std::string to_json();
	virtual Type type() { return Type::BASE; }
};


struct StatementAST : public AST {
	virtual std::string to_json();
	virtual Type type() { return Type::STMT; }

};

struct ExpressionAST : public AST {
	virtual Type type() { return Type::EXPR; }
};

struct ErrorAST : public StatementAST {
	std::string error_msg;
	virtual std::string to_json();
	virtual Type type() { return Type::ERR; }
};

struct ProgramAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
	virtual Type type() { return Type::PROG; }
};

struct StmtBlockAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_BLOCK; }
};

struct StmtExpressionAST : public StatementAST {
	std::shared_ptr<AST> expression;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_EXPR; }
};

struct StmtDefineAST : public StatementAST {
	std::string identifier;
	Type define_type;
	std::shared_ptr<AST> value;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_DEF; }
};

struct StmtAssignAST : public StatementAST {
	Token variable;
	std::shared_ptr<AST> value;

	StmtAssignAST(Token variable, std::shared_ptr<AST> value) : variable(variable), value(value){}

	virtual std::string to_json();
	virtual Type type() { return Type::STMT_ASSIGN; }
};

struct StmtInterfaceAssignAST : public StatementAST {
	std::shared_ptr<AST> variable; // the actual interface
	Token member;				   // the interface member
	std::shared_ptr<AST> value;    // the value to assign

	StmtInterfaceAssignAST(std::shared_ptr<AST> variable,
						   Token member,
						   std::shared_ptr<AST> value
		) : variable(variable), member(member), value(value) {}

	virtual std::string to_json();
	virtual Type type() { return Type::STMT_INTER_ASSIGN; }
};

struct StmtReturnAST : public StatementAST {
	std::shared_ptr<ExpressionAST> value;
	StmtReturnAST(){}
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_RET; }
};

struct StmtContinueAST: public StatementAST {
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_CONT; }
};

struct StmtBreakAST : public StatementAST {
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_BRK; }
};

struct StmtIfAST : public StatementAST {
	std::shared_ptr<AST> if_cond;
	std::shared_ptr<AST> if_stmt;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_IF; }
};

struct StmtLoopAST : public StatementAST {
	virtual Type type() { return Type::STMT_LOOP; }
};

struct ExprVarAST : public ExpressionAST {
	Token identifier;
	ExprVarAST(Token identifier) : identifier(identifier){}
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_VAR; }
};


struct ExprPatternAST : public ExpressionAST {
	std::vector<std::shared_ptr<AST>> asts;
	ExprPatternAST(){}
	ExprPatternAST(std::vector<std::shared_ptr<AST>> asts) : asts(asts) {}
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_PATTERN; }
};

struct ExprInterfaceGetAST : public ExpressionAST {
	std::shared_ptr<AST> value;
	Token member;
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_INTER_GET; }
};


struct ExprBinAST : public ExpressionAST {
	std::shared_ptr<AST> lhs;
	std::shared_ptr<AST> rhs;
	Token op;
	ExprBinAST(){}
	ExprBinAST(std::shared_ptr<AST> lhs, std::shared_ptr<AST> rhs, Token op) : lhs(lhs), rhs(rhs), op(op){}
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_BIN; }
};

struct ExprUnAST : public ExpressionAST {
	Token op;
	std::shared_ptr<AST> ast;
	u8 side; // left (0), right (1)

	ExprUnAST(){}
	ExprUnAST(Token op, std::shared_ptr<AST> ast, u8 side) : op(op), ast(ast), side(side){}

	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_UN; }
};

struct ExprGroupAST : public ExpressionAST {
	std::shared_ptr<AST> expression;
	ExprGroupAST(){}
	ExprGroupAST(std::shared_ptr<AST> expression) : expression(expression) {}
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_GROUP; }
};

// @TODO technically a function is a literal?
struct ExprLiteralAST : public ExpressionAST {
	Type literal_type;
	ExprLiteralAST(){}
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_LIT; }
};

struct ASTVisitor {
	virtual std::shared_ptr<AST> visit_program(std::shared_ptr<ProgramAST> program_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_block(std::shared_ptr<StmtBlockAST> stmt_block_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_expression(std::shared_ptr<StmtExpressionAST> stmt_expression_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_define(std::shared_ptr<StmtDefineAST> stmt_define_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_interface_define(std::shared_ptr<StmtInterfaceDefineAST> stmt_interface_define_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_assign(std::shared_ptr<StmtAssignAST> stmt_assign_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_interface_assign(std::shared_ptr<StmtInterfaceAssignAST> stmt_interface_assign_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_return(std::shared_ptr<StmtReturnAST> stmt_return_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_continue_ast(std::shared_ptr<StmtContinueAST> stmt_continue_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_break_ast(std::shared_ptr<StmtBreakAST> stmt_break_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_if_ast(std::shared_ptr<StmtIfAST> stmt_if_ast) = 0;
	virtual std::shared_ptr<AST> visit_stmt_loop_ast(std::shared_ptr<StmtLoopAST> stmt_loop_ast) = 0;
	virtual std::shared_ptr<AST> visit_expr_var_ast(std::shared_ptr<ExprVarAST> expr_var_ast) = 0;
	virtual std::shared_ptr<AST> visit_expr_interface_get_ast(std::shared_ptr<ExprInterfaceGetAST> expr_interface_get_ast) = 0;
	virtual std::shared_ptr<AST> visit_expr_bin_ast(std::shared_ptr<ExprBinAST> expr_bin_ast) = 0;
	virtual std::shared_ptr<AST> visit_expr_un_ast(std::shared_ptr<ExprUnAST> expr_un_ast) = 0;
	virtual std::shared_ptr<AST> visit_expr_group_ast(std::shared_ptr<ExprGroupAST> expr_group_ast) = 0;
	virtual std::shared_ptr<AST> visit_expr_literal_ast(std::shared_ptr<ExprLiteralAST> expr_literal_ast) = 0;
};