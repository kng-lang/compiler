/*
James Clarke - 2021
*/

#include "generator.h"
#include "compiler.h"
#include <iostream>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/CodeGen/LinkAllAsmWriterComponents.h>
#include <llvm/CodeGen/LinkAllCodegenComponents.h>
#include <llvm/CodeGen/MIRParser/MIRParser.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/IR/AutoUpgrade.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/MC/SubtargetFeature.h>
#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/PluginLoader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Utils/Cloning.h>

void LLVMCodeGen::generate() {
	this->llvm_context = std::make_unique<llvm::LLVMContext>();
	this->llvm_builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*llvm_context));
	this->llvm_module = std::make_unique<llvm::Module>(unit->compile_file.file_path, *llvm_context);

	this->ast->visit(this);

	make_runtime();

	using namespace llvm;

	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmPrinters();
	InitializeAllAsmParsers();

	auto target_triple = sys::getDefaultTargetTriple();

	std::string error;
	auto target = TargetRegistry::lookupTarget(target_triple, error);

	// Print an error and exit if we couldn't find the requested target.
	// This generally occurs if we've forgotten to initialise the
	// TargetRegistry or we have a bogus target triple.
	if (!target) {
		kng_errr("{}", error);
		return;
	}

	auto CPU = "generic";
	auto Features = "";

	TargetOptions opt;
	auto RM = Optional<Reloc::Model>();
	auto target_machine = target->createTargetMachine(target_triple, CPU, Features, opt, RM);

	llvm_module->setDataLayout(target_machine->createDataLayout());
	llvm_module->setTargetTriple(target_triple);

	auto filename = "C:/kng/compiler/tests/output.s";

	std::error_code ec;
	raw_fd_ostream dest(filename, ec, sys::fs::OF_None);

	if (ec) {
		kng_errr("Could not open file: {}", ec.message());
		return;
	}

	legacy::PassManager pass;
	auto filetype = CGFT_AssemblyFile; // CGFT_ObjectFile;
		
	
    // https://llvm.org/docs/Passes.html
	if (target_machine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
		kng_errr("TargetMachine can't emit a file of this type");
		return;
	}
	if (unit->compile_options.optimise) {
		pass.add(llvm::createCodeGenPreparePass());
		pass.add(llvm::createPromoteMemoryToRegisterPass());
		pass.add(llvm::createReassociatePass());
		pass.add(llvm::createInterleavedLoadCombinePass());
		pass.add(llvm::createNewGVNPass());
		pass.add(llvm::createCFGSimplificationPass());
		pass.add(llvm::createLoopUnrollPass());
		pass.add(llvm::createLoopUnswitchPass());
		pass.add(llvm::createLoopRotatePass());
	}

	pass.run(*llvm_module);

	if (this->unit->compile_options.debug_emission_flags & EMIT_IR_DEBUG)
		llvm_module->print(llvm::errs(), nullptr);
	dest.flush();
}

void LLVMCodeGen::make_runtime() {

	// first create the main function

	auto ptr_to_ptr = llvm::PointerType::getUnqual(llvm::Type::getInt32PtrTy(*llvm_context));

	llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(*llvm_context), { llvm::Type::getInt32Ty(*llvm_context), ptr_to_ptr }, false);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", *llvm_module);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(*llvm_context, "main_block", f);
	llvm_builder->SetInsertPoint(bb);
	llvm_builder->CreateRet(llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*llvm_context), 0));
	llvm_builder->ClearInsertionPoint();
	llvm::verifyFunction(*f);
}

void LLVMCodeGen::optimise(){}


