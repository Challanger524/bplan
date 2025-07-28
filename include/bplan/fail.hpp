#pragma once

#include <string>
#include <string_view>

namespace bplan {

struct Success {}; // same as `Fail()`

struct Fail {
	std::string message;

public:
	Fail(std::string_view what) : message(what) {}
	Fail(      Success&&)       : message(    ) {}
	Fail(const Success& ) = delete;
	operator bool() const { return !message.empty(); }
	const std::string& what() const { return message; }
};

//Fail Success() { return Fail(); }
//using Success = bplan::Fail;


}
