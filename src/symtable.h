#pragma once


#include "types.h"
#include "common.h"

struct SymTableEntry {
	void* optional_data;
	Type* type;
	u8 is_global = 0;
	u8 is_constant = 0;

	SymTableEntry() {}
	SymTableEntry(void* optional_data, Type* type, u8 is_global, u8 is_constant)
		: optional_data(optional_data), type(type), is_global(is_global), is_constant(is_constant) {}
};

template<typename T>
struct SymTable {
	// keep track of the latest entry
	std::pair<Token, SymTableEntry> latest_entry;
	std::map<s32, std::map<Token, SymTableEntry>> entries;
	s32 level = 0;

	SymTable() {}


	void dump() {
		kng_warn("dumping symtable at level {}", level);
		for (s32 i = level; i >= 0; i--) {
			kng_warn("level {}", i);
			for (auto& [key, value] : entries[i]) {
				kng_warn("level: {}, k: {}", i, key);
			}
		}
	}
	void add_symbol(Token entry_id, SymTableEntry entry = SymTableEntry()) {
		latest_entry = std::pair<Token, SymTableEntry>(entry_id, entry);
		entries[level][entry_id] = entry;
	}

	void set_symbol(Token entry_id, SymTableEntry entry) {
		latest_entry = std::pair<Token, SymTableEntry>(entry_id, entry);
		entries[level][entry_id] = entry;
	}

	u8 contains_symbol(Token entry_id) {
		for (s32 i = level; i >= 0; i--) {
			if (this->entries[i].count(entry_id) > 0) {
				return 1;
			}
		}
		return 0;
	}

	SymTableEntry get_symbol(Token entry_id) {
		for (s32 i = level; i >= 0; i--) {
			if (this->entries[i].count(entry_id) > 0) {
				return entries[i][entry_id];
			}
		}
		return SymTableEntry();
	}
	void enter_scope() { level++; };
	void pop_scope() {
		// @TODO this doesn't actually clear the symbols at the current scope and causes
		// a "sym already defined" error, to fix clear the scope
		entries[level].clear();
		level--;
	}
};