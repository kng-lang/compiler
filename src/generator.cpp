#include "generator.h"

#include "compiler.h"
#include <iostream>


void LLVMCodeGen::generate() {
	kng_log("generating...");
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();

	this->llvm_context = std::make_unique<llvm::LLVMContext>();
	this->llvm_builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*llvm_context));
	this->llvm_module = std::make_unique<llvm::Module>(unit->compile_file.file_path, *llvm_context);

	this->ast->visit(this);

	make_runtime();
	optimise();
	llvm_module->print(llvm::outs(), nullptr);
}

void LLVMCodeGen::make_runtime() {
	// first create the main function
	llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(*llvm_context), { llvm::Type::getInt32Ty(*llvm_context), llvm::Type::getInt32PtrTy(*llvm_context) }, false);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", *llvm_module);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(*llvm_context, "main_block", f);
	llvm_builder->SetInsertPoint(bb);
	llvm_builder->ClearInsertionPoint();
	llvm::verifyFunction(*f);
}

void LLVMCodeGen::optimise(){}


llvm::Type* LLVMCodeGen::convert_type(Type type) {
	switch (type.t) {
		case Type::Types::U0:     return llvm::Type::getVoidTy(*llvm_context);
		case Type::Types::U8:     return llvm::Type::getInt8Ty(*llvm_context);
		case Type::Types::U16:    return llvm::Type::getInt16Ty(*llvm_context);
		case Type::Types::U32:    return llvm::Type::getInt32Ty(*llvm_context);
		case Type::Types::S32:    return llvm::Type::getInt32Ty(*llvm_context);
		case Type::Types::S64:    return llvm::Type::getInt64Ty(*llvm_context);
		case Type::Types::F32:    return llvm::Type::getFloatTy(*llvm_context);
		case Type::Types::F64:    return llvm::Type::getDoubleTy(*llvm_context);
		case Type::Types::CHAR:   return llvm::Type::getInt8Ty(*llvm_context); // ASCII FOR NOW?
		case Type::Types::FN:     return sym_table.get_symbol(type.fn_signature.anonymous_identifier); // @TODO return the reference to the fn in the symbol table
		case Type::Types::STRING: return NULL; // @TODO return a reference to the string interface using the symbol table

	}
}

void* LLVMCodeGen::visit_program(ProgramAST* program_ast){
	for (const auto& stmt : program_ast->stmts)
		stmt->visit(this);
	return NULL;
}
void* LLVMCodeGen::visit_stmt_block(StmtBlockAST* stmt_block_ast) {
	sym_table.enter_scope();
	for (const auto& stmt : stmt_block_ast->stmts)
		stmt->visit(this);
	sym_table.pop_scope();
	return NULL;
}
void* LLVMCodeGen::visit_stmt_expression(StmtExpressionAST* stmt_expression_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_define(StmtDefineAST* stmt_define_ast) {

	if (!stmt_define_ast->is_global){
		// do alloca
		if (stmt_define_ast->define_type.t != Type::Types::FN)
			llvm_builder->CreateAlloca(convert_type(stmt_define_ast->define_type), NULL, stmt_define_ast->identifier.value);
	}
	else {
		if(stmt_define_ast->define_type.t!=Type::Types::FN)
			auto g = llvm_module->getOrInsertGlobal(llvm::StringRef(stmt_define_ast->identifier.value), convert_type(stmt_define_ast->define_type));
		else {
		}
	}
	if (stmt_define_ast->is_initialised)
		stmt_define_ast->value->visit(this);
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


void* LLVMCodeGen::visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast){
	// create the struct and the virtual table for the struct
	auto name = llvm::StringRef(expr_interface_ast->anonymous_name);
	auto interface_type = llvm::StructType::create(*llvm_context, name);
	auto interface_vtable = llvm::StructType::create(*llvm_context, llvm::StringRef("vtable_"+expr_interface_ast->anonymous_name));
	return NULL;
}

void* LLVMCodeGen::visit_expr_fn_ast(ExprFnAST* expr_fn_ast) {
	std::vector<llvm::Type*> operation_types;
	for (const auto& t : expr_fn_ast->full_type.fn_signature.operation_types)
		operation_types.push_back(convert_type(t));
	llvm::FunctionType* ft = llvm::FunctionType::get(operation_types.at(0), {}, false);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, expr_fn_ast->full_type.fn_signature.anonymous_identifier, *llvm_module);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(*llvm_context, "entry_block", f);
	llvm_builder->SetInsertPoint(bb);

	// code gen the fn body
	expr_fn_ast->body->visit(this);
	
	llvm_builder->ClearInsertionPoint();
	llvm::verifyFunction(*f);


	// add the fn type to the symbol table
	sym_table.add_symbol(expr_fn_ast->full_type.fn_signature.anonymous_identifier, ft);
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