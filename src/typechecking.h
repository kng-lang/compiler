/*
James Clarke - 2021
*/

#pragma once

#include <memory>

#include "symtable.h"
#include "types.h"
#include "common.h"
#include "ast.h"

struct CompilationUnit;

struct TypeChecker : public ASTVisitor {
	CompilationUnit* m_unit;
	SymTable m_sym_table;
	Type m_checked_type;
	Type* m_checked_type_ptr;
	// these are used as identifiers for anonymous types
	u32 m_anon_fn_counter = 0;
	u32 m_anon_inter_counter = 0;

	TypeChecker() {}
	TypeChecker(std::shared_ptr<AST> ast, CompilationUnit* unit) : ASTVisitor(ast), m_unit(unit) {}

	

	u8 is_l_value(std::shared_ptr<AST> ast);
	u8 is_r_value(std::shared_ptr<AST> ast);
	// cast any ast (must be an expression) to a given type
	void niave_cast_ast(Type t, std::shared_ptr<AST> ast);
	std::shared_ptr<AST> check();
	void cast_array(Type* r_type, Type l_type, std::shared_ptr<ExprLiteralArrayAST> array_ast);

	virtual void* visit_program(ProgramAST* program_ast);
	virtual void* visit_stmt_block(StmtBlockAST* stmt_block_ast);
	virtual void* visit_stmt_expression(StmtExpressionAST* stmt_expression_ast);
	virtual void* visit_stmt_define(StmtDefineAST* stmt_define_ast);
	virtual void* visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast);
	virtual void* visit_stmt_assign(StmtAssignAST* stmt_assign_ast);
	virtual void* visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast);
	virtual void* visit_stmt_return(StmtReturnAST* stmt_return_ast);
	virtual void* visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast);
	virtual void* visit_stmt_break_ast(StmtBreakAST* stmt_break_ast);
	virtual void* visit_stmt_if_ast(StmtIfAST* stmt_if_ast);
	virtual void* visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast);
	virtual void* visit_stmt_defer_ast(StmtDeferAST* stmt_defer_ast);
	virtual void* visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast);
	virtual void* visit_expr_fn_ast(ExprFnAST* expr_fn_ast);
	virtual void* visit_expr_cast_ast(ExprCastAST* expr_cast_ast);
	virtual void* visit_expr_call_ast(ExprCallAST* expr_call_ast);
	virtual void* visit_expr_var_ast(ExprVarAST* expr_var_ast);
	virtual void* visit_expr_pattern_ast(ExprPatternAST* expr_pattern_ast);
	virtual void* visit_expr_interface_get_ast(ExprGetAST* expr_interface_get_ast);
	virtual void* visit_expr_bin_ast(ExprBinAST* expr_bin_ast);
	virtual void* visit_expr_un_ast(ExprUnAST* expr_un_ast);
	virtual void* visit_expr_group_ast(ExprGroupAST* expr_group_ast);
	virtual void* visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast);
	virtual void* visit_expr_literal_array_ast(ExprLiteralArrayAST* expr_literal_array_ast);
	virtual void* visit_expr_type_ast(ExprTypeAST* expr_type_ast);
	virtual void* visit_expr_included_ast(ExprIncludedAST* expr_included_ast);
	virtual void* visit_expr_module_ast(ExprModuleAST* expr_module_ast);
};