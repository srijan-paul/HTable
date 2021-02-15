#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <stdint.h>
#include <vector>

namespace ntk {

template <typename Kt, typename Vt, typename Ht = std::size_t>
struct HTEntry_ {
	Kt key;
	Vt value;
	Ht hash = 0; 
	std::size_t probe_count = 0;
};

template <typename Kt, typename Vt, typename Ht>
bool operator==(const HTEntry_<Kt, Vt, Ht>& a, const HTEntry_<Kt, Vt, Ht>& b) {
	return a.hash == b.hash and a.key == b.key;
}

template <typename Kty, typename Vty, class Kty_Hasher = std::hash<Kty>>
class HTable {
	static constexpr std::size_t InitialSizePowOf2 = 4; // the initial capacity of the hashtable as a power of 2
	static constexpr std::size_t DefaultCapacity = pow(2, InitialSizePowOf2);
	static constexpr std::uint8_t GrowthFactor = 2;
	// Grow the hashtable when it's at least 85% full.
	static constexpr float LoadFactor = 0.85;

	using hash_type = std::size_t;
	using key_type = Kty;
	using value_type = Vty;
	using HTEntry = HTEntry_<key_type, value_type, hash_type>;

	const Kty_Hasher m_hasher;

	template <typename Th, typename Rt>
	Rt& get(Th* pThis, const key_type& key, const hash_type hash) {
		std::size_t mask = pThis->m_cap - 1;
		std::size_t index = hash & mask;

		while (true) {
			HTEntry& entry = pThis->m_entries[index];
			if (entry.key == key or entry.hash == 0) return entry;
			index = (index + 1) & mask;
		}
		return pThis->m_entries[0];
	}


#define NTK_HT_GET_CONST(k, h) get<const HTable<Kty, Vty, Kty_Hasher>, const HTEntry>(this, k, h)
// We add 1 to the hash key, so that even if the hasher returned 0, 
// our hash is guaranteed to be positive. This is important because the hash value of `0` is
// reserved for uninitialized entries.
#define NTK_HASH_KEY(k) (m_hasher(k) + 1)
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
			if (entry.hash == 0) continue;
			HTEntry& new_entry = NTK_HT_GET(entry.key, entry.hash);
			new_entry = std::move(entry);
		}

		delete[] old_entries;
	}

  public:
	explicit HTable() : m_hasher{Kty_Hasher()} {
		m_entries[0].hash = 0;
	}

	// The first slot in the hashtable is treated as the "NULL"
	// value. So whenever there is a failed lookup, we return a 
	// reference to the first slot to indicate that no such key 
	// exists in the hashtable.
	const HTEntry& null() const {
		return m_entries[0];
	}

	const HTEntry& find(const key_type& key) const {
		return NTK_HT_GET_CONST(key, NTK_HASH_KEY(key));
	}

	HTEntry& find(const key_type& key) {
		return NTK_HT_GET(key, NTK_HASH_KEY(key));
	}

	bool set(key_type key, value_type value) {
		ensure_capacity();
		hash_type hash = NTK_HASH_KEY(key);
		std::size_t mask = m_cap - 1;

		std::size_t index = hash & mask;
		// The probe distance that we have covered so far.
		// Initially we are at our "desired" slot, where
		// our entry would ideally sit. So the probe distance is
		// 0.
		std::uint32_t dist = 0;

		while (true) {
			HTEntry& entry = m_entries[index];

			// If we found an unitialized spot 
			// in the entries list, use that slot.
			if (entry.hash == 0) {
				entry.key = std::move(key);
				entry.value = std::move(value);
				entry.hash = hash;
				entry.probe_count = dist;
				m_num_entries++;
				return true;
			}

			// if we found an intialized list that has the 
			// same key as the current key we're trying to set,
			// then change the valuye in that entry.
			if (entry.key == key) {
				entry.value = std::move(value);
				entry.probe_count = dist;
				return true;
			}

			// if we found an entry that isn't what 
			// we're looking for, but has a lower 
			// probe count, then we swap the entries
			// and keep moving.
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

	bool remove(const key_type& key) {
		return false;	
	}

	const value_type& operator[](const key_type& key) {
		return NTK_HT_GET_CONST(key, NTK_HASH_KEY(key)).value;
	}

	~HTable() {
		delete[] m_entries;
	}
};

#undef NTK_HT_GET
#undef NTK_HT_GET_CONST
#undef NTK_HASH_KEY

} // namespace ntk