llvm::Type* LLVMCodeGen::convert_type(Type type) {
	llvm::Type* tmp_type = NULL;
	switch (type.t) {
	case Type::Types::U0:     tmp_type = llvm::Type::getVoidTy(*llvm_context); break;
		case Type::Types::U8:     tmp_type = llvm::Type::getInt8Ty(*llvm_context); break;
		case Type::Types::U16:    tmp_type = llvm::Type::getInt16Ty(*llvm_context); break;
		case Type::Types::U32:    tmp_type = llvm::Type::getInt32Ty(*llvm_context); break;
		case Type::Types::S32:    tmp_type = llvm::Type::getInt32Ty(*llvm_context); break;
		case Type::Types::S64:    tmp_type = llvm::Type::getInt64Ty(*llvm_context); break;
		case Type::Types::F32:    tmp_type = llvm::Type::getFloatTy(*llvm_context); break;
		case Type::Types::F64:    tmp_type = llvm::Type::getDoubleTy(*llvm_context); break;
		case Type::Types::CHAR:   tmp_type = llvm::Type::getInt8Ty(*llvm_context); break; // ASCII FOR NOW?
		case Type::Types::FN:     tmp_type = (llvm::FunctionType*)((llvm::Function*)sym_table.get_symbol(type.fn_signature.anonymous_identifier).optional_data)->getType(); break; // @TODO return the reference to the fn in the symbol table
		case Type::Types::STRING: tmp_type = NULL; break; // @TODO return a reference to the string interface using the symbol table

	}
	// its only an array and not a ptr if the arr_length > 0 otherwise x : u8[] is a ptr
	if (type.is_arr && type.arr_length > 0)
		tmp_type = llvm::ArrayType::get(tmp_type, type.arr_length);
	if (type.ptr_indirection) {
		for (int i = 0; i < type.ptr_indirection; i++)
			tmp_type = llvm::PointerType::getUnqual(tmp_type);
	}
	kng_assert(tmp_type, "tmp_type NULL");
	return tmp_type;
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

	void* creation_instr = NULL; // either AllocaInst or Constant*
	//llvm::AllocaInst* alloca_instr = NULL;
	if (!stmt_define_ast->is_global){
		// do alloca
		if (stmt_define_ast->define_type.t != Type::Types::FN) {
			creation_instr = llvm_builder->CreateAlloca(convert_type(stmt_define_ast->define_type), NULL, stmt_define_ast->identifier.value);
			sym_table.add_symbol(stmt_define_ast->identifier.value, SymTableEntry(creation_instr, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->is_constant));
		}
	}
	else {
		if(stmt_define_ast->define_type.t!=Type::Types::FN){
			creation_instr = llvm_module->getOrInsertGlobal(llvm::StringRef(stmt_define_ast->identifier.value), convert_type(stmt_define_ast->define_type));
			sym_table.add_symbol(stmt_define_ast->identifier.value, SymTableEntry(creation_instr, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->is_constant));
		}else {
			
		}
	}
	
	// @TODO this works if it isn't a function...
	if (stmt_define_ast->is_initialised
		&& stmt_define_ast->define_type.t != Type::Types::FN) {
		auto val = stmt_define_ast->value->visit(this);
		auto is_volative = false;
		kng_assert(creation_instr != NULL, "creaton_instr was null");
		llvm_builder->CreateStore((llvm::Value*)val, (llvm::Value*) creation_instr, is_volative);
	}

	// @TODO jesus fix this pls
	if (stmt_define_ast->define_type.t == Type::Types::FN) {
		llvm::Function* fn = (llvm::Function*)stmt_define_ast->value->visit(this);
		sym_table.add_symbol(stmt_define_ast->identifier.value, SymTableEntry(fn, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->is_constant));
	}
	return NULL;
}
void* LLVMCodeGen::visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_assign(StmtAssignAST* stmt_assign_ast) {
	// @TODO this assumes the variable decleration e.g. x : s32 was an alloca and not a global or a malloc etc
	auto val = (llvm::Value*)stmt_assign_ast->value->visit(this);
	auto ptr = (llvm::Value*)stmt_assign_ast->assignee->visit(this);
	//auto ptr = (llvm::Value*)sym_table.get_symbol(stmt_assign_ast->variable.value).optional_data;
	auto is_volative = false;
	llvm_builder->CreateStore(val, ptr, is_volative);
	return NULL;
}
void* LLVMCodeGen::visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_return(StmtReturnAST* stmt_return_ast) {
	llvm_builder->CreateRet((llvm::Value*)stmt_return_ast->value->visit(this));
	return NULL;
}
void* LLVMCodeGen::visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_break_ast(StmtBreakAST* stmt_break_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_stmt_if_ast(StmtIfAST* stmt_if_ast) {
	auto infered_type = infer_type(stmt_if_ast->if_cond);
	// different compares for integer/float
	if (infered_type.is_integer_type()) {
		// convert the condition to a bool


		auto cmp = llvm_builder->CreateICmpEQ(
			(llvm::Value*)stmt_if_ast->if_cond->visit(this),
			llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(*llvm_context), 1),
			"test_if_block"
		);

		llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*llvm_context, "then");
		llvm::BasicBlock* else_block = llvm::BasicBlock::Create(*llvm_context, "else");
		llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(*llvm_context, "ifcont");

		llvm_builder->CreateCondBr(cmp, then_block, else_block);

		auto prev_block = llvm_builder->GetInsertPoint();
		// add the blocks to the fn 
		llvm_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(then_block);
		llvm_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(else_block);
		llvm_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(merge_block);

		llvm_builder->SetInsertPoint(then_block);
		stmt_if_ast->if_stmt->visit(this);
		llvm_builder->CreateBr(merge_block);

		if (stmt_if_ast->has_else) {
			llvm_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(else_block);
			llvm_builder->SetInsertPoint(else_block);
			stmt_if_ast->else_stmt->visit(this);
			llvm_builder->CreateBr(merge_block);
		}
		else {
			llvm_builder->SetInsertPoint(else_block);
			llvm_builder->CreateBr(merge_block);
		}

		llvm_builder->SetInsertPoint(merge_block);
	}
	else if (infered_type.is_float_type()) {
		// convert the condition to a bool
		llvm::Value* cond_value = (llvm::Value*)stmt_if_ast->if_cond->visit(this);
		auto bool_condition = llvm_builder->CreateFCmpONE(
			cond_value, llvm::ConstantFP::get(*llvm_context, llvm::APFloat(0.0)), "ifcond");
	}
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
	
	if (expr_fn_ast->has_body) {
		llvm::BasicBlock* bb = llvm::BasicBlock::Create(*llvm_context, "entry_block", f);
		llvm_builder->SetInsertPoint(bb);

		// code gen the fn body
		expr_fn_ast->body->visit(this);

		if(!expr_fn_ast->full_type.fn_signature.has_return)
			llvm_builder->CreateRetVoid();

		llvm_builder->ClearInsertionPoint();
		llvm::verifyFunction(*f);
	}

	// add the fn type to the symbol table
	sym_table.add_symbol(expr_fn_ast->full_type.fn_signature.anonymous_identifier, SymTableEntry(f, &expr_fn_ast->full_type, 0,0));
	return f;
}



