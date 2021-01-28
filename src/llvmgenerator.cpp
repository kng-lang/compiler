/*
James Clarke - 2021
*/

#include <filesystem>
#include <iostream>
#include "llvmgenerator.h"
#include "compiler.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/CodeGen/MIRParser/MIRParser.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/AutoUpgrade.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"

void LLVMGenerator::generate() {
	this->m_context = std::make_unique<llvm::LLVMContext>();
	this->m_builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*m_context));
	this->m_module = std::make_unique<llvm::Module>(m_unit->m_compile_file.m_file_path, *m_context);

	make_runtime();

	this->m_ast->visit(this);


	using namespace llvm;

	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmPrinters();
	InitializeAllAsmParsers();

	auto target_triple = sys::getDefaultTargetTriple();


	kng_log("target triple: {}", target_triple);

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

	m_module->setDataLayout(target_machine->createDataLayout());
	m_module->setTargetTriple(target_triple);

	auto curr_path = std::filesystem::current_path().string();
	std::stringstream ss;
	ss << curr_path << "/output";

	std::error_code ec;

	raw_fd_ostream dest(ss.str(), ec, sys::fs::OF_None);

	if (ec) {
		kng_errr("Could not open file: {}", ec.message());
		return;
	}

	legacy::PassManager pass;
	llvm::CodeGenFileType file_type;
	switch (m_unit->m_compile_options.m_build_target) {
		case CompileOptions::BuildTarget::ASSEMBLY: file_type = CGFT_AssemblyFile; break;
		case CompileOptions::BuildTarget::OBJECT: 
		case CompileOptions::BuildTarget::EXEC:
			file_type = CGFT_ObjectFile; break;
	}
		
	
    // https://llvm.org/docs/Passes.html
	if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
		kng_errr("TargetMachine can't emit a file of this type");
		return;
	}
	if (m_unit->m_compile_options.m_optimise) {
		// disabled for debugging!
		//pass.add(llvm::createCodeGenPreparePass());
		//pass.add(llvm::createPromoteMemoryToRegisterPass());
		//pass.add(llvm::createLoopFlattenPass());
		//pass.add(llvm::createDeadCodeEliminationPass());
		//pass.add(llvm::createDeadStoreEliminationPass());
	}

	pass.run(*m_module);

	if (this->m_unit->m_compile_options.m_debug_flags & EMIT_IR_DEBUG)
		m_module->print(llvm::errs(), nullptr);



	dest.flush();
}

void LLVMGenerator::make_runtime() {
	auto type_interface = Type::create_interface(Token::create("type"), {
		{Token::create("__name__"), Type::create_pointer(Type::Types::U8, 1)},
		{Token::create("__size__"), Type::create_basic(Type::Types::U32)}
	});

	auto type = create_interface_definition("type", type_interface);
	m_sym_table.add(Token::create("type"), SymEntry(type, type_interface));
}


void LLVMGenerator::optimise(){}



llvm::Value* cast_to_type(llvm::Value* value, llvm::Type* type) {
	return NULL;
}



llvm::FunctionType* LLVMGenerator::create_fn_type(Type type) {
	std::vector<llvm::Type*> param_types;
	for (s32 i = 1; i < type.m_fn_operation_types.size(); i++)
		param_types.push_back(convert_type(type.m_fn_operation_types.at(i)));
	// convert to fn pointer
	return llvm::FunctionType::get(
		convert_type(type.m_fn_operation_types.at(0)),
		param_types,
		false);
}

llvm::StructType* LLVMGenerator::create_interface_type(Type type) {
	// this assumes that the interface type has already been created using an expr_inter_ast
	llvm::StructType* t = llvm::StructType::getTypeByName(*m_context, llvm::StringRef(type.m_interface_identifier.m_value));
	if (!t) {
		// create an interface type here
		kng_assert(false, "interface type doesn't exist");
	}

	return t;
}

llvm::Value* LLVMGenerator::alloc_interface(std::string identifier, std::vector<llvm::Value*> values) {
	auto interface_type = llvm::StructType::getTypeByName(*m_context, identifier);
	// now we allocate it on the stack
	auto allocate = m_builder->CreateAlloca(interface_type);
	return NULL;
}

llvm::Value* LLVMGenerator::create_str_constant(std::string s) {
	if (m_string_constants.count(s)) {
		return m_string_constants[s];
	}
	else {
		auto value = m_builder->CreateGlobalStringPtr(s);
		m_string_constants[s] = value;
		return value;
	}

}

