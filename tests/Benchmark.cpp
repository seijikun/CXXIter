#include <optional>

#include <CXXIter/CXXIter.h>

#include <benchmark/benchmark.h>

static std::vector<std::string> makeInput() {
	std::vector<std::string> input;
	for(size_t i = 0; i < 1024 * 1024 * 100; ++i) {
		input.push_back(std::to_string(i));
	}

	return input;
}

static std::function<std::optional<std::string>(std::string&&)> mapper = [](std::string&& item) -> std::optional<std::string> {
	int itemValue = std::stoi(item);
	if(itemValue % 2 == 0) {
		return std::to_string(itemValue * 2 + 1);
	}
	return {};
};

static void BM_PureNative(benchmark::State& state) {
	auto input = makeInput();

	for(auto _ : state) {
		std::vector<std::string> output;

		for(size_t i = 0; i < input.size(); ++i) {
			std::string& item = input[i];
			int itemValue = std::stoi(item);
			if(itemValue % 2 == 0) {
				output.push_back(std::to_string(itemValue * 2 + 1));
			}
		}
	}
}
BENCHMARK(BM_PureNative);

static void BM_NativeUsingMapperFunctor(benchmark::State& state) {
	auto input = makeInput();

	for(auto _ : state) {
		std::vector<std::string> output;

		for(size_t i = 0; i < input.size(); ++i) {
			auto res = mapper(std::forward<std::string>(input[i]));
			if(res) {
				output.push_back(res.value());
			}
		}
	}
}
BENCHMARK(BM_NativeUsingMapperFunctor);

// Define another benchmark
static void BM_CXXIter(benchmark::State& state) {
	auto input = makeInput();

	for(auto _ : state) {
		std::vector<std::string> output = CXXIter::from(input)
			.filterMap(mapper)
			.collect<std::vector>();
	}
}
BENCHMARK(BM_CXXIter);

BENCHMARK_MAIN();
