#pragma once

#include <array>
#include <functional>

namespace std {

	/// Custom std::hash specialization to allow using std::array as key in hash table based containers.
	template <typename Type, size_t Size> struct hash<array<Type, Size>> {
		size_t operator()(const array<Type, Size>& arr) const {
			hash<Type> hasher;
			size_t outHash = 0;
			for (size_t i = 0; i < Size; ++i) {
				// Stolen from java.lang.String.hashCode. It seems to be a popular number, because it is prime, and 31 * x can be implemented quite efficiently as (x << 5) - x.
				outHash = outHash * 31 + hasher(arr[i]);
			}
			return outHash;
		}
	};
} // namespace std

namespace RTE {
	/// Holds an arbitrary tree of hashing data for storage alongside a preset entry.
	struct HashingData {
		/// Constructor method used to instantiate a HashingData object in memory.
		HashingData(uint64_t hash) :
		    m_Hash(hash), m_Constituents(), m_ParseValues() {}

		/// The hash of the Entity represented by this hash.
		uint64_t m_Hash;
		/// A list of the hash data for all components of this Entity.
		std::vector<uint64_t> m_Constituents;
		/// A list of integers for handling optional and collection properties by communicating their numeracy on a per-property basis.
		std::vector<size_t> m_ParseValues;
	};
} // namespace RTE