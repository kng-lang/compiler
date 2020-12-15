#pragma once

// http://what-when-how.com/compiler-writing/compile-time-error-handling-part-2/

// when an error occurs that we can't recover from, we just don't go to the next pass (e.g. code gen)

#include <stack>
#include <sstream>

#include "common.h"


struct Compiler;

struct Error{
	// these are the possible levels
	enum class Levels{
		RECOVERABLE, // errors that the compiler can fix
		WARNING,     // errors that the compiler cannot fix but can still complete compilation 
		CRITICAL,	 // errors that the compiler cannot recover from
	};

	enum class Types{
		MISSING_DELIMITER,	// e.g. missing newline or ;
	};
};

struct ErrorTable {
	
};

struct ErrorHandler {


	// the level of messages to be reported
	u8 report_level = 0;
	Compiler* compiler = NULL;

	std::stack<Error> errors_occured;

	ErrorHandler(){}
	ErrorHandler(Compiler* compiler) : compiler(compiler) {}

	virtual void error(
		const std::string problem, u32 p_start_index, u32 p_start_line, u32 p_end_index, u32 p_end_line
	);
};

extern std::string get_src_at_line(u32 line);
extern std::vector<std::string> split_string_by_newline(const std::string& str);
extern std::string select_problem_area(std::string& original, u32 p_start_index, u32 p_start_line, u32 p_end_index, u32 p_end_line);
extern std::string select_area(std::string& original, u32 to, u32 from);
extern std::string build_pointer(u32 index);