void* LLVMCodeGen::visit_expr_cast_ast(ExprCastAST* expr_cast_ast) {

	auto value = (llvm::Value*)expr_cast_ast->value->visit(this);
	if (expr_cast_ast->niavely_resolved)
		return value;

	auto l_type = expr_cast_ast->from_type;
	auto r_type = expr_cast_ast->to_type;
	llvm::Instruction::CastOps cast_instr;
	
	// check if the cast has been resolved at compile time

	
	return llvm_builder->CreateCast(llvm::Instruction::CastOps::SIToFP, value, convert_type(r_type));
}

void* LLVMCodeGen::visit_expr_call_ast(ExprCallAST* expr_call_ast) {
	auto fn = (llvm::Function*)(expr_call_ast->callee->visit(this));
	return llvm_builder->CreateCall(fn, llvm::None);
}

void* LLVMCodeGen::visit_expr_var_ast(ExprVarAST* expr_var_ast) {

	// the problem here is that a variable can be a load, store etc

	auto var_type = sym_table.get_symbol(expr_var_ast->identifier.value).type;

	switch (var_type->t) {
		case Type::Types::FN: {
			auto fn_type = (llvm::Function*)sym_table.get_symbol(expr_var_ast->identifier.value).optional_data;
			return fn_type;
		}
		default: {
			return (llvm::Value*)sym_table.get_symbol(expr_var_ast->identifier.value).optional_data;
			//// create a load instruction
			//auto instr = (llvm::StoreInst*)sym_table.get_symbol(expr_var_ast->identifier.value).optional_data;
			//return llvm_builder->CreateLoad(instr);
		}
	}
}
void* LLVMCodeGen::visit_expr_interface_get_ast(ExprInterfaceGetAST* expr_interface_get_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_bin_ast(ExprBinAST* expr_bin_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_un_ast(ExprUnAST* expr_un_ast) {

	switch (expr_un_ast->op.type) {
	case Token::Type::POINTER: {
		//@TODO implement me!
		// we first need to get the type that the load is of 
		auto value = (llvm::StoreInst*)expr_un_ast->ast->visit(this);
		return llvm_builder->CreateLoad(value);
		break;
	}
	}

	return NULL;
}
void* LLVMCodeGen::visit_expr_group_ast(ExprGroupAST* expr_group_ast) {
	return NULL;
}
void* LLVMCodeGen::visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) {
	switch (expr_literal_ast->t.t) {
	case Type::Types::U8: return llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(*llvm_context), expr_literal_ast->v.v.as_u8); break;
	case Type::Types::U16: return llvm::ConstantInt::getSigned(llvm::Type::getInt16Ty(*llvm_context), expr_literal_ast->v.v.as_u16); break;
	case Type::Types::U32: return llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*llvm_context), expr_literal_ast->v.v.as_u32); break;
	case Type::Types::S32: return llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*llvm_context), expr_literal_ast->v.v.as_s32); break;
	case Type::Types::S64: return llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(*llvm_context), expr_literal_ast->v.v.as_s64); break;
	case Type::Types::F32: return llvm::ConstantInt::getSigned(llvm::Type::getFloatTy(*llvm_context), expr_literal_ast->v.v.as_f32); break;
	case Type::Types::F64: return llvm::ConstantInt::getSigned(llvm::Type::getDoubleTy(*llvm_context), expr_literal_ast->v.v.as_s64); break;
	case Type::Types::CHAR: return llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(*llvm_context), expr_literal_ast->v.v.as_u8); break;
	}
	return (void*)llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*llvm_context), 123);
}

void* LLVMCodeGen::visit_expr_literal_array_ast(ExprLiteralArrayAST* expr_literal_array_ast) {
	std::vector<llvm::Constant*> constants;
	for (const auto& val : expr_literal_array_ast->values) {
		constants.push_back((llvm::Constant * )val->visit(this));
	}
	llvm::ArrayRef<llvm::Constant*> array_ref(constants);
	llvm::ArrayType* array_type = llvm::ArrayType::get(convert_type(expr_literal_array_ast->contained_type), expr_literal_array_ast->array_type.arr_length);
	// note we do t.t here as the array type will be u8 array size 123, and we just want u8
	return llvm::ConstantArray::get(array_type, array_ref);
}