#pragma once

#include <cmath>
#include <cstddef>
#include <iterator>
#include <stdint.h>
#include <vector>

namespace ntk {

template <typename Kty, typename Vty, class Kty_Hasher = std::hash<Kty>>
class HTable {
	static constexpr std::size_t InitialCapLog2 = 8;
	static constexpr std::size_t DefaultCapacity = pow(2, InitialCapLog2);
	static constexpr std::uint8_t GrowthFactor = 2;
	static constexpr float LoadFactor = 0.5;

	using hash_type = std::int64_t;
	using key_type = Kty;
	using value_type = Vty;

	const Kty_Hasher m_hasher;

	struct HTEntry {
		key_type key;
		value_type value;
		hash_type hash = -1; // hash value is cached

		bool operator==(const HTEntry& other) const {
			return hash == other.hash and key == other.key;
		}
	};

	HTEntry* m_entries = new HTEntry[DefaultCapacity];
	std::size_t m_num_entries = 0;
	std::size_t m_cap = DefaultCapacity;
	std::uint32_t m_growth_pow2 = InitialCapLog2;

	void ensure_capacity() {
		if (m_num_entries < m_cap * LoadFactor) return;
		std::size_t old_cap = m_cap;
		m_cap *= GrowthFactor;
		m_growth_pow2++;
		HTEntry* old_entries = m_entries;
		m_entries = new HTEntry[m_cap];

		for (std::size_t i = 0; i < old_cap; ++i) {
			HTEntry& entry = old_entries[i];
			if (entry.hash == -1) continue;
			HTEntry& new_entry = get(entry.key, entry.hash);
			new_entry = entry;
		}

		delete[] old_entries;
	}

  public:
	explicit HTable() : m_hasher{Kty_Hasher()} {
		m_entries[0].hash = 0;
	}

	const HTEntry& null() const {
		return m_entries[0];
	}

	const HTEntry& find(const key_type& key) {
		HTEntry& entry = get(key, m_hasher(key));
		if (entry.hash == -1) return null();
		return entry;
	}

	const HTEntry& find_by_hash(const hash_type& hash, const key_type& key) {
		return get(key, hash);
	}

	bool set(const key_type& key, const value_type& value) {
		ensure_capacity();
		const hash_type& hash = m_hasher(key);
		HTEntry& entry = get(key, hash);
		if (entry == null()) return false;
		if (entry.hash == -1) m_num_entries++;
		entry.key = key;
		entry.value = value;
		entry.hash = hash;
		return true;
	}

	value_type& operator[](const key_type& key) {
		const hash_type hash = m_hasher(key);
		HTEntry& entry = get(key, hash);
		return entry.value;
	}

	~HTable() {
		delete[] m_entries;
	}

  private:
	HTEntry& get(const key_type& key, const hash_type& hash) {
		std::size_t cap = m_cap - 1;
		std::size_t first_idx = hash & cap;
		std::size_t index = first_idx;
		std::int32_t x = 1;
		while (true) {
			HTEntry& entry = m_entries[index];
			if (entry.key == key or entry.hash == -1) return entry;
			index = (first_idx + ((x * x + x) >> 1)) & cap;
			x++;
		}

		return m_entries[0];
	}
};

} // namespace ntk