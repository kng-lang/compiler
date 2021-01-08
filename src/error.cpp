/*
James Clarke - 2021
*/

#include "error.h"
#include "compiler.h"

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

// select an area of the original src 
std::string select_area(std::string& original, u32 to, u32 from){
	return original.substr(to, from);
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
	how_many++;

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

	errors_occured.push(Error(Error::Level::CRITICAL, Error::Type::CYCLIC_DEP));
}