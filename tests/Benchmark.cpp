#include <iostream>
#include <optional>
#include <version>
#include <vector>
#include <cmath>

#ifdef CXXITER_HAS_CXX20RANGES
#include <ranges>
#endif

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


#define FILTERMAP_FN [](std::string item) -> std::optional<std::string> { \
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

static const std::vector<std::string> INPUT1 = makeInput1();
static const std::vector<double> INPUT2 = makeInput2();

// ################################################################################################
// BENCHMARKS
// ################################################################################################

// FILTER MAP
// ==========
static void BM_FilterMap_Native(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<std::string> output;
		for(size_t i = 0; i < INPUT1.size(); ++i) {
			auto res = FILTERMAP_FN( INPUT1[i] );
			if(res) {
				output.push_back(res.value());
			}
		}
	}
} BENCHMARK(BM_FilterMap_Native)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void BM_FilterMap_CXX20Ranges(benchmark::State& state) {
	for (auto _ : state) {
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
} BENCHMARK(BM_FilterMap_CXX20Ranges)->MinTime(10);
#endif

static void BM_FilterMap_CXXIter(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<std::string> output = CXXIter::from(INPUT1)
			.filterMap(FILTERMAP_FN)
			.collect<std::vector>();
	}
} BENCHMARK(BM_FilterMap_CXXIter)->MinTime(10);



// FILTER
// ==========
static void BM_Filter_Native(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<double> output;
		auto outputInserter = std::back_inserter(output);
		std::copy_if(INPUT2.begin(), INPUT2.end(), outputInserter, FILTER_FN);
	}
} BENCHMARK(BM_Filter_Native)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void BM_Filter_CXX20Ranges(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<double> output;
		auto range = INPUT2 | std::views::filter(FILTER_FN);
		for(const auto& item : range) { output.push_back(item); }
	}
} BENCHMARK(BM_Filter_CXX20Ranges)->MinTime(10);
#endif

static void BM_Filter_CXXIter(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<double> output = CXXIter::from(INPUT2)
				.filter(FILTER_FN)
				.collect<std::vector>();
	}
} BENCHMARK(BM_Filter_CXXIter)->MinTime(10);



// MAP
// ==========
static void BM_Map_Native(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<double> output;
		for(size_t i = 0; i < INPUT2.size(); ++i) {
			output.push_back(MAP_FN(INPUT2[i]));
		}
	}
} BENCHMARK(BM_Map_Native)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void BM_Map_CXX20Ranges(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<double> output;
		auto range = INPUT2 | std::views::transform([](double item) { return MAP_FN(item); });
		for(const auto& item : range) { output.push_back(item); }
	}
} BENCHMARK(BM_Map_CXX20Ranges)->MinTime(10);
#endif

static void BM_Map_CXXIter(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<double> output = CXXIter::from(INPUT2)
				.map([](double val) { return std::sqrt(val); })
				.collect<std::vector>();
	}
} BENCHMARK(BM_Map_CXXIter)->MinTime(10);



// CAST
// ==========
static void BM_Cast_Native(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<float> output;
		for(size_t i = 0; i < INPUT2.size(); ++i) {
			output.push_back(static_cast<float>(INPUT2[i]));
		}
	}
} BENCHMARK(BM_Cast_Native)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void BM_Cast_CXX20Ranges(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<float> output;
		auto range = INPUT2 | std::views::transform([](double item) { return static_cast<float>(item); });
		for(const auto& item : range) { output.push_back(item); }
	}
} BENCHMARK(BM_Cast_CXX20Ranges)->MinTime(10);
#endif

static void BM_Cast_CXXIter(benchmark::State& state) {
	for (auto _ : state) {
		std::vector<float> output = CXXIter::from(INPUT2).cast<float>().collect<std::vector>();
	}
} BENCHMARK(BM_Cast_CXXIter)->MinTime(10);



// GROUP BY
// ==========
static void BM_GroupBy_Native(benchmark::State& state) {
	for (auto _ : state) {
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
} BENCHMARK(BM_GroupBy_Native)->MinTime(10);

static void BM_GroupBy_CXXIter(benchmark::State& state) {
	for (auto _ : state) {
		// quite a lot slower than native of course, because groupBy() internally creates
		// a unordered_map, that is then pushed into the iterator, just to put the elements
		// in a new unordered_map in collect().
		std::unordered_map<size_t, std::vector<std::string>> output = CXXIter::from(INPUT1)
				.groupBy([](const std::string& item) { return item.size(); })
				.collect<std::unordered_map>();
	}
} BENCHMARK(BM_GroupBy_CXXIter)->MinTime(10);


BENCHMARK_MAIN();
