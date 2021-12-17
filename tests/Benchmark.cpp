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

static std::vector<std::string> INPUT1 = makeInput1();
static std::vector<double> INPUT2 = makeInput2();

// ################################################################################################
// BENCHMARKS
// ################################################################################################
static void BM_NativeUsingLambdas(benchmark::State& state) {
	for (auto _ : state) { //TODO: output sizeHint
		{ // filterMap
			std::vector<std::string> output;
			for(size_t i = 0; i < INPUT1.size(); ++i) {
				auto res = FILTERMAP_FN(std::forward<std::string>(INPUT1[i]));
				if(res) {
					output.push_back(res.value());
				}
			}
		}
		{ // filter
			std::vector<double> output;
			auto outputInserter = std::back_inserter(output);
			std::copy_if(INPUT2.begin(), INPUT2.end(), outputInserter, FILTER_FN);
		}
		{ // map
			std::vector<double> output;
			for(size_t i = 0; i < INPUT2.size(); ++i) {
				output.push_back(MAP_FN(INPUT2[i]));
			}
		}
		{ // cast
			std::vector<float> output;
			for(size_t i = 0; i < INPUT2.size(); ++i) {
				output.push_back(static_cast<float>(INPUT2[i]));
			}
		}
		{ // groupBy
			std::unordered_map<size_t, std::vector<std::string>> output;
			for(size_t i = 0; i < INPUT1.size(); ++i) {
				size_t groupIdent = INPUT1[i].size();
				if(output.find(groupIdent) == output.end()) {
					output[groupIdent] = { INPUT1[i] };
				} else {
					output[groupIdent].push_back(INPUT1[i]);
				}
			}
		}
	}
}
BENCHMARK(BM_NativeUsingLambdas)->MinTime(60);




// Define another benchmark
static void BM_CXXIter(benchmark::State& state) {
	for (auto _ : state) {
		{ // filterMap
			std::vector<std::string> output = CXXIter::from(INPUT1)
				.filterMap(FILTERMAP_FN)
				.collect<std::vector>();
		}
		{ // filter
			std::vector<double> output = CXXIter::from(INPUT2)
					.filter(FILTER_FN)
					.collect<std::vector>();
		}
		{ // map
			std::vector<double> output = CXXIter::from(INPUT2)
					.map([](double val) { return std::sqrt(val); })
					.collect<std::vector>();
		}
		{ // cast
			std::vector<float> output = CXXIter::from(INPUT2).cast<float>().collect<std::vector>();
		}
		{ // groupBy
			std::unordered_map<size_t, std::vector<std::string>> output = CXXIter::from(INPUT1)
					.groupBy([](const std::string& item) { return item.size(); })
					.collect<std::unordered_map>();
		}
	}
}
BENCHMARK(BM_CXXIter)->MinTime(60);



BENCHMARK_MAIN();
