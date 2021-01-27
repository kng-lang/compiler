/*
James Clarke - 2021
*/

#include "error.h"
#include "compiler.h"
#include <iostream>


#ifndef ANSI_SUPPORT
	#define ANSI_SUPPORT
#endif // ! ANSI_SUPPORT

std::string red = "\u001b[31m";
std::string green = "\u001b[32m";
std::string reset = "\u001b[0m";

std::string get_src_at_line(const std::string& src, u32 line) {
	auto src_lines = split_string_by_newline(src);
	return src_lines.at(line);
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

std::string build_pointer(u32 start, u32 end, std::string& colour){
	std::stringstream ss;
	for (u32 i = 1; i < start; i++)
		ss << " ";
#ifdef ANSI_SUPPORT
	ss << colour;
#endif
	for (u32 i = start-1; i < end-1; i++)
		ss << "^";
#ifdef ANSI_SUPPORT
	ss << reset;
#endif
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
	kng_log("~~~ {}", build_pointer(p_start_index, p_end_index, red));
	kng_log("~~~ ");
	kng_log("~~~ {}", problem);

	m_error_stack.push(Error(Error::Level::CRITICAL, Error::Type::CYCLIC_DEP));
}




// Used for errors that the compiler cannot determine the solution to
void ErrorHandler::error(
	Error::Level level,
	Error::Type type,
	const std::string problem,
	Token::Position problem_position
){
	Error error = Error(level, type);
	error.m_problem_msg = problem;
	error.m_problem_position = problem_position;
	m_how_many++;
	m_error_stack.push(error);

	print_error(error);
}

// used for errors that the compiler can determine the solution to
// !@TODO make tokens have a Position member struct so we can directly pass that
void ErrorHandler::error(
	Error::Level level,
	Error::Type type,
	const std::string problem,
	const std::string solution,
	Token::Position problem_position,
	Token::Position solution_position
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

	// this is what we wan't
	// https://doc.rust-lang.org/edition-guide/rust-2018/the-compiler/improved-error-messages.html


	std::cout << std::endl;
	// first print what wen't wrong
	std::cout << "error ("<< (u32)error.m_type <<") : " << error.m_problem_msg << std::endl;

	// then print why it wen't wrong
	std::stringstream ss;
	ss << unit->m_compile_file.m_file_path << ":"
		<< error.m_problem_position.m_line_start << ":"
		<< error.m_problem_position.m_index_start;


	std::cout << ss.str() << std::endl;
	std::cout << pretty_format_str(error.m_problem_position, red) << std::endl;
	std::cout << build_pointer(error.m_problem_position.m_index_start, error.m_problem_position.m_index_end, red) << std::endl;

	if (error.m_has_solution) {
		std::cout << error.m_solution_msg << std::endl;
		std::cout << pretty_format_str(error.m_solution_position, green) << std::endl;
		std::cout << build_pointer(error.m_solution_position.m_index_start, error.m_solution_position.m_index_end, green) << std::endl;
	}

	std::cout << std::endl;
	//std::cout << "error (error code here) " <<
	//	unit->m_compile_file.m_file_path << ":"
	//	<< error.m_problem_position.m_line_start << ":"
	//	<< error.m_problem_position.m_index_start << std::endl;
	//
	//// print the problem along with the problem line
	//std::cout << error.m_problem_msg << std::endl;
	//std::cout << pretty_format_str(error.m_problem_position, red) << std::endl;
	//std::cout << build_pointer(error.m_problem_position.m_index_start, error.m_problem_position.m_index_end, red) << std::endl;
	//// print the solution, along with the solution line
	//if (error.m_has_solution) {
	//	std::cout << error.m_solution_msg << std::endl;
	//	std::cout << pretty_format_str(error.m_solution_position, green) << std::endl;
	//	std::cout << build_pointer(error.m_solution_position.m_index_start, error.m_solution_position.m_index_end, green) << std::endl;
	//}
}


std::string ErrorHandler::pretty_format_str(Token::Position& pos, std::string& colour) {
	std::string reset = "\u001b[0m";
	//!@TODO for now only support single line
	//! 
	
	const std::string& problem_string = get_src_at_line(unit->m_compile_file.m_file_contents, pos.m_line_start);
	//problem_string.erase(std::remove(problem_string.begin(), problem_string.end(), '\t'), problem_string.end());
	// if we don't support ANSI terminals then return the standard string
#ifndef ANSI_SUPPORT
	return problem_string;
#endif // !ANSI_SUPPORT
	const std::string& before = problem_string.substr(0, pos.m_index_start);
	
	// we substract -1 because technically 1,1, is the first char of the first line
	const std::string& during = problem_string.substr(pos.m_index_start, pos.m_index_end - pos.m_index_start);
	const std::string& after = problem_string.substr(pos.m_index_end, problem_string.length() - pos.m_index_end);
	
	std::stringstream ss;
	ss << before << colour << during << reset << after;


	return remove_leading_chars(ss.str());
}



std::string remove_leading_chars(const std::string& s) {
	const char* whitespace = " \t\v\r\n";
	std::size_t start = s.find_first_not_of(whitespace);
	std::size_t end = s.find_last_not_of(whitespace);
	return start == end ? std::string() : s.substr(start, end - start + 1);
}