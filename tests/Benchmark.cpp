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

static std::vector<std::string> makeInput1(size_t cnt) {
	return CXXIter::range<size_t>(0, cnt)
			.map([](size_t item) { return std::to_string(item); })
			.collect<std::vector>();
}

static std::vector<double> makeInput2(size_t cnt) {
	return CXXIter::range<double>(0, static_cast<double>(cnt))
			.map([](double item) { return std::pow(std::sqrt((double)item), M_PI); })
			.collect<std::vector>();
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

static const std::vector<std::string> INPUT1 = makeInput1(1024 * 1024 * 100);
static const std::vector<std::string> INPUT1_BURST = makeInput1(10);
static const std::vector<double> INPUT2 = makeInput2(1024 * 1024 * 200);
static const std::vector<double> INPUT2_BURST = makeInput2(10);

// ################################################################################################
// BENCHMARKS
// ################################################################################################

// FILTER MAP
// ==========
static void FilterMap_Native(benchmark::State& state, const std::vector<std::string>& input) {
	for (auto _ : state) {
		std::vector<std::string> output;
		for(size_t i = 0; i < input.size(); ++i) {
			auto res = FILTERMAP_FN( input[i] );
			if(res) {
				output.push_back(res.value());
			}
		}
	}
}
BENCHMARK_CAPTURE(FilterMap_Native, Large, INPUT1)->MinTime(10);
BENCHMARK_CAPTURE(FilterMap_Native, Small, INPUT1_BURST)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void FilterMap_CXX20Ranges(benchmark::State& state, const std::vector<std::string>& input) {
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
		auto range = input | std::views::filter(filterFn) | std::views::transform(mapFn);
		for(const auto& item : range) { output.push_back(item); }
	}
}
BENCHMARK_CAPTURE(FilterMap_CXX20Ranges, Large, INPUT1)->MinTime(10);
BENCHMARK_CAPTURE(FilterMap_CXX20Ranges, Small, INPUT1_BURST)->MinTime(10);
#endif

static void FilterMap_CXXIter(benchmark::State& state, const std::vector<std::string>& input) {
	for (auto _ : state) {
		std::vector<std::string> output = CXXIter::from(input)
			.filterMap(FILTERMAP_FN)
			.collect<std::vector>();
	}
}
BENCHMARK_CAPTURE(FilterMap_CXXIter, Large, INPUT1)->MinTime(10);
BENCHMARK_CAPTURE(FilterMap_CXXIter, Small, INPUT1_BURST)->MinTime(10);



// FILTER
// ==========
static void Filter_Native(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<double> output;
		auto outputInserter = std::back_inserter(output);
		std::copy_if(input.begin(), input.end(), outputInserter, FILTER_FN);
	}
}
BENCHMARK_CAPTURE(Filter_Native, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Filter_Native, Small, INPUT2_BURST)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void Filter_CXX20Ranges(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<double> output;
		auto range = input | std::views::filter(FILTER_FN);
		for(const auto& item : range) { output.push_back(item); }
	}
}
BENCHMARK_CAPTURE(Filter_CXX20Ranges, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Filter_CXX20Ranges, Small, INPUT2_BURST)->MinTime(10);
#endif

static void Filter_CXXIter(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<double> output = CXXIter::from(input)
				.filter(FILTER_FN)
				.collect<std::vector>();
	}
}
BENCHMARK_CAPTURE(Filter_CXXIter, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Filter_CXXIter, Small, INPUT2_BURST)->MinTime(10);



// MAP
// ==========
static void Map_Native(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<double> output;
		for(size_t i = 0; i < input.size(); ++i) {
			output.push_back(MAP_FN(input[i]));
		}
	}
}
BENCHMARK_CAPTURE(Map_Native, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Map_Native, Small, INPUT2_BURST)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void Map_CXX20Ranges(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<double> output;
		auto range = input | std::views::transform([](double item) { return MAP_FN(item); });
		for(const auto& item : range) { output.push_back(item); }
	}
}
BENCHMARK_CAPTURE(Map_CXX20Ranges, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Map_CXX20Ranges, Small, INPUT2_BURST)->MinTime(10);
#endif

static void Map_CXXIter(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<double> output = CXXIter::from(input)
				.map([](double val) { return std::sqrt(val); })
				.collect<std::vector>();
	}
}
BENCHMARK_CAPTURE(Map_CXXIter, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Map_CXXIter, Small, INPUT2_BURST)->MinTime(10);



// CAST
// ==========
static void Cast_Native(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<float> output;
		for(size_t i = 0; i < input.size(); ++i) {
			output.push_back(static_cast<float>(input[i]));
		}
	}
}
BENCHMARK_CAPTURE(Cast_Native, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Cast_Native, Small, INPUT2_BURST)->MinTime(10);

#ifdef CXXITER_HAS_CXX20RANGES
static void Cast_CXX20Ranges(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<float> output;
		auto range = input | std::views::transform([](double item) { return static_cast<float>(item); });
		for(const auto& item : range) { output.push_back(item); }
	}
}
BENCHMARK_CAPTURE(Cast_CXX20Ranges, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Cast_CXX20Ranges, Small, INPUT2_BURST)->MinTime(10);
#endif

static void Cast_CXXIter(benchmark::State& state, const std::vector<double>& input) {
	for (auto _ : state) {
		std::vector<float> output = CXXIter::from(input).cast<float>().collect<std::vector>();
	}
}
BENCHMARK_CAPTURE(Cast_CXXIter, Large, INPUT2)->MinTime(10);
BENCHMARK_CAPTURE(Cast_CXXIter, Small, INPUT2_BURST)->MinTime(10);



// GROUP BY
// ==========
static void GroupBy_Native(benchmark::State& state, const std::vector<std::string>& input) {
	for (auto _ : state) {
		std::unordered_map<size_t, std::vector<std::string>> output;
		for(size_t i = 0; i < input.size(); ++i) {
			size_t groupIdent = input[i].size();
			if(output.find(groupIdent) == output.end()) {
				output[groupIdent] = { input[i] };
			} else {
				output[groupIdent].push_back(input[i]);
			}
		}
	}
}
BENCHMARK_CAPTURE(GroupBy_Native, Large, INPUT1)->MinTime(10);
BENCHMARK_CAPTURE(GroupBy_Native, Small, INPUT1_BURST)->MinTime(10);

static void GroupBy_CXXIter(benchmark::State& state, const std::vector<std::string>& input) {
	for (auto _ : state) {
		// quite a lot slower than native of course, because groupBy() internally creates
		// a unordered_map, that is then pushed into the iterator, just to put the elements
		// in a new unordered_map in collect().
		std::unordered_map<size_t, std::vector<std::string>> output = CXXIter::from(input)
				.groupBy([](const std::string& item) { return item.size(); })
				.collect<std::unordered_map>();
	}
}
BENCHMARK_CAPTURE(GroupBy_CXXIter, Large, INPUT1)->MinTime(10);
BENCHMARK_CAPTURE(GroupBy_CXXIter, Small, INPUT1_BURST)->MinTime(10);


BENCHMARK_MAIN();