void LLVMGenerator::set_interface_member(llvm::Type* type, llvm::Value* interface_value, u32 ptr_index, u32 member_index, llvm::Value* value, bool is_volatile) {
	auto element = get_interface_member(type, interface_value, ptr_index, member_index);
	m_builder->CreateStore(value, element);
}

llvm::Value* LLVMGenerator::get_interface_member(llvm::Type* type, llvm::Value* interface_value, u32 ptr_index, u32 member_index) {
	return m_builder->CreateGEP(type, interface_value, {
		llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*m_context), ptr_index),
		llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*m_context), member_index)});
}

inline llvm::StructType* LLVMGenerator::create_interface_definition(std::string name, Type type) {
	// create the struct and the virtual table for the struct
	auto n = llvm::StringRef(name);
	auto interface_type = llvm::StructType::create(*m_context, n);

	std::vector<llvm::Type*> members;
	for (s32 i = 0; i < type.m_interface_members.size(); i++) {
		members.push_back(convert_type(type.m_interface_members[i]));
	}
	interface_type->setBody(llvm::ArrayRef<llvm::Type*>(members));
	auto interface_vtable = llvm::StructType::create(*m_context, llvm::StringRef("vtable_" + name));
	return interface_type;
}

llvm::Type* LLVMGenerator::convert_type(Type type) {
	llvm::Type* tmp_type = NULL;
	switch (type.m_type) {
		case Type::Types::U0:     tmp_type = llvm::Type::getVoidTy(*m_context); break;
		case Type::Types::U8:     tmp_type = llvm::Type::getInt8Ty(*m_context); break;
		case Type::Types::S8:     tmp_type = llvm::Type::getInt8Ty(*m_context); break;
		case Type::Types::U16:    tmp_type = llvm::Type::getInt16Ty(*m_context); break;
		case Type::Types::S16:    tmp_type = llvm::Type::getInt16Ty(*m_context); break;
		case Type::Types::U32:    tmp_type = llvm::Type::getInt32Ty(*m_context); break;
		case Type::Types::S32:    tmp_type = llvm::Type::getInt32Ty(*m_context); break;
		case Type::Types::S64:    tmp_type = llvm::Type::getInt64Ty(*m_context); break;
		case Type::Types::F32:    tmp_type = llvm::Type::getFloatTy(*m_context); break;
		case Type::Types::F64:    tmp_type = llvm::Type::getDoubleTy(*m_context); break;
		case Type::Types::CHAR:   tmp_type = llvm::Type::getInt8Ty(*m_context); break; // ASCII FOR NOW?
		case Type::Types::STRING: tmp_type = llvm::Type::getInt8PtrTy(*m_context); break; // @TODO return a reference to the string interface using the symbol table
		case Type::Types::FN: {
			tmp_type = create_fn_type(type)->getPointerTo();
			break;
		}
		case Type::Types::INTERFACE: {
			tmp_type = create_interface_type(type);  
			break; 
		}
		case Type::Types::TYPE: {
			// the 'type' type is actually an interface
			tmp_type = llvm::StructType::getTypeByName(*m_context, llvm::StringRef("type"));
			break;
		}
	}
	// its only an array and not a ptr if the arr_length > 0 otherwise x : u8[] is a ptr
	if (type.m_is_arr && type.m_arr_length > 0)
		tmp_type = llvm::ArrayType::get(tmp_type, type.m_arr_length);
	if (type.m_ptr_indirection) {
		for (int i = 0; i < type.m_ptr_indirection; i++)
			tmp_type = llvm::PointerType::getUnqual(tmp_type);
	}
	kng_assert(tmp_type, "llvm converted type is NULL");
	return tmp_type;
}

llvm::Value* LLVMGenerator::convert_fetched_to_value() {
	switch (m_fetched_type) {
		case FetchedType::VARIABLE: {
			m_fetched_value = m_builder->CreateLoad(m_fetched_value);
			m_fetched_type = FetchedType::VALUE;
			break;
		}
		// if the fetched type is already a value then return
		case FetchedType::VALUE: break;
		case FetchedType::FN: break;
	}
	return NULL;
}


