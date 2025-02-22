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
	/// Holds an arbitrary tree of hashing data for storage alongside a preset entry.  and a vector of other nodes, as well as another vector indicating the length of the sublists which may be present.
	struct HashingData {
		/// Constructor method used to instantiate a HashingData object in memory.
		HashingData(uint64_t hash, std::vector<HashingData> constituents) :
		    m_Hash(hash), m_Constituents(constituents) {}

		/// The hash of the entity represented by this hash.
		uint64_t m_Hash;
		/// A list of the hash data for all the components of this thing.
		std::vector<HashingData> m_Constituents;
		/// A list of integers for handling the subcollections of components of this thing.
		std::vector<size_t> m_ParseValues;
	};
} // namespace RTE