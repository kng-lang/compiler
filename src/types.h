/*
James Clarke - 2021
*/

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
		// the difference TYPE & interface is subtle
		// this is a type alias & only exists at compile time:
		// x : type = interface { name : u8^ }
		// this is a literal interface value and exists at runtime
		// y : interface = interface { name : u8^}
		U0,
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
		FN,
		INTERFACE,
		PATTERN,		// sequence of types
	};
	Types t = Types::UNKNOWN;
	u8 constant = 0;
	// e.g. ^u8
	u8 ref = 0;
	// e.g. u8[5]
	u8 arr = 0;
	u8 pattern = 0;
	u32 arr_length = 0;
	std::vector<Type> patterns;
	InterfaceSignature interface_signature;
	FnSignature fn_signature;

	Type(){}
	Type(Types t) : t(t){}
	Type(Types t, u8 ref) : t(t), ref(ref){}
	Type(Types t, u8 arr, u32 arr_length) : t(t), arr(arr), arr_length(arr_length) {}
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
	union v {
		u8  as_u8;
		u16 as_u16;
		u32 as_u32;
		s32 as_s32;
		s64 as_s64;
		f32 as_f32;
		f64 as_f64;
	} v;
};

template <typename T>
struct SymTableEntry {
	T value;
	u8 is_global = 0;
	u8 is_constant = 0;

	SymTableEntry(){}
	SymTableEntry(T value, u8 is_global = 0, u8 is_constant = 0) 
		: value(value), is_global(is_global), is_constant(is_constant){}
};

template<typename T>
struct SymTable {
	// keep track of the latest entry
	std::pair<std::string, SymTableEntry<T>> latest_entry;
	std::map<s32, std::map<std::string, SymTableEntry<T>>> entries;
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

	void add_symbol(std::string entry_id, SymTableEntry<T> entry) {
		latest_entry = std::pair<std::string, SymTableEntry<T>>(entry_id, entry);
		entries[level][entry_id] = entry;
	}

	SymTableEntry<T> get_symbol(std::string entry_id) {
		for (s32 i = level; i >= 0; i--) {
			if (this->entries[i].count(entry_id) > 0) {
				return entries[i][entry_id];
			}
		}
		return NULL;
	}
	void enter_scope() { level++; };
	void pop_scope() { level--; }
};

extern Type infer_type(std::shared_ptr<AST> ast);