void* LLVMGenerator::visit_program(ProgramAST* program_ast){
	for (const auto& stmt : program_ast->stmts)
		stmt->visit(this);
	return NULL;
}
void* LLVMGenerator::visit_stmt_block(StmtBlockAST* stmt_block_ast) {
	m_sym_table.enter_anon();
	for (const auto& stmt : stmt_block_ast->stmts)
		stmt->visit(this);
	m_sym_table.exit();
	return NULL;
}
void* LLVMGenerator::visit_stmt_expression(StmtExpressionAST* stmt_expression_ast) {
	return NULL;
}
void* LLVMGenerator::visit_stmt_define(StmtDefineAST* stmt_define_ast) {
	auto is_volatile = false;
	if (stmt_define_ast->m_is_initialised) {
		stmt_define_ast->value->visit(this);
		convert_fetched_to_value();
	}



	void* creation_instr = NULL;

	// if we are dealing with a type, then just visit the type
	if (stmt_define_ast->define_type.m_type != Type::Types::TYPE) {


		auto define_type = convert_type(stmt_define_ast->define_type);
		if (!stmt_define_ast->m_is_underscore && !(stmt_define_ast->define_type.m_type==Type::Types::FN && stmt_define_ast->m_is_constant)) {


			creation_instr = m_builder->CreateAlloca(define_type, NULL, stmt_define_ast->identifier.m_value);

			if (stmt_define_ast->m_is_initialised)
				m_builder->CreateStore(m_fetched_value, (llvm::Value*)creation_instr, is_volatile);
		}
	}
	else {
		creation_instr = m_fetched_value;
	}


	// finally add to the symbol table
	m_sym_table.add(stmt_define_ast->identifier, SymEntry(creation_instr, stmt_define_ast->define_type));



	



	//llvm::Value* creation_instr = NULL;
	//// only handle the definition if it isn't an underscore
	//if (!stmt_define_ast->m_is_underscore) {
	//	llvm::Type* define_type = convert_type(stmt_define_ast->define_type);
	//
	//	if (!stmt_define_ast->m_is_global) {
	//
	//		creation_instr = m_builder->CreateAlloca(define_type, NULL, stmt_define_ast->identifier.m_value);
	//
	//		switch (stmt_define_ast->define_type.m_type) {
	//			case Type::Types::NAMESPACE: {
	//				break;
	//			}
	//			// if we are dealing with a type decleration, then we need to assign the type variable
	//			case Type::Types::TYPE: {
	//				
	//				set_interface_member(define_type, creation_instr, 0, 0, create_str_constant("test"), false);
	//				set_interface_member(define_type, creation_instr, 0, 1, llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*m_context), 123), false);
	//
	//				break;
	//			}
	//			default: {
	//				if(stmt_define_ast->m_is_initialised)
	//					m_builder->CreateStore(m_fetched_value, (llvm::Value*)creation_instr, is_volatile);
	//				break;
	//			}
	//		}
	//		m_sym_table.add(stmt_define_ast->identifier, SymEntry(creation_instr, stmt_define_ast->define_type));
	//	}
	//	else {
	//		//creation_instr = m_module->getOrInsertGlobal(llvm::StringRef(stmt_define_ast->identifier.m_value), define_type);
	//		//m_global_expressions.push(m_builder->CreateStore(m_fetched_value, (llvm::Constant*)creation_instr, is_volatile));
	//	}
	//}
	return NULL;















	//if (stmt_define_ast->m_is_underscore
	//	&& stmt_define_ast->m_is_initialised) {
	//	stmt_define_ast->value->visit(this);
	//	return NULL;
	//}
	//
	//
	//
	//
	//if (!stmt_define_ast->is_global) {
	//	// do alloca
	//	if (
	//		stmt_define_ast->define_type.m_type != Type::Types::FN
	//		&& stmt_define_ast->define_type.m_type != Type::Types::INTERFACE
	//		&& stmt_define_ast->define_type.m_type != Type::Types::TYPE
	//		) {
	//		creation_instr = m_builder->CreateAlloca(convert_type(stmt_define_ast->define_type), NULL, stmt_define_ast->identifier.m_value);
	//		m_sym_table.add_symbol(stmt_define_ast->identifier, SymTableEntry(creation_instr, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->m_is_constant));
	//	}
	//}
	//else {
	//	if (
	//		stmt_define_ast->define_type.m_type != Type::Types::FN
	//		&& stmt_define_ast->define_type.m_type != Type::Types::INTERFACE
	//		&& stmt_define_ast->define_type.m_type != Type::Types::TYPE
	//		) {
	//		creation_instr = m_module->getOrInsertGlobal(llvm::StringRef(stmt_define_ast->identifier.m_value), convert_type(stmt_define_ast->define_type));
	//		m_sym_table.add_symbol(stmt_define_ast->identifier, SymTableEntry(creation_instr, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->m_is_constant));
	//	}
	//	else {
	//
	//	}
	//}
	//
	//// @TODO this works if it isn't a function...
	//if (stmt_define_ast->m_is_initialised
	//	&& stmt_define_ast->define_type.m_type != Type::Types::FN
	//	&& stmt_define_ast->define_type.m_type != Type::Types::INTERFACE
	//	&& stmt_define_ast->define_type.m_type != Type::Types::TYPE) {
	//	stmt_define_ast->value->visit(this);
	//	if (!stmt_define_ast->m_is_underscore) {
	//		auto is_volative = false;
	//		kng_assert(creation_instr != NULL, "creation_instr was null");	
	//		// prepare the value for assignment
	//		convert_fetched_to_value();
	//		m_builder->CreateStore(m_fetched_value, (llvm::Value*)creation_instr, is_volative);
	//	}
	//}
	//
	//// @TODO jesus fix this pls
	//if (stmt_define_ast->define_type.m_type == Type::Types::FN) {
	//	stmt_define_ast->value->visit(this);
	//	m_sym_table.add_symbol(stmt_define_ast->identifier, SymTableEntry((llvm::Function*)m_fetched_value, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->m_is_constant));
	//}
	//
	//
	//// @TODO for now this only supports interface define types e.g. vec : type = interface {};
	//if(stmt_define_ast->define_type.m_type==Type::Types::TYPE){
	//	stmt_define_ast->value->visit(this);
	//	//	m_sym_table.add_symbol(stmt_define_ast->identifier, SymTableEntry((llvm::StructType*)m_fetched_value, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->is_constant));
	//}
	//
	//
	//
	//if (stmt_define_ast->define_type.m_type == Type::Types::INTERFACE) {
	//
	//	// first get the llvm::StructType* from the define type e.g. x : vec
	//	//llvm::StructType* type = (llvm::StructType*)m_sym_table.get_symbol(stmt_define_ast->define_type.m_interface_identifier).optional_data;
	//	//auto creation_instr = m_builder->CreateAlloca(type);
	//	//if (stmt_define_ast->is_initialised) {
	//	//	stmt_define_ast->value->visit(this);
	//	//	convert_fetched_to_value();
	//	//	m_builder->CreateStore(m_fetched_value, (llvm::Value*)creation_instr, false);
	//	//}
	//
	//	//stmt_define_ast->value->visit(this);
	//	//m_sym_table.add_symbol(stmt_define_ast->identifier, SymTableEntry((llvm::StructType*)m_fetched_value, &stmt_define_ast->define_type, stmt_define_ast->is_global, stmt_define_ast->is_constant));
	//}
	//return NULL;
}
void* LLVMGenerator::visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) {
	return NULL;
}
void* LLVMGenerator::visit_stmt_assign(StmtAssignAST* stmt_assign_ast) {

	// @TODO this assumes the variable decleration e.g. x : s32 was an alloca and not a global or a malloc etc
	stmt_assign_ast->value->visit(this);
	// prepare the value for assignment
	convert_fetched_to_value();
	auto val = m_fetched_value;

	stmt_assign_ast->assignee->visit(this);
	auto ptr = m_fetched_value; // what if we are assigning to a deref??? ^x = 1

	//auto ptr = (llvm::Value*)sym_table.get_symbol(stmt_assign_ast->variable.value).optional_data;
	auto is_volatile = false;
	m_builder->CreateStore(val, ptr, is_volatile);
	return NULL;
}
void* LLVMGenerator::visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast) {
	return NULL;
}
void* LLVMGenerator::visit_stmt_return(StmtReturnAST* stmt_return_ast) {
	stmt_return_ast->value->visit(this);
	convert_fetched_to_value();
	m_builder->CreateRet(m_fetched_value);
	return NULL;
}
void* LLVMGenerator::visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast) {
	return NULL;
}
void* LLVMGenerator::visit_stmt_break_ast(StmtBreakAST* stmt_break_ast) {
	kng_assert(m_exit_block, "exit_block was null");
	m_builder->CreateBr(m_exit_block);
	return NULL;
}
void* LLVMGenerator::visit_stmt_if_ast(StmtIfAST* stmt_if_ast) {
	auto infered_type = infer_type(stmt_if_ast->if_cond);
	// different compares for integer/float
	if (infered_type.is_integer_type()) {
		// convert the condition to a bool

		stmt_if_ast->if_cond->visit(this);
		convert_fetched_to_value();
		auto cmp = m_builder->CreateICmpEQ(
			m_fetched_value,
			llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(*m_context), 1),
			"test_if_block"
		);

		llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*m_context, "then");
		llvm::BasicBlock* else_block = llvm::BasicBlock::Create(*m_context, "else");
		llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(*m_context, "ifcont");

		m_builder->CreateCondBr(cmp, then_block, else_block);

		auto prev_block = m_builder->GetInsertPoint();
		// add the blocks to the fn 
		m_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(then_block);
		m_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(else_block);
		m_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(merge_block);

		m_builder->SetInsertPoint(then_block);
		stmt_if_ast->if_stmt->visit(this);
		m_builder->CreateBr(merge_block);

		if (stmt_if_ast->has_else) {
			m_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(else_block);
			m_builder->SetInsertPoint(else_block);
			stmt_if_ast->else_stmt->visit(this);
			m_builder->CreateBr(merge_block);
		}
		else {
			m_builder->SetInsertPoint(else_block);
			m_builder->CreateBr(merge_block);
		}

		m_builder->SetInsertPoint(merge_block);
	}
	else if (infered_type.is_float_type()) {
		// convert the condition to a bool
		stmt_if_ast->if_cond->visit(this);
		convert_fetched_to_value();
		auto bool_condition = m_builder->CreateFCmpONE(
			m_fetched_value, llvm::ConstantFP::get(*m_context, llvm::APFloat(0.0)), "ifcond");
	}
	return NULL;
}
void* LLVMGenerator::visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast) {

	// first create the start block
	llvm::BasicBlock* body_block = llvm::BasicBlock::Create(*m_context, "then");
	llvm::BasicBlock* loop_end_block = llvm::BasicBlock::Create(*m_context, "end");
	this->m_exit_block = loop_end_block;
	
	m_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(body_block);
	m_builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(loop_end_block);


	if (stmt_loop_ast->loop_type == StmtLoopAST::LoopType::INF) {
		m_builder->CreateBr(body_block);
		auto prev_insert = m_builder->GetInsertPoint();
		m_builder->SetInsertPoint(body_block);
		stmt_loop_ast->body->visit(this);
		m_builder->CreateBr(body_block);
		m_builder->SetInsertPoint(loop_end_block);
	}
	return NULL;
}


