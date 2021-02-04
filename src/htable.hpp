#pragma once

#include <cstddef>
#include <iterator>
#include <vector>

namespace ntk {

template <typename Kty, typename Vty, class Kty_Hasher = std::hash<Kty>>
class HTable {
	static constexpr std::size_t DefaultCapacity = 16;
	static constexpr std::uint8_t GrowthFactor = 2;
	static constexpr float LoadFactor = 0.75;

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

	std::vector<HTEntry> m_entries{DefaultCapacity};

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
		const hash_type& hash = m_hasher(key);
		HTEntry& entry = get(key, hash);
		if (entry == null()) return false;
		entry.key = key;
		entry.value = value;
		entry.hash = hash;
		return true;
	}

	value_type& operator[](const key_type& key) {
		const hash_type hash = m_hasher(key);
		HTEntry& entry = get(key, hash);
		entry.hash = hash;
		entry.key = key;
		return entry.value;
	}

  private:
	HTEntry& get(const key_type& key, const hash_type& hash) {
		const std::size_t cap = m_entries.capacity();
		for (std::size_t index = hash % cap;; index = (index + 1) % cap) {
			HTEntry& entry = m_entries[index];
			if (entry.key == key or entry.hash == -1) return entry;
		}

		return m_entries[0];
	}
};

} // namespace ntk