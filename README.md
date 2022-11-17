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
Compiler: `gcc version 12.2.1 20221020`
```
--------------------------------------------------------------------------------------------
Benchmark                                                  Time             CPU   Iterations
--------------------------------------------------------------------------------------------
FilterMap_Native/Small                                   198 ns          198 ns     71457425
FilterMap_CXX20Ranges/Small                              182 ns          182 ns     67359112
FilterMap_CXXIter/Small                                  233 ns          233 ns     63686125
                                                  
FilterMap_Native/Large                            2883871358 ns   2883876073 ns            5
FilterMap_CXX20Ranges/Large                       3255969494 ns   3255896584 ns            5
FilterMap_CXXIter/Large                           3323312771 ns   3323321153 ns            4
                                                  
                                                  
Filter_Native/Large                                416454279 ns    416454614 ns           35
Filter_CXX20Ranges/Large                           381753661 ns    381728809 ns           37
Filter_CXXIter/Large                               480220190 ns    480220984 ns           29
                                                  
Filter_Native/Small                                     64.2 ns         64.2 ns    262228564
Filter_CXX20Ranges/Small                                65.6 ns         65.6 ns    212019379
Filter_CXXIter/Small                                    66.6 ns         66.6 ns    210251620
                                                  
                                                  
Map_Native/Large                                   726392652 ns    726391678 ns           19
Map_CXX20Ranges/Large                              709160787 ns    709064098 ns           20
Map_CXXIter/Large                                  730594430 ns    730572539 ns           19
                                                  
Map_Native/Small                                        68.6 ns         68.6 ns    167033097
Map_CXX20Ranges/Small                                   64.6 ns         64.6 ns    169229151
Map_CXXIter/Small                                       67.6 ns         67.6 ns    207433543
                                                  
                                                  
Cast_Native/Large                                  301692473 ns    301692325 ns           47
Cast_CXX20Ranges/Large                             300917110 ns    300850142 ns           47
Cast_CXXIter/Large                                 306349150 ns    306340990 ns           47
                                                  
Cast_Native/Small                                       80.4 ns         80.4 ns    173120146
Cast_CXX20Ranges/Small                                  66.7 ns         66.7 ns    208613349
Cast_CXXIter/Small                                      67.2 ns         67.2 ns    193698681
                                                  
                                                  
GroupBy_Native/Large                              1566267584 ns   1566269799 ns            9
GroupBy_CXXIter/Large                             1742169188 ns   1741891872 ns            8
                                                  
GroupBy_Native/Small                                     202 ns          202 ns     69780765
GroupBy_CXXIter/Small                                    274 ns          274 ns     51176489
                                                  
                                                  
ChunkedExactMath_CXXIter/Large                     113857930 ns    113858127 ns          125
ChunkedExactPtrMath_Native/Large                   110339030 ns    110290451 ns          127
ChunkedExactPtrMath_CXXIter/Large                  110315922 ns    110316119 ns          133
                                                  
ChunkedExactMath_CXXIter/Small                          13.5 ns         13.5 ns   1000000000
ChunkedExactPtrMath_Native/Small                        13.8 ns         13.8 ns    968896485
ChunkedExactPtrMath_CXXIter/Small                       15.1 ns         15.1 ns    927411168
                                                  
                                                  
OverlappingChunkedExactMath_CXXIter/Large          740346495 ns    740250478 ns           19
OverlappingChunkedExactPtrMath_Native/Large        300844689 ns    300844205 ns           46
OverlappingChunkedExactPtrMath_CXXIter/Large       330140465 ns    330066006 ns           44
                                                  
OverlappingChunkedExactMath_CXXIter/Small               37.3 ns         37.3 ns    385042581
OverlappingChunkedExactPtrMath_Native/Small             26.5 ns         26.5 ns    532771173
OverlappingChunkedExactPtrMath_CXXIter/Small            31.8 ns         31.8 ns    440700725
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