void* LLVMGenerator::visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast){

	// when we visit an interface ast, we are creating an interface instance interface
	// e.g. interaface {
	//			xyz : f32;
	//		}



	// create the struct and the virtual table for the struct
	auto name = llvm::StringRef(expr_interface_ast->m_lambda_name.m_value);
	auto interface_type = llvm::StructType::create(*m_context, name);

	std::vector<llvm::Type*> members;
	for (s32 i = 0; i < expr_interface_ast->m_definitions.size(); i++){
		members.push_back(convert_type(expr_interface_ast->m_type.m_interface_members[i]));
	}
	interface_type->setBody(llvm::ArrayRef<llvm::Type*>(members));
	auto interface_vtable = llvm::StructType::create(*m_context, llvm::StringRef("vtable_"+expr_interface_ast->m_lambda_name.m_value));
	
	//m_sym_table.add_symbol(
	//	expr_interface_ast->m_lambda_name,
	//	SymTableEntry(interface_type, &expr_interface_ast->m_full_type, 0, 0)
	//);

	//m_fetched_value = interface_type;
	
	return NULL;
}

void* LLVMGenerator::visit_expr_fn_ast(ExprFnAST* expr_fn_ast) {
	auto prev_insert_point = m_builder->GetInsertBlock();

	llvm::FunctionType* ft = create_fn_type(expr_fn_ast->m_type);
	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, expr_fn_ast->m_lambda_name.m_value, *m_module);

	m_sym_table.add_enter(expr_fn_ast->m_lambda_name, SymEntry(f, expr_fn_ast->m_type));
	if (expr_fn_ast->has_body) {
		llvm::BasicBlock* bb = llvm::BasicBlock::Create(*m_context, "entry_block", f);
		m_builder->SetInsertPoint(bb);

		// add the paramater to the sym table
		u32 i = 0;
		for (auto arg = f->arg_begin(); arg != f->arg_end(); arg++) {
			auto stmt_define_arg = std::static_pointer_cast<StmtDefineAST>(expr_fn_ast->params.at(i));
			//// set the name of the argument & add it to the symbol table
			//arg->setName(stmt_define_arg->identifier.m_value);
			//auto arg_name = stmt_define_arg->identifier.m_value;
			//m_sym_table.add_symbol(arg_name, SymTableEntry(arg, &stmt_define_arg->define_type, 0, 0));

			/*
					the above is wrong, if we want to use an arg we should allocate it on the stack
					for the following:

					x : (a : s32){
						b := a;
					}

					0 = allocate s32 // create storage for a
					store a, 0		 // store a in 0
					b = allocate s32 // create storage for b
					1 = load 0		 // load 0 value
			*/

			// create stack storage for the argument
			auto arg_name = stmt_define_arg->identifier;
			auto alloca_instr = m_builder->CreateAlloca(arg->getType(), NULL, arg_name.m_value);
			// initialise it
			auto is_volatile = false;
			m_builder->CreateStore(arg, alloca_instr, is_volatile);
			m_sym_table.add(arg_name, SymEntry(alloca_instr, stmt_define_arg->define_type));
			i++;
		}

		// if this is main then insert the global expressions
		if (expr_fn_ast->m_lambda_name.m_value.compare("main") == 0) {
			while (!m_global_expressions.empty()) {
				m_global_expressions.top()->visit(this);
				m_global_expressions.pop();
			}
		}
		// code gen the fn body
		expr_fn_ast->body->visit(this);
		if (!expr_fn_ast->m_type.m_fn_has_return)
			m_builder->CreateRetVoid();
	
		//m_builder->ClearInsertionPoint();
		m_builder->SetInsertPoint(prev_insert_point);
	}
	llvm::verifyFunction(*f);

	m_sym_table.exit();
	// add the fn type to the symbol table
	//m_sym_table.add_symbol(expr_fn_ast->m_lambda_name, SymTableEntry(f, &expr_fn_ast->m_type, 0,0));
	m_fetched_value = f;
	return NULL;
}



