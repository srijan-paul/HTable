# HTable
A fast hashtable data structure written in C++.

`ntk::Htable` is upto ~30-40% faster than clang and gcc's `std::unordered_map` on my benchmark suite.
The library is a single header file, so it is easy to include in your project.

`ntk::Htable` uses cache friendly open addressing and robin hood hashing scheme to acheive
it's speed.

# Usage

```cpp
// 1. include the header file
#include "htable.hpp"
#include <iostream>

int main() {
	// 2. default construct an HTable.
	ntk::HTable<std::string, std::string> table;

	// set
	table.set("foo", "bar");

	// get
	std::string value = table.get("foo");
	
	// delete
	table.del("foo");

	// check if entry exists
	auto entry = table.get("abc");
	if (entry == table.end()) {
		// does not exist
	} else {
		// table["abc"] exists.
	}

	// alternative use
	table["abc"] = "def";
	std::cout << table["abc"] << '\n';

	return 0;
};
```

# Run benchmarks.

You can either use the benchmark suite provided under `benchmark` directory, or
prepare your own test cases and put them in the `benchmark` folder.

```sh
clang++ -Wall -Werror -std=c++17 -O3 ./test.cpp -o ./test
./test
```
