#include "generator.h"

#include "compiler.h"
#include <iostream>


std::shared_ptr<CodeGen> LLVMGenerator::generate() {
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();

	// create a module
	auto m = std::make_shared<llvm::Module>(unit->compile_file.file_path, llvm_context);

	llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvm_context), { llvm::Type::getInt32Ty(llvm_context) }, true);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "test_fn", *m);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(llvm_context, "entry_block", f);


	m->dump();

	return std::make_shared<LLVMCodeGen>();
}

void* LLVMCodeGen::visit_program(ProgramAST* program_ast){
	return NULL;
}
void* LLVMCodeGen::visit_stmt_block(StmtBlockAST* stmt_block_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_expression(StmtExpressionAST* stmt_expression_ast) {
	return NULL;
}
void*  LLVMCodeGen::visit_stmt_define(StmtDefineAST* stmt_define_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_assign(StmtAssignAST* stmt_assign_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_return(StmtReturnAST* stmt_return_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_break_ast(StmtBreakAST* stmt_break_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_if_ast(StmtIfAST* stmt_if_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_fn_ast(ExprFnAST* expr_fn_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_var_ast(ExprVarAST* expr_var_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_interface_get_ast(ExprInterfaceGetAST* expr_interface_get_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_bin_ast(ExprBinAST* expr_bin_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_un_ast(ExprUnAST* expr_un_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_group_ast(ExprGroupAST* expr_group_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) {
	return NULL;
}