void* LLVMGenerator::visit_expr_cast_ast(ExprCastAST* expr_cast_ast) {

	expr_cast_ast->value->visit(this);
	convert_fetched_to_value();
	// check if the cast has been niavely_resolved at compile time
	if (expr_cast_ast->niavely_resolved)
		return m_fetched_value;
	// if it hasn't, then perform a runtime bitcast
	auto l_type = expr_cast_ast->from_type;
	auto r_type = expr_cast_ast->to_type;
	m_fetched_value = m_builder->CreateCast(llvm::Instruction::CastOps::SIToFP, m_fetched_value, convert_type(r_type));
	return NULL;
}

void* LLVMGenerator::visit_expr_call_ast(ExprCallAST* expr_call_ast) {
	expr_call_ast->callee->visit(this);
	llvm::Function* fn = (llvm::Function*)m_fetched_value;
	llvm::ArrayRef<llvm::Value*> arg_array;
	std::vector<llvm::Value*> args;
	if (expr_call_ast->has_args) {
		for (const auto& arg : expr_call_ast->args) {
			arg->visit(this);
			convert_fetched_to_value();
			args.push_back((llvm::Value*)m_fetched_value);
		}
		//std::vector<llvm::Value*> args = { (llvm::Value*)expr_call_ast->args->visit(this) };
		arg_array = llvm::ArrayRef<llvm::Value*>(args);
		kng_assert(arg_array.data(), "arg_array data was NULL");
	}
	else {
		arg_array = llvm::None;
	}

	m_fetched_value = m_builder->CreateCall(fn, arg_array);
	return NULL;
}

