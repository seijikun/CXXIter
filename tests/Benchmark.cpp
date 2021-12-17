#include <iostream>
#include <optional>
#include <cmath>

#include <CXXIter/CXXIter.h>

#include <benchmark/benchmark.h>

static std::vector<std::string> makeInput1() {
	std::vector<std::string> input;
	for(size_t i = 0; i < 1024 * 1024 * 100; ++i) {
		input.push_back(std::to_string(i));
	}
	return input;
}

static std::vector<double> makeInput2() {
	std::vector<double> input;
	for(size_t i = 0; i < 1024 * 1024 * 200; ++i) {
		input.push_back(std::pow(std::sqrt((double)i), M_PI));
	}
	return input;
}


#define FILTERMAP_FN [](std::string&& item) -> std::optional<std::string> { \
	int itemValue = std::stoi(item); \
	if(itemValue % 2 == 0) { \
		return std::to_string(itemValue * 2 + 1); \
	} \
	return {}; \
}

#define FILTER_FN [](double val) { \
	int64_t iVal = static_cast<int64_t>(std::floor(val)); \
	return (iVal % 2 == 0); \
}

#define MAP_FN std::sqrt


// ################################################################################################
// BENCHMARKS
// ################################################################################################
static void BM_NativeUsingLambdas(benchmark::State& state) {
	auto input1 = makeInput1();
	auto input2 = makeInput2();

	for(auto _ : state) { //TODO: output sizeHint
		{ // filterMap
			std::vector<std::string> output;
			for(size_t i = 0; i < input1.size(); ++i) {
				auto res = FILTERMAP_FN(std::forward<std::string>(input1[i]));
				if(res) {
					output.push_back(res.value());
				}
			}
		}
		{ // filter
			std::vector<double> output;
			auto outputInserter = std::back_inserter(output);
			std::copy_if(input2.begin(), input2.end(), outputInserter, FILTER_FN);
		}
		{ // map
			std::vector<double> output;
			for(size_t i = 0; i < input2.size(); ++i) {
				output.push_back(MAP_FN(input2[i]));
			}
		}
		{ // cast
			std::vector<float> output;
			for(size_t i = 0; i < input2.size(); ++i) {
				output.push_back(static_cast<float>(input2[i]));
			}
		}
		{ // groupBy
			std::unordered_map<size_t, std::vector<std::string>> output;
			for(size_t i = 0; i < input1.size(); ++i) {
				size_t groupIdent = input1[i].size();
				if(output.find(groupIdent) == output.end()) {
					output[groupIdent] = { input1[i] };
				} else {
					output[groupIdent].push_back(input1[i]);
				}
			}
		}
	}
}
BENCHMARK(BM_NativeUsingLambdas);




// Define another benchmark
static void BM_CXXIter(benchmark::State& state) {
	auto input1 = makeInput1();
	auto input2 = makeInput2();

	for(auto _ : state) {
		{ // filterMap
			std::vector<std::string> output = CXXIter::from(input1)
				.filterMap(FILTERMAP_FN)
				.collect<std::vector>();
		}
		{ // filter
			std::vector<double> output = CXXIter::from(input2)
					.filter(FILTER_FN)
					.collect<std::vector>();
		}
		{ // map
			std::vector<double> output = CXXIter::from(input2)
					.map([](double val) { return std::sqrt(val); })
					.collect<std::vector>();
		}
		{ // cast
			std::vector<float> output = CXXIter::from(input2).cast<float>().collect<std::vector>();
		}
		{ // groupBy
			std::unordered_map<size_t, std::vector<std::string>> output = CXXIter::from(input1)
					.groupBy([](const std::string& item) { return item.size(); })
					.collect<std::unordered_map>();
		}
	}
}
BENCHMARK(BM_CXXIter);



BENCHMARK_MAIN();
