#include "types.h"
#include "ast.h"

Type u8_type() {
	return Type(Type::Types::U8);
}

Type infer_type(std::shared_ptr<AST> ast){
	return u8_type();
}

void SymTable::add_symbol(std::string identifier, SymTableEntry entry) {
	entries[identifier] = entry;
}

std::shared_ptr<SymTable> SymTable::enter_scope(std::shared_ptr<SymTable> parent_sym_table){
	return enter_scope("block", parent_sym_table);
}

std::shared_ptr<SymTable> SymTable::enter_scope(std::string identifier, std::shared_ptr<SymTable> parent_sym_table){
	auto new_table = std::make_shared<SymTable>();
	SymTableEntry entry(std::make_shared<SymTable>(parent_sym_table));
	add_symbol(identifier, entry);
	return new_table;
}

std::shared_ptr<SymTable> SymTable::pop_scope() {
	return parent_sym_table;
}