void* LLVMGenerator::visit_expr_var_ast(ExprVarAST* expr_var_ast) {
	// the problem here is that a variable can be a load, store etc
	auto var_type = m_sym_table.get(expr_var_ast->identifier)->m_type;
	switch (var_type.m_type) {
		case Type::Types::FN: {
			m_fetched_type = FetchedType::FN;
			m_fetched_value = (llvm::Function*)m_sym_table.get(expr_var_ast->identifier)->m_optional_data;
			break;
		}
		default: {
			m_fetched_type = FetchedType::VARIABLE;
			m_fetched_value = (llvm::StoreInst*)m_sym_table.get(expr_var_ast->identifier)->m_optional_data;
			break;
		}
	}
	return NULL;
}


void* LLVMGenerator::visit_expr_pattern_ast(ExprPatternAST* expr_pattern_ast){
	return NULL;
}

void* LLVMGenerator::visit_expr_interface_get_ast(ExprGetAST* expr_interface_get_ast) {
	switch (expr_interface_get_ast->m_get_type) {
		case ExprGetAST::GetType::INTERFACE: {
			// first fetch the value of the interface
			// we dont convert it to a load because we access the pointer directly
			expr_interface_get_ast->m_value->visit(this);
			m_fetched_value = get_interface_member(convert_type(expr_interface_get_ast->m_interface_type), m_fetched_value, 0, expr_interface_get_ast->m_idx);
			m_fetched_type = FetchedType::VALUE;
			return NULL;
		}
		case ExprGetAST::GetType::NAMESPACE: {
			// if we are getting a namespace, then simply return the value that the namespace points to
		}
										   
	}

	return NULL;
}
void* LLVMGenerator::visit_expr_bin_ast(ExprBinAST* expr_bin_ast) {

	// we cannot assume that the lhs & the rhs are the same type
	expr_bin_ast->lhs->visit(this);
	convert_fetched_to_value();
	auto lhs_value = m_fetched_value;
	expr_bin_ast->rhs->visit(this);
	convert_fetched_to_value();
	auto rhs_value = m_fetched_value;

	//!@TODO these should be in different functions
	switch(expr_bin_ast->op.m_type){
		case Token::Type::LOR: {
			return NULL;
		}
		case Token::Type::LAND: {
			return NULL;
		}
		case Token::Type::BOR: {
			m_fetched_value = m_builder->CreateOr(lhs_value, rhs_value);
			return NULL;
		}
		case Token::Type::BAND: {
			m_fetched_value = m_builder->CreateAnd(lhs_value, rhs_value);
			return NULL;
		}
		case Token::Type::PLUS: {
			llvm::Value* add_instr;
			if(expr_bin_ast->m_value_type==Type::Types::F32 
				|| expr_bin_ast->m_value_type == Type::Types::F64){
				add_instr = m_builder->CreateFAdd(lhs_value, rhs_value);
			} else {
				add_instr = m_builder->CreateAdd(lhs_value, rhs_value);
			}
			m_fetched_value = add_instr;
			return NULL;
		};
		case Token::Type::MINUS: {
			llvm::Value* add_instr;
			if (expr_bin_ast->m_value_type == Type::Types::F32
				|| expr_bin_ast->m_value_type == Type::Types::F64) {
				add_instr = m_builder->CreateFSub(lhs_value, rhs_value);
			} else {
				add_instr = m_builder->CreateSub(lhs_value, rhs_value);
			}
			m_fetched_value = add_instr;
			return NULL;
		};
		case Token::Type::STAR: {
			llvm::Value* add_instr;
			if (expr_bin_ast->m_value_type == Type::Types::F32
				|| expr_bin_ast->m_value_type == Type::Types::F64) {
				add_instr = m_builder->CreateFMul(lhs_value, rhs_value);
			} else {
				add_instr = m_builder->CreateMul(lhs_value, rhs_value);
			}
			m_fetched_value = add_instr; 
			return NULL;
		};
		case Token::Type::DIV: {
			llvm::Value* add_instr;
			if (expr_bin_ast->m_value_type == Type::Types::F32
				|| expr_bin_ast->m_value_type == Type::Types::F64) {
				add_instr = m_builder->CreateFDiv(lhs_value, rhs_value);
			} else {
				add_instr = m_builder->CreateFDiv(lhs_value, rhs_value);
			}
			m_fetched_value = add_instr;
			return NULL;
		};
		case Token::Type::LSHIFT: {
			m_fetched_value = m_builder->CreateShl(lhs_value, rhs_value);;
			return NULL;
		};

		case Token::Type::RSHIFT: {
			m_fetched_value = m_builder->CreateLShr(lhs_value, rhs_value);
			return NULL;
		};
			// 
	}



	return NULL;
}
void* LLVMGenerator::visit_expr_un_ast(ExprUnAST* expr_un_ast) {
	switch (expr_un_ast->op.m_type) {
		// we want to get the type of the next expression and allocate an interface
		case Token::Type::TYPEOF: {
			auto type_interface = alloc_interface("type", {});
			m_fetched_value = type_interface;
			m_fetched_type = FetchedType::VALUE;
			break;
		}
		case Token::Type::POINTER: {
			// we first visit the variable expression to set the stored_var_instr.
			// after, we then load the variable value.
			expr_un_ast->ast->visit(this);
			kng_assert(m_fetched_value, "fetched_value was null, likely attempting to dereference a non pointer");
			m_fetched_value = m_builder->CreateLoad(m_fetched_value);
			m_fetched_type = FetchedType::VALUE;
		}
		case Token::Type::BAND: {
			// we first visit the variable expression to set the stored_var_instr.
			// after, we then load the variable value.
			expr_un_ast->ast->visit(this);
			kng_assert(m_fetched_value, "stored_var_insr was null, likely attempting to dereference a non pointer");
			m_fetched_value = m_fetched_value;
			m_fetched_type = FetchedType::VALUE;
		}
	}
	return NULL;
}
void* LLVMGenerator::visit_expr_group_ast(ExprGroupAST* expr_group_ast) {
	expr_group_ast->expression->visit(this);
	return NULL;
}
void* LLVMGenerator::visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) {
	switch (expr_literal_ast->t.m_type) {
		case Type::Types::U8:  { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(*m_context),   expr_literal_ast->v.as_u8()); break; }
		case Type::Types::S8:  { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(*m_context), expr_literal_ast->v.as_s8()); break;   }
		case Type::Types::U16: { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt16Ty(*m_context), expr_literal_ast->v.as_u16()); break; }
		case Type::Types::S16: { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt16Ty(*m_context), expr_literal_ast->v.as_s16()); break; }
		case Type::Types::U32: { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*m_context), expr_literal_ast->v.as_u32()); break; }
		case Type::Types::S32: { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(*m_context), expr_literal_ast->v.as_s32()); break; }
		case Type::Types::S64: { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(*m_context), expr_literal_ast->v.as_s64()); break; }
		case Type::Types::F32: { m_fetched_value = llvm::ConstantFP::get(llvm::Type::getFloatTy(*m_context), expr_literal_ast->v.as_f32()); break; }
		case Type::Types::F64: { m_fetched_value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*m_context),expr_literal_ast->v.as_f64()); break; }
		case Type::Types::CHAR: { m_fetched_value = llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(*m_context), expr_literal_ast->v.as_char()); break;}
		case Type::Types::STRING: { 
			create_str_constant(expr_literal_ast->v.as_string());
			m_fetched_type = FetchedType::VALUE;
			
			return NULL; 
		}
	}
	m_fetched_type = FetchedType::VALUE;
	return NULL;
}

