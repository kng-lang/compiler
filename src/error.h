/*
James Clarke - 2021
*/

#pragma once

// http://what-when-how.com/compiler-writing/compile-time-error-handling-part-2/

// when an error occurs that we can't recover from, we just don't go to the next pass (e.g. code gen)

#include <stack>
#include <sstream>
#include "common.h"
#include "token.h"


struct Compiler;
struct CompilationUnit;

struct Error{

	// these are the possible levels
	enum class Level{
		RECOVERABLE, // errors that the compiler can fix
		WARNING,     // errors that the compiler cannot fix but can still complete compilation 
		CRITICAL,	 // errors that the compiler cannot recover from
	};

	enum class Type{
		MISSING_DELIMITER,	  // e.g. missing newline or ;
		TYPE_MISMATCH,		  // e.g. x : s32 = "hello world!"
		UNEXPECTED_CHARACTER, // e.g. x : s32 u32
		UNEXPECTED_EOF,       // e.g. if x { EOF
		CYCLIC_DEP,			  // when files include each other
		SYMBOL_ALREADY_DEFINED,
		UNKNOWN_TYPE
	};

	Level m_level;
	Type m_type;
	u8 m_has_solution = 0;
	Token::Position m_problem_position;
	Token::Position m_solution_position;
	//!@TODO these can be put in a message table, however this wouldn't support dynamic error reporting?
	std::string m_problem_msg;
	std::string m_solution_msg;


	Error(){}
	Error(Level level, Type type) : m_level(level), m_type(type){}
};

struct ErrorTable {
	
};

struct ErrorHandler {


	// the level of messages to be reported
	u8 report_level = 0;
	CompilationUnit* unit = NULL;

	std::stack<Error> m_error_stack;
	u32 m_how_many = 0;

	ErrorHandler(){}
	ErrorHandler(CompilationUnit* unit) : unit(unit) {}


	// @TODO report how to fix the error with colour coding e.g. the original in white and the fix in red
	virtual void error(
		const std::string problem, 
		u32 p_start_index, 
		u32 p_start_line,
		u32 p_end_index, 
		u32 p_end_line
	);

	// Used for errors that the compiler cannot determine the solution to
	virtual void error(
		Error::Level level,
		Error::Type type,
		const std::string problem,
		Token::Position problem_position
		);

	// used for errors that the compiler can determine the solutionto
	virtual void error(
		Error::Level level,
		Error::Type type,
		const std::string problem,
		const std::string solution,
		Token::Position problem_position,
		Token::Position solution_position
	);

	virtual void print_error(Error& error);
	virtual std::string pretty_format_str(Token::Position& pos, std::string& colour);
};

extern std::string red;
extern std::string green;
extern std::string reset;
extern std::string get_src_at_line(const std::string& src, u32 line);
extern std::vector<std::string> split_string_by_newline(const std::string& str);
extern std::string select_problem_area(std::string& original, u32 p_start_index, u32 p_start_line, u32 p_end_index, u32 p_end_line);
extern std::string build_pointer(u32 start, u32 end);
extern std::string remove_leading_chars(const std::string& s);