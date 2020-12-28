#include "typechecking.h"
#include "compiler.h"

std::shared_ptr<AST> TypeChecker::check() {
	ast->visit(this);
	return ast;
}

void* TypeChecker::visit_program(ProgramAST* program_ast) { 
	for (const auto& ast : program_ast->stmts)
		ast->visit(this);
	return NULL; 
}
void* TypeChecker::visit_stmt_block(StmtBlockAST* stmt_block_ast) { 
	return NULL; 
}
void* TypeChecker::visit_stmt_expression(StmtExpressionAST* stmt_expression_ast) { return NULL; }

void* TypeChecker::visit_stmt_define(StmtDefineAST* stmt_define_ast) { 
	// if we need to infer the type then do it here
	if(stmt_define_ast->requires_type_inference){
		auto inferred = stmt_define_ast->value->visit(this);
		stmt_define_ast->define_type = *((Type*)inferred);
	}
	else if (stmt_define_ast->value != NULL) {
		// else check the assignment value is valid
		auto inferred = stmt_define_ast->value->visit(this);
		Type t = *((Type*)inferred);

		if (!t.matches_basic(stmt_define_ast->define_type))
			unit->error_handler.error("types do not match",
				stmt_define_ast->identifier.index,
				stmt_define_ast->identifier.line,
				stmt_define_ast->identifier.index+stmt_define_ast->identifier.length,
				stmt_define_ast->identifier.line);
	}


	
	return NULL; 
}

void* TypeChecker::visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) { return NULL; }

void* TypeChecker::visit_stmt_assign(StmtAssignAST* stmt_assign_ast) { 
	return NULL; 
}

void* TypeChecker::visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast) { return NULL; }

void* TypeChecker::visit_stmt_return(StmtReturnAST* stmt_return_ast) { return NULL; }

void* TypeChecker::visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast) { return NULL; }

void* TypeChecker::visit_stmt_break_ast(StmtBreakAST* stmt_break_ast) { return NULL; }

void* TypeChecker::visit_stmt_if_ast(StmtIfAST* stmt_if_ast) { return NULL; }

void* TypeChecker::visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast) { return NULL; }


void* TypeChecker::visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast) { return NULL; }

void* TypeChecker::visit_expr_fn_ast(ExprFnAST* expr_fn_ast) { return NULL;  }

void* TypeChecker::visit_expr_var_ast(ExprVarAST* expr_var_ast) { return NULL; }

void* TypeChecker::visit_expr_interface_get_ast(ExprInterfaceGetAST* expr_interface_get_ast) { return NULL; }

void* TypeChecker::visit_expr_bin_ast(ExprBinAST* expr_bin_ast) { return NULL; }

void* TypeChecker::visit_expr_un_ast(ExprUnAST* expr_un_ast) { return NULL; }

void* TypeChecker::visit_expr_group_ast(ExprGroupAST* expr_group_ast) {
	return expr_group_ast->visit(this);
}

void* TypeChecker::visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) { 
	kng_log("type checking literal!");
	return (void*)&expr_literal_ast->t;
}