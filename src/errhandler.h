#pragma once

struct ErrorHandler {

	// the original source
	std::string src;

	ErrorHandler(const std::string src) : src(src) {}

	virtual void error(const std::string msg, u32 index, u32 line) {
		log("{}:{} error: {}", index, line, msg);
		log("{}", "[src here]");
	}


};