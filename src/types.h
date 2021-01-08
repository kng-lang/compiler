/*
James Clarke - 2021
*/

#pragma once

#include <variant>
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
	std::string anonymous_identifier;
	std::vector<Type> operation_types;
	u8 has_return = 0;
};

struct Pattern {

};

struct Type {

	const static char* debug_types[];

	enum class Types{
		UNKNOWN,
		TYPE,    
		// A type is used at compile time as a type alias
		// e.g. x : f32; y : x = 1.233;
		// At runtime, the type contains type information (made up by a structure)
		// e.g. x : type = f32; printf("%s\n", x.name); will print f32
		// e.g. x : type = f32; printf("%s\n", x.size); will print 32
		NAMESPACE,
		U0,
		U8,
		S8,
		U16,
		S16,
		U32,
		U64,
		S32,
		S64,
		F32,
		F64,
		CHAR,
		STRING,
		FN,
		INTERFACE,
		PATTERN,		// sequence of types
	};
	Types t = Types::UNKNOWN;
	
	// if this type is a generic in a fn e.g. (x : $generic){}, here x would have type generic.
	// the compiler will then resolve this generic type at compile time
	u8 is_generic = 0;

	u8 constant = 0;
	// e.g. ^^^u8
	u8 ptr_indirection = 0;
	// e.g. u8[5]
	u8 is_arr = 0;
	u8 pattern = 0;
	u8 is_constant_str; // for when we have ""?
	u32 arr_length = 0;


	u8 is_constant = 0;
	u8 is_global = 0;

	std::vector<Type> patterns;
	InterfaceSignature interface_signature;
	FnSignature fn_signature;

	Type(){}
	Type(Types t) : t(t){}
	Type(Types t, u8 ptr_indirection) : t(t), ptr_indirection(ptr_indirection){}
	Type(Types t, u8 is_arr, u32 arr_length) : t(t), is_arr(is_arr), arr_length(arr_length) {}
	Type(Types t, u8 pattern, std::vector<Type> types) : t(t), pattern(pattern), patterns(patterns) {}
	Type(Types t, FnSignature fn_sig) : t(t), fn_signature(fn_sig) {}

	std::string to_json();

	// matches basic determines whether a type's type (e.g. u8, interface, etc) is the same
	u8 matches_basic(Type other);

	// matches deep determines whether a type's full type signature matches (e.g. do the members match etc)
	u8 matches_deep(Type other);

	// this is for when we have different number types that the compiler tries to cast implicitly
	// an example is y : u8 = 1, by default integers are s32 and so the compiler tries to cast 1 to u8
	u8 can_niave_cast(Type other);
	void cast(Type other);
	u8 is_number_type();
	u8 is_integer_type();
	u8 is_float_type();
};

struct Value {

	std::string value;
	u8 as_u8() { return std::atoi(value.c_str()); }
	s8 as_s8() { return std::atoi(value.c_str()); }
	u16 as_u16() { return std::atoi(value.c_str()); }
	s16 as_s16() { return std::atoi(value.c_str()); }
	u32 as_u32() { return std::atoi(value.c_str()); }
	s32 as_s32() { return std::atoi(value.c_str()); }
	s64 as_s64() { return std::atoi(value.c_str()); }
	f32 as_f32() { return std::atof(value.c_str()); }
	f64 as_f64() { return std::atof(value.c_str()); }
	char as_char() { return value.at(0); }
	std::string as_string() { return value; }
};

struct SymTableEntry {
	void* optional_data;
	Type* type;
	u8 is_global = 0;
	u8 is_constant = 0;

	SymTableEntry(){}
	SymTableEntry(void* optional_data, Type* type, u8 is_global, u8 is_constant) 
		: optional_data(optional_data), type(type), is_global(is_global), is_constant(is_constant){}
};

template<typename T>
struct SymTable {
	// keep track of the latest entry
	std::pair<std::string, SymTableEntry> latest_entry;
	std::map<s32, std::map<std::string, SymTableEntry>> entries;
	s32 level = 0;

	SymTable(){}


	void dump() {
		kng_warn("dumping symtable at level {}", level);
		for (s32 i = level; i >= 0; i--) {
			kng_warn("level {}", i);
			for (auto& [key, value] : entries[i]) {
				kng_warn("level: {}, k: {}", i, key);
			}
		}
	}
	void add_symbol(std::string entry_id, SymTableEntry entry = SymTableEntry()) {
		latest_entry = std::pair<std::string, SymTableEntry>(entry_id, entry);
		entries[level][entry_id] = entry;
	}

	void set_symbol(std::string entry_id, SymTableEntry entry) {
		latest_entry = std::pair<std::string, SymTableEntry>(entry_id, entry);
		entries[level][entry_id] = entry;
	}

	SymTableEntry get_symbol(std::string entry_id) {
		for (s32 i = level; i >= 0; i--) {
			if (this->entries[i].count(entry_id) > 0) {
				return entries[i][entry_id];
			}
		}
		return SymTableEntry();
	}
	void enter_scope() { level++; };
	void pop_scope() { level--; }
};

extern Type infer_type(std::shared_ptr<AST> ast);