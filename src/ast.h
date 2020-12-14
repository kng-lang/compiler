#pragma once

#include <sstream>
#include <string>
#include "common.h"
#include "types.h"

struct AST {
	virtual void debug();
	virtual std::string to_json();
};


struct StatementAST : public AST {
	virtual std::string to_json();
};

struct ExpressionAST : public AST {};

struct ErrorAST : public StatementAST {
	std::string error_msg;
};

struct ProgramAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
};

struct StmtBlockAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
};

struct StmtExpressionAST : public StatementAST {
	std::shared_ptr<AST> expression;
	virtual std::string to_json();
};

struct StmtDefineAST : public StatementAST {
	std::string identifier;
	Type type;
	std::shared_ptr<ExpressionAST> value;
	virtual std::string to_json();
};

struct StmtAssignAST : public StatementAST {
	virtual std::string to_json();
};

struct StmtInterfaceSetAST : public StatementAST {
	std::string i_identifier;
	std::string m_identifier;
	std::shared_ptr<ExpressionAST> value;
	virtual std::string to_json();
};

struct StmtReturnAST : public StatementAST {
	std::shared_ptr<ExpressionAST> value;
	StmtReturnAST(){}
	virtual std::string to_json();
};

struct StmtContinueAST: public StatementAST {
	virtual std::string to_json();
};

struct StmtBreakAST : public StatementAST {
	virtual std::string to_json();
};

struct StmtIfAST : public StatementAST {
	std::shared_ptr<AST> if_cond;
	std::shared_ptr<AST> if_stmt;
	virtual std::string to_json();
};

struct StmtLoopAST : public StatementAST {

};

struct ExprVarAST : public ExpressionAST {
	std::string identifier;
	ExprVarAST(std::string identifier) : identifier(identifier){}
};

struct ExprInterfaceGetAST : public ExpressionAST {
	std::string i_identifier;
	std::string m_identifier;
	virtual std::string to_json();
};


struct ExprBinAST : public ExpressionAST {
	std::shared_ptr<ExpressionAST> lhs;
	std::shared_ptr<ExpressionAST> rhs;
	virtual std::string to_json();
};

struct ExprLiteralAST : public ExpressionAST {
	Type type;
	ExprLiteralAST(){}
	virtual std::string to_json();
};