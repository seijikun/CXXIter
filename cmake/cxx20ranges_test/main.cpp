#include <ranges>
#include <vector>

int main() {
	static std::vector<std::string> INPUT1 = {"1", "2", "3"};

	auto filterFn = [](const std::string& item){
		int itemValue = std::stoi(item);
		return (itemValue % 2 == 0);
	};
	auto mapFn = [](const std::string& item) {
		int itemValue = std::stoi(item);
		return std::to_string(itemValue * 2 + 1);
	};


	std::vector<std::string> output;
	auto range = INPUT1 | std::views::filter(filterFn) | std::views::transform(mapFn);
	for(const auto& item : range) { output.push_back(item); }
}
