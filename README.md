# CXXIter

CXXIter is a ergonomic C++ Iterator interface for STL containers, similar to the iterators found in Rust or C#'s LINQ.
It supports passing values by (const) reference or by using move semantics, which is tricky in some places, since references can not be stored in STL containers.

## The API

A full API-Documentation can be found [here](https://seijikun.github.io/CXXIter/).

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
-----------------------------------------------------------------------------------
Benchmark                                         Time             CPU   Iterations
-----------------------------------------------------------------------------------
FilterMap_Native                      3068779086 ns   3068000455 ns            4
FilterMap_CXX20Ranges                 3219577931 ns   3219559105 ns            4
FilterMap_CXXIter                     3152898478 ns   3152911409 ns            4

Filter_Native                          398029908 ns    398031220 ns           35
Filter_CXX20Ranges                     448209324 ns    448207127 ns           32
Filter_CXXIter                         412271366 ns    412271629 ns           34

Map_Native                             827857987 ns    827854529 ns           17
Map_CXX20Ranges                        828180531 ns    828175523 ns           17
Map_CXXIter                            832396096 ns    832392214 ns           17

Cast_Native                            339857189 ns    339838111 ns           41
Cast_CXX20Ranges                       332377684 ns    332377654 ns           42
Cast_CXXIter                           482904938 ns    482907990 ns           29

GroupBy_Native                        1616139752 ns   1616134295 ns            9
GroupBy_CXXIter                       1890404863 ns   1890360021 ns            7
```
The results show that CXXIter seems to be suspiciously slow with the cast operation. This effect only appears on GCC as far as I could see.

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