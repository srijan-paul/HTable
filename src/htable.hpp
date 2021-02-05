#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
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
		std::uint32_t probe_count = 0;

		bool operator==(const HTEntry& other) const {
			return hash == other.hash and key == other.key;
		}
	};

	template <typename Th, typename Rt>
	Rt& get(Th* pThis, const key_type& key, const hash_type hash) {
		std::size_t mask = pThis->m_cap - 1;
		std::size_t index = hash & mask;

		while (true) {
			HTEntry& entry = pThis->m_entries[index];
			if (entry.key == key or entry.hash == -1) return entry;
			index = (index + 1) & mask;
		}
		return pThis->m_entries[0];
	}
#define NTK_HT_GET_CONST(k, h) get<const HTable<Kty, Vty, Kty_Hasher>, const HTEntry>(this, k, h)

#define NTK_HT_GET(k, h) get<HTable<Kty, Vty, Kty_Hasher>, HTEntry>(this, k, h)

	HTEntry* m_entries = new HTEntry[DefaultCapacity];
	std::size_t m_num_entries = 0;
	std::size_t m_cap = DefaultCapacity;

	void ensure_capacity() {
		if (m_num_entries < m_cap * LoadFactor) return;
		std::size_t old_cap = m_cap;
		m_cap *= GrowthFactor;
		HTEntry* old_entries = m_entries;
		m_entries = new HTEntry[m_cap];

		for (std::size_t i = 0; i < old_cap; ++i) {
			HTEntry& entry = old_entries[i];
			if (entry.hash == -1) continue;
			HTEntry& new_entry = NTK_HT_GET(entry.key, entry.hash);
			new_entry = std::move(entry);
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

	const HTEntry& find(const key_type& key) const {
		return NTK_HT_GET_CONST(key, m_hasher(key));
	}

	HTEntry& find(const key_type& key) {
		return NTK_HT_GET(key, m_hasher(key));
	}

	bool set(key_type key, value_type value) {
		ensure_capacity();
		hash_type hash = m_hasher(key);
		std::size_t mask = m_cap - 1;

		std::size_t index = hash & mask;
		std::uint32_t dist = 0;

		while (true) {
			HTEntry& entry = m_entries[index];

			if (entry.hash == -1) {
				entry.key = std::move(key);
				entry.value = std::move(value);
				entry.hash = hash;
				entry.probe_count = dist;
				m_num_entries++;
				return true;
			}

			if (entry.key == key) {
				entry.value = std::move(value);
				entry.probe_count = dist;
			}

			if (entry.probe_count < dist) {
				std::swap(hash, entry.hash);
				std::swap(key, entry.key);
				std::swap(value, entry.value);
				dist = entry.probe_count;
			}

			index = (index + 1) & mask;
			dist++;
		}
		return true;
	}

	const value_type& operator[](const key_type& key) {
		return NTK_HT_GET_CONST(key, m_hasher(key)).value;
	}

	~HTable() {
		delete[] m_entries;
	}
};

#undef NTK_HT_GET
#undef NTK_HT_GET_CONST

} // namespace ntk