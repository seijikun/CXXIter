# CXXIter

CXXIter is a ergonomic C++ Iterator interface for STL containers, similar to the iterators found in Rust or C#'s LINQ.
It supports passing values by (const) reference or by using move semantics, which is tricky in some places, since references can not be stored in STL containers.

## The API

### Entry
The `CXXIter` interface is entered by instantiating one of the possible source classes.
The type of the source determines how the values from the container are passed (const reference / reference / move semantics).

For that, there exist the three sources `CXXIter::SrcCRef`, `CXXIter::SrcRef`, and `CXXIter::SrcMov` respectively.

```c++
std::vector<float> input = {1.34f, 1.37f};

CXXIter::SrcCRef constRefIter(input);
CXXIter::SrcRef  mutableRefIter(input);
CXXIter::SrcMov  moveIter(std::move(input));
```

There is also the shortcut using `CXXIter::from()`, which tries to determine the type of the source class to use, depending on the type of the given input parameter:
```c++
const std::vector<float> constInput = {1.34f, 1.37f};
std::vector<float> input = {1.34f, 1.37f};

auto constRefIter   = CXXIter::from(constInput);
auto mutableRefIter = CXXIter::from(input);
auto moveIter       = CXXIter::from(std::move(input));
```

### Chaining
From there, everything returned by calling a member function either returns another iterator, or resolves the iterator to a final result. There are a lot of chain functions. Here **some examples**:

**cast** - Casting the elements of the iterator
```c++
std::vector<float> input = {1.35, 56.123};
std::vector<double> output = CXXIter::from(input)
	.cast<double>()
	.collect<std::vector>();
```

**filter** - Filtering elements of the iterator
```c++
std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
std::vector<int> output = CXXIter::from(input)
        .filter([](int item) { return (item % 2) == 0; })
        .collect<std::vector>();
```

**flatMap** - Merging nested containers into the outer iterator
```c++
std::vector<std::pair<std::string, std::vector<int>>> input = {
    {"first pair", {1337, 42}},
    {"second pair", {6, 123, 7888}}
};
std::vector<int> output = CXXIter::from(std::move(input))
	.flatMap([](auto&& item) { return std::get<1>(item); })
	.collect<std::vector>();
```

**zip** - Zipping two iterators together so they run in parallel (yielding pairs)
```c++
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
