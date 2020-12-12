#include "generator.h"


std::shared_ptr<CodeGen> LLVMGenerator::generate() {
	return std::make_shared<LLVMCodeGen>();
}

std::shared_ptr<CodeGen> CLRGenerator::generate() {
	return std::make_shared<CLRCodeGen>();
}