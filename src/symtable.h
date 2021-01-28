#pragma once

#include <optional>
#include "types.h"
#include "common.h"


struct SymTable;
struct SymTableScope;

struct SymEntry {
	void* m_optional_data;
	Type m_type;
	// used if this entry is a symbol table itself
	std::shared_ptr<SymTableScope> m_table;

	SymEntry(){}
	SymEntry(std::shared_ptr<SymTableScope> table) : m_table(table) {}
	SymEntry(void* optional_data, Type type)
		: m_optional_data(optional_data), m_type(type) {}
	SymEntry(void* optional_data, Type type, std::shared_ptr<SymTableScope> table)
		: m_optional_data(optional_data), m_type(type), m_table(table) {}
};

struct SymTableScope {
	// keep track of the latest entry
	std::pair<Token, SymEntry> latest_entry;
	std::map<Token, SymEntry> m_entries;
	s32 level = 0;


	std::shared_ptr<SymTableScope> m_above;
	std::shared_ptr<SymTableScope> m_current;

	SymTableScope() {}
};



struct SymTable {
	u32 m_level = 0;
	std::map<u32, SymTableScope> m_scopes;

	SymTable() {
		m_scopes[0] = SymTableScope();
	}

	void add(Token id, SymEntry entry = SymEntry()) {
		m_scopes[m_level].m_entries[id] = entry;
		m_scopes[m_level].latest_entry = std::pair<Token, SymEntry>(id, entry);
	}

	SymEntry* get(Token id) {
		for(int i = m_level;i>=0;i--){
			for (const auto& [key, value] : m_scopes[i].m_entries)
				if (key.m_value.compare(id.m_value) == 0) {
					return &m_scopes[i].m_entries[key];
			}
		}
		return {};
	}

	u8 global() {
		return m_level == 0;
	}

	u8 empty() {
		return m_scopes[m_level].m_entries.size() == 0;
	}

	void do_stuff_upwards(std::function<void(Token, SymEntry)> callback){
		for (int i = m_level; i >= 0; i--) {
			for (const auto& [key, value] : m_scopes[i].m_entries)
				callback(key, value);
		}
	}



	std::pair<Token, SymEntry> latest() {
		return m_scopes[m_level].latest_entry;
	}


	u8 exists_currently(Token id) {
		for (const auto& [key, value] : m_scopes[m_level].m_entries)
			if (key.m_value.compare(id.m_value) == 0)
				return 1;
		return 0;
	}

	u8 exists(Token id) {
		for (int i = m_level; i >= 0; i--) {
			for (const auto& [key, value] : m_scopes[i].m_entries)
				if (key.m_value.compare(id.m_value) == 0)
					return 1;
		}
		return 0;
	}

	void add_enter(Token id = Token("anon"), SymEntry entry = SymEntry()) {
		m_scopes[m_level].latest_entry = std::pair<Token, SymEntry>(id, entry);
		m_scopes[m_level].m_entries[id] = entry;
		m_level++;
		m_scopes[m_level] = SymTableScope();
	}

	void enter_anon() {
		add_enter();
	}

	void exit() {
		m_level--;
	}


	void set_symbol(Token entry_id, SymEntry entry) {
		m_scopes[m_level].m_entries[entry_id] = entry;
	}
};


//struct SymTable {
//	std::shared_ptr<SymTableScope> m_global;
//	std::shared_ptr<SymTableScope> m_current;
//
//	SymTable() {
//		m_global = std::make_shared<SymTableScope>();
//		m_current = m_global;
//	}
//
//	void add(Token id, SymEntry entry = SymEntry()) {
//		m_current->m_entries[id] = entry;
//		m_current->latest_entry = std::pair<Token, SymEntry>(id, entry);
//	}
//
//
//	SymEntry get(Token id) {
//		std::shared_ptr<SymTableScope> table = m_current;
//		while (table) {
//			for (const auto& [key, value] : table->m_entries)
//				if (key.m_value.compare(id.m_value) == 0)
//					return value;
//			table = table->m_above;
//		}
//		return {};
//	}
//
//	u8 global() {
//		return m_current->m_above==NULL;
//	}
//
//
//	void do_stuff_upwards(std::function<void(Token, SymEntry)> callback){
//		std::shared_ptr<SymTableScope> table = m_current;
//		while (table) {
//			for (const auto& [key, value] : table->m_entries)
//				callback(key, value);
//			table = table->m_above;
//		}
//	}
//
//
//
//	std::pair<Token, SymEntry> latest() {
//		return m_current->latest_entry;
//	}
//
//
//	u8 exists_currently(Token id) {
//		for (const auto& [key, value] : m_current->m_entries)
//			if (key.m_value.compare(id.m_value) == 0)
//				return 1;
//		return 0;
//	}
//
//	u8 exists(Token id) {
//		std::shared_ptr<SymTableScope> table = m_current;
//		while (table) {
//			for (const auto& [key, value] : table->m_entries)
//				if (key.m_value.compare(id.m_value) == 0)
//					return 1;
//			table = table->m_above;
//		}
//		return 0;
//	}
//
//	void add_enter(Token id = Token("anon"), SymEntry entry = SymEntry()) {
//
//		entry.m_table = std::make_shared<SymTableScope>();
//		m_current->latest_entry = std::pair<Token, SymEntry>(id, entry);
//		m_current->m_entries[id] = entry;
//		entry.m_table->m_above = m_current;
//		m_current = entry.m_table;
//	}
//
//	void enter_anon() {
//		add_enter();
//	}
//
//	void exit() {
//		m_current = m_current->m_above;
//	}
//
//
//	void set_symbol(Token entry_id, SymEntry entry) {
//		m_current->latest_entry = std::pair<Token, SymEntry>(entry_id, entry);
//		m_current->m_entries[entry_id] = entry;
//	}
//};