#include "src/htable.hpp"
#include <cstdint>
#include <string>

struct SHasher {
	std::int64_t operator()(const std::string& s) const {
		return s.length();
	}
};

int main() {
	ntk::HTable<std::string, int> table;

	printf("%d\n", table.set("heeheee", 123));
	printf("%d\n", table.set("foofoo", 789));
	printf("%d\n", table.set("aaabbb", 456));
	printf("%d\n", table["foofoo"]);
	printf("%d\n", table["heeheee"]);
	printf("%d\n", table["aaabbb"]);
	return 0;
}