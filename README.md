# CXXIter

CXXIter is a ergonomic C++ Iterator interface for STL containers, similar to the iterators found in Rust or C#'s LINQ.
It supports passing values by (const) reference or by using move semantics, which is tricky in some places, since references can not be stored in STL containers.

## The API

A full API-Documentation can be found [here](https://seijikun.github.io/CXXIter/docs/).

### Entry
The `CXXIter` interface is entered by instantiating one of the possible source classes.
The type of the source determines how the values from the container are passed (const reference / reference / move semantics).

For that, there exist the three sources `CXXIter::SrcCRef`, `CXXIter::SrcRef`, and `CXXIter::SrcMov` respectively.

```cpp
std::vector<float> input = {1.34f, 1.37f};

CXXIter::SrcCRef constRefIter(input);
CXXIter::SrcRef  mutableRefIter(input);
CXXIter::SrcMov  moveIter(std::move(input));
```

There is also the shortcut using `CXXIter::from()`, which tries to determine the type of the source class to use, depending on the type of the given input parameter:
```cpp
const std::vector<float> constInput = {1.34f, 1.37f};
std::vector<float> input = {1.34f, 1.37f};

auto constRefIter   = CXXIter::from(constInput);
auto mutableRefIter = CXXIter::from(input);
auto moveIter       = CXXIter::from(std::move(input));
```

### Chaining
From there, everything returned by calling a member function either returns another iterator, or resolves the iterator to a final result. There are a lot of chain functions. Here **some examples**:

**cast** - Casting the elements of the iterator
```cpp
std::vector<float> input = {1.35, 56.123};
std::vector<double> output = CXXIter::from(input)
	.cast<double>()
	.collect<std::vector>();
```

**filter** - Filtering elements of the iterator
```cpp
std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
std::vector<int> output = CXXIter::from(input)
        .filter([](int item) { return (item % 2) == 0; })
        .collect<std::vector>();
```

**flatMap** - Merging nested containers into the outer iterator
```cpp
std::vector<std::pair<std::string, std::vector<int>>> input = {
    {"first pair", {1337, 42}},
    {"second pair", {6, 123, 7888}}
};
std::vector<int> output = CXXIter::from(std::move(input))
	.flatMap([](auto&& item) { return std::get<1>(item); })
	.collect<std::vector>();
```

**zip** - Zipping two iterators together so they run in parallel (yielding pairs)
```cpp
std::vector<std::string> input1 = {"1337", "42"};
std::vector<int> input2 = {1337, 42};
std::vector<std::pair<std::string, int>> output = CXXIter::from(input1).copied()
        .zip(CXXIter::from(input2).copied())
        .collect<std::vector>();
```

### Consuming
Without calling a consumer method on the created iterator, no actual work is done.
There is a wide variety of consumers, ranging from simple aggregating ones, such as `count()`, or `sum()`, to the classics such as `collect()` and `forEach()`.


## Examples
For a large list of examples, have a look at the unit-tests in `tests/`.

## Benchmarks
CXXIter's design tries to avoid any structural slowdowns. As such, it does avoid using virtual dispatch in hot paths, as well as the usage of `std::function<>`.
Here are the benchmark results from the simple benchmark in the `tests/` folder on my machine (Ryzen 5800X), comparing a native, a C++20 ranges and a CXXIter implementation of each micro-benchmark.
```
-----------------------------------------------------------------------------------------
Benchmark                                               Time             CPU   Iterations
-----------------------------------------------------------------------------------------
FilterMap_Native/Large/min_time:10.000      2901615173 ns   2901558802 ns            5
FilterMap_CXX20Ranges/Large/min_time:10.000 2947774570 ns   2946858702 ns            5
FilterMap_CXXIter/Large/min_time:10.000     2986734484 ns   2986205047 ns            5

FilterMap_Native/Small/min_time:10.000             204 ns          204 ns     68795396
FilterMap_CXX20Ranges/Small/min_time:10.000        197 ns          197 ns     71459697
FilterMap_CXXIter/Small/min_time:10.000            204 ns          204 ns     69366238


Filter_Native/Large/min_time:10.000          391584224 ns    391479324 ns           36
Filter_CXX20Ranges/Large/min_time:10.000     435718595 ns    435689028 ns           32
Filter_CXXIter/Large/min_time:10.000         405839687 ns    405818030 ns           35

Filter_Native/Small/min_time:10.000               62.1 ns         62.1 ns    228092966
Filter_CXX20Ranges/Small/min_time:10.000          66.3 ns         66.3 ns    211273332
Filter_CXXIter/Small/min_time:10.000              62.2 ns         62.2 ns    223274566


Map_Native/Large/min_time:10.000             827658901 ns    827441763 ns           17
Map_CXX20Ranges/Large/min_time:10.000        830283730 ns    830172863 ns           17
Map_CXXIter/Large/min_time:10.000            832004965 ns    831876755 ns           17

Map_Native/Small/min_time:10.000                  73.5 ns         73.5 ns    190828412
Map_CXX20Ranges/Small/min_time:10.000             75.3 ns         75.3 ns    186357996
Map_CXXIter/Small/min_time:10.000                 72.6 ns         72.6 ns    194156850


Cast_Native/Large/min_time:10.000            328406589 ns    328362799 ns           43
Cast_CXX20Ranges/Large/min_time:10.000       325023974 ns    324994770 ns           43
Cast_CXXIter/Large/min_time:10.000           482944749 ns    482808030 ns           29

Cast_Native/Small/min_time:10.000                 64.6 ns         64.6 ns    217301590
Cast_CXX20Ranges/Small/min_time:10.000            63.4 ns         63.4 ns    221671894
Cast_CXXIter/Small/min_time:10.000                72.0 ns         72.0 ns    194525183


GroupBy_Native/Large/min_time:10.000        1630731999 ns   1630347772 ns            9
GroupBy_CXXIter/Large/min_time:10.000       2114174670 ns   2114070369 ns            7

GroupBy_Native/Small/min_time:10.000               205 ns          205 ns     67938042
GroupBy_CXXIter/Small/min_time:10.000              275 ns          275 ns     50986762
```
Time: lower is better,
Iterations: higher is better

## Including In Your Project
To include CXXIter in your cmake project, you can do this:
```cmake
include(FetchContent)

# fetching CXXIter from github
FetchContent_Declare(
	CXXIter
	GIT_REPOSITORY "https://github.com/seijikun/CXXIter"
	GIT_TAG master
)
FetchContent_MakeAvailable(CXXIter)

# "link" your project against CXXIter, which adds the correct include paths
target_link_libraries(${PROJECT_NAME} PRIVATE CXXIter)

```
