#include "clock.hpp"
#include "src/htable.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>

static const std::size_t nruns = 100;
static std::vector<std::string> ks;
static std::vector<std::string> vs;

struct SimpleHasher {
	const std::size_t operator()(const std::string& s) const {
		return s.length();
	}
};

void read_from_file(const std::string& file_name, std::vector<std::string>& ks,
					std::vector<std::string>& vs) {
	std::ifstream file(file_name);
	while (true) {
		std::string* k = new std::string();
		if (!std::getline(file, *k)) break;
		std::string* v = new std::string();
		if (!std::getline(file, *v)) break;
		ks.emplace_back(*k);
		vs.emplace_back(*v);
	}
}

void prepare_bench_suite() {
	static const char* filename = "benchmark/dictionary.txt";
	read_from_file(filename, ks, vs);
}

void benchmark_unp() {
	Clock clock{"std::unordered_map", nruns};

	for (std::size_t i = 0; i < nruns; ++i) {
		std::unordered_map<std::string, std::string> table;
		for (std::size_t i = 0; i < ks.size(); ++i) {
			table[ks[i]] = vs[i];
		}

		for (std::size_t i = 0; i < ks.size(); ++i) {
			if (table.find(ks[i])->second != vs[i]) {
				exit(0);
			}
		}
	}
}

void benchmark_table() {
	Clock clock{"ntk::HTable", nruns};

	for (std::size_t i = 0; i < nruns; ++i) {
		ntk::HTable<std::string, std::string> table;
		for (std::size_t i = 0; i < ks.size(); ++i) {
			table.set(ks[i], vs[i]);
		}

		for (std::size_t i = 0; i < ks.size(); ++i) {
			if (table.find(ks[i]).value != vs[i]) {
				exit(0);
			}
		}
	}
}

void benchmark() {
	prepare_bench_suite();
	benchmark_unp();
	benchmark_table();
	std::cout << "\n\n";
}

int main() {
	ntk::HTable<std::string, std::size_t> table;
	benchmark();
	return 0;
}