#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <optional>
#include <unordered_set>
#include <unordered_map>

#include "CIter.h"


struct DebugLogger {
	std::string heapTest;

	DebugLogger(std::string heapTest) : heapTest(heapTest) {
		std::cout << "ctor(" << this << ")" << std::endl;
	}
	~DebugLogger() {
		std::cout << "~dtor(" << this << ")" << std::endl;
	}
	DebugLogger(const DebugLogger& o) : heapTest(o.heapTest) {
		std::cout << "cpyctor(" << this << ") from " << &o << std::endl;
	}
	DebugLogger& operator=(const DebugLogger& o) {
		heapTest = o.heapTest;
		std::cout << "copy= (" << this << ") from " << &o << std::endl;
		return *this;
	}
	DebugLogger(DebugLogger&& o) : heapTest(std::move(o.heapTest)) {
		std::cout << "movctor(" << this << ") from " << &o << std::endl;
	}
	DebugLogger& operator=(DebugLogger&& o) {
		heapTest = std::move(o.heapTest);
		std::cout << "move= (" << this << ") from " << &o << std::endl;
		return *this;
	}
};
std::ostream& operator<<(std::ostream& o, const DebugLogger& l) {
	o << "DebugLogger" << std::endl;
	return o;
}

using namespace CIter;

int main() {
	{
		std::vector<float> input = {1.35, 56.123};
		std::vector<double> output = CIter::from(input)
				.cast<double>()
				.collect<std::vector>();
		std::cout << output[0] << std::endl;
	}
	{
		std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
		std::vector<std::string> output = CIter::from(input)
				.map([](const auto& pair) { return pair.second; })
				.collect<std::vector>();
		std::cout << output[0] << std::endl;
	}
	{
		std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
		std::unordered_set<int> output = CIter::from(input)
				.map([](const auto& pair) { return pair.first; })
				.collect<std::unordered_set>();
		std::cout << output.size() << std::endl;
	}
	{
		std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
		std::unordered_map<int, std::string> output = CIter::from(input)
				.modify([](auto& keyValue) { keyValue.second = "-" + keyValue.second; })
				.collect<std::unordered_map>();
		std::cout << output.size() << std::endl;
	}
	{
		std::vector<int> input = {1337, 42};
		std::unordered_map<int, std::string> output = CIter::from(input)
				.map([](int i) { return std::make_pair(i, std::to_string(i)); })
				.collect<std::unordered_map>();
		std::cout << "1337: " << output[1337] << std::endl;
		std::cout << "42: " << output[42] << std::endl;
	}
}

//int main() {
//	std::vector<DebugLogger> inVec;
//	inVec.emplace_back("Hallo, ich bin der HeapTest");

//	{ std::cout << "########## " << "Const references" << std::endl;
//		std::vector<std::string> outVec = CIter::from(inVec)
//				.filter([](const DebugLogger& o){ return true; })
//				.map([](const DebugLogger& o) { return o.heapTest; })
//				.collect<std::vector>();
//		std::cout << outVec.size() << std::endl;
//	}
//	{ std::cout << "########## " << "Mutable References" << std::endl;
//		std::vector<std::string> outVec = CIter::from(inVec)
//				.filter([](const DebugLogger& o){ return true; })
//				.map([](DebugLogger& o) { return o.heapTest; })
//				.collect<std::vector>();
//		std::cout << outVec.size() << std::endl;
//	}
//	{ std::cout << "########## " << "Moved Objects" << std::endl;
//		auto outVec = CIter::from(std::move(inVec))
//			.filterMap([](DebugLogger&& o) -> std::optional<std::string> {
//				return std::move(o.heapTest);
//			})
//			.collect<std::vector>();
//		std::cout << outVec.size() << std::endl;
//	}

//	return 0;
//}
