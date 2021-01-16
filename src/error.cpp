/*
James Clarke - 2021
*/

#include "error.h"
#include "compiler.h"
#include <iostream>


#ifndef  ANSI_SUPPORT
	#define ANSI_SUPPORT
#endif // ! ANSI_SUPPORT


std::string get_src_at_line(const std::string& src, u32 line) {
	auto src_lines = split_string_by_newline(src);
	return src_lines.at(line - 1);
}

std::vector<std::string> split_string_by_newline(const std::string& str){
	auto result = std::vector<std::string>{};
	auto ss = std::stringstream{ str };

	for (std::string line; std::getline(ss, line, '\n');)
		result.push_back(line);

	return result;
}
// used to select the area that is the problem
std::string select_problem_area(std::string& original, u32 p_start_index, u32 p_start_line, u32 p_end_index, u32 p_end_line) {
	std::stringstream ss;

	auto lines = split_string_by_newline(original);
	u32 min_line = (p_start_line - 1) >= 1 ? p_start_line - 1 : 1;
	u32 max_line = (p_start_line + 1) <= lines.size() ? p_start_line + 1 : p_start_line;

	for (int i = min_line; i <= max_line; i++)
		ss << lines.at(i-1) << "\n";
	return ss.str();
}

std::string build_pointer(u32 start, u32 end){
	std::stringstream ss;
	for (u32 i = 1; i < start; i++)
		ss << " ";
	for (u32 i = start-1; i < end-1; i++)
		ss << "^";
	return ss.str();
}

// @TODO support warnings which can be recoverable
void ErrorHandler::error(
	const std::string problem, u32 p_start_index, u32 p_start_line, u32 p_end_index, u32 p_end_line
) {
	m_how_many++;

	auto problem_string = select_problem_area(
		unit->m_compile_file.m_file_contents, p_start_index, p_start_line, p_end_index, p_end_line
		);

	problem_string = get_src_at_line(unit->m_compile_file.m_file_contents, p_start_line);

	// @TODO calculate where the start of the line is on the error line
	kng_log("~~~ error in {}:{}:{}", unit->m_compile_file.m_file_path, p_start_line, p_start_index);
	kng_log("~~~ ");
	kng_log("~~~ {}", problem_string);
	kng_log("~~~ {}", build_pointer(p_start_index, p_end_index));
	kng_log("~~~ ");
	kng_log("~~~ {}", problem);

	m_error_stack.push(Error(Error::Level::CRITICAL, Error::Type::CYCLIC_DEP));
}




// Used for errors that the compiler cannot determine the solution to
void ErrorHandler::error(
	Error::Level level,
	Error::Type type,
	const std::string problem,
	Error::ErrorPosition problem_position
){
	Error error = Error(level, type);
	error.m_problem_msg = problem;
	error.m_problem_position = problem_position;
	m_how_many++;
	m_error_stack.push(error);

	print_error(error);
}

// used for errors that the compiler can determine the solutionto
void ErrorHandler::error(
	Error::Level level,
	Error::Type type,
	const std::string problem,
	const std::string solution,
	Error::ErrorPosition problem_position,
	Error::ErrorPosition solution_position
) {
	Error error = Error(level, type);
	error.m_problem_msg = problem;
	error.m_solution_msg = solution;
	error.m_has_solution = 1;
	error.m_problem_position = problem_position;
	error.m_solution_position = solution_position;
	m_how_many++;
	m_error_stack.push(error);

	print_error(error);
}

void ErrorHandler::print_error(Error& error) {


	std::string red = "\u001b[31m";
	std::string green = "\u001b[32m";
	std::string reset = "\u001b[0m";

	std::cout << error.m_problem_msg << std::endl;
	std::cout << pretty_format_str(error.m_problem_position, red) << std::endl;
	if (error.m_has_solution) {
		std::cout << error.m_solution_msg << std::endl;
		std::cout << pretty_format_str(error.m_solution_position, green) << std::endl;
	}
}


std::string ErrorHandler::pretty_format_str(Error::ErrorPosition& pos, std::string& colour) {
	std::string reset = "\u001b[0m";
	//!@TODO for now only support single line
	//! 
	
	std::string& problem_string = get_src_at_line(unit->m_compile_file.m_file_contents, pos.m_line_start);
	// if we don't support ANSI terminals then return the standard string
#ifndef ANSI_SUPPORT
	return problem_string;
#endif // !ANSI_SUPPORT

	std::string& before = problem_string.substr(0, pos.m_index_start);
	std::string& during = problem_string.substr(pos.m_index_start, pos.m_index_end - pos.m_index_start);
	std::string& after = problem_string.substr(pos.m_index_end, problem_string.length() - pos.m_index_end);
	
	//kng_log("original: {}", problem_string);
	//kng_log("line len: {}", problem_string.length());
	//kng_log("i: {}, i+length: {}", pos.m_index_start, pos.m_index_end);
	//kng_log("before: {}:)", before);
	//kng_log("during: {}:)", during);
	//kng_log("after: {}:)", after);

	std::stringstream ss;
	ss << before << during << reset << after;

	return ss.str();
}