void* LLVMGenerator::visit_expr_literal_array_ast(ExprLiteralArrayAST* expr_literal_array_ast) {
	std::vector<llvm::Constant*> constants;
	for (const auto& val : expr_literal_array_ast->values) {
		constants.push_back((llvm::Constant * )val->visit(this));
	}
	llvm::ArrayRef<llvm::Constant*> array_ref(constants);
	llvm::ArrayType* array_type = llvm::ArrayType::get(convert_type(expr_literal_array_ast->contained_type), expr_literal_array_ast->array_type.m_arr_length);
	// note we do t.t here as the array type will be u8 array size 123, and we just want u8
	m_fetched_value = llvm::ConstantArray::get(array_type, array_ref);
	m_fetched_type = FetchedType::VALUE;
	return NULL;
}


void* LLVMGenerator::visit_expr_type_ast(ExprTypeAST* expr_type_ast) {
	// create a type structure here
	return NULL;
}

void* LLVMGenerator::visit_expr_included_ast(ExprIncludedAST* expr_included_ast) {
	expr_included_ast->m_ast->visit(this);
	return NULL;
}
void* LLVMGenerator::visit_expr_module_ast(ExprModuleAST* expr_module_ast) {
	return NULL;
}

void* LLVMGenerator::visit_stmt_defer_ast(StmtDeferAST* stmt_defer_ast) {
	return NULL;
}