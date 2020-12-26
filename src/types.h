#pragma once

#include <memory>
#include <map>
#include "common.h"

struct Type;
struct AST;

// the signature of an interface is made up of the type of its members
struct InterfaceSignature{
	std::vector<Type> members;
};

struct FnSignature {
	// operation types are the types of the params and the return value
	std::vector<Type> operation_types;
};

struct Type {

	const static char* debug_types[];

	enum Types{
		UNKNOWN,
		U8,
		U16,
		U32,
		U64,
		S32,
		S64,
		F32,
		F64,
		CHAR,
		STRING,
		INTERFACE,
		SEQ,		// sequence of types
		ANY,		// _
	};
	Types t;
	// e.g. ^u8
	u8 ref = 0;
	// e.g. u8[5]
	u8 arr = 0;
	u32 arr_length = 0;
	InterfaceSignature interface_signature;
	FnSignature fn_signature;

	Type(){}
	Type(Types t) : t(t){}
	Type(Types t, u8 ref) : t(t), ref(ref){}
	Type(Types t, u8 arr, u32 arr_length) : t(t), arr(arr), arr_length(arr_length) {}

	std::string to_json();

	// matches basic determines whether a type's type (e.g. u8, interface, etc) is the same
	u8 matches_basic(Type other);
	// matches deep determines whether a type's full type signature matches (e.g. do the members match etc)
	u8 matches_deep(Type other);
};

struct Value {
	union v {
		u8  as_u8;
		u16 as_16;
		u32 as_u32;
		s32 as_s32;
		s64 as_s64;
		f32 as_f32;
		f64 as_f64;
	} v;
};

struct SymTable;

struct SymTableEntry {

	enum EntryType {
		TYPE,
		SYM_TABLE,
	};

	SymTableEntry(){}
	SymTableEntry(Type t) : entry_type(TYPE), type(t) {}
	SymTableEntry(std::shared_ptr<SymTable> sym_table) 
		: entry_type(SYM_TABLE), sym_table(sym_table) {}

	u8 entry_type;
	// @TODO these 2 should be in a union
	Type type;
	std::shared_ptr<SymTable> sym_table;
};

struct SymTable {
	std::unordered_map<std::string, SymTableEntry> entries;
	std::shared_ptr<SymTable> parent_sym_table;

	SymTable(){}
	SymTable(std::shared_ptr<SymTable> parent_sym_table) : parent_sym_table(parent_sym_table){}

	void add_symbol(std::string identifier, SymTableEntry entry);
	std::shared_ptr<SymTable> enter_scope(std::shared_ptr<SymTable> parent_sym_table);
	std::shared_ptr<SymTable> enter_scope(std::string identifier, std::shared_ptr<SymTable> parent_sym_table);
	std::shared_ptr<SymTable> pop_scope();
};

extern Type infer_type(std::shared_ptr<AST> ast);