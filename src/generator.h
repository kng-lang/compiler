#pragma once

#include "common.h"
#include "ast.h"

struct CodeGen {}; //: public ASTVisitor {};

struct LLVMCodeGen : public CodeGen {};

struct CLRCodeGen : public CodeGen{};

struct Generator {
	std::shared_ptr<AST> ast;
	Generator(std::shared_ptr<AST> ast) : ast(ast){}
	virtual std::shared_ptr<CodeGen> generate(std::shared_ptr<AST> ast) = 0;
};

struct LLVMGenerator : public Generator {
	LLVMGenerator(std::shared_ptr<AST> ast) : Generator(ast) {}
	virtual std::shared_ptr<CodeGen> generate();
};

struct CLRGenerator : public Generator {
	CLRGenerator(std::shared_ptr<AST> ast) : Generator(ast) {}
	virtual std::shared_ptr<CodeGen> generate();
};