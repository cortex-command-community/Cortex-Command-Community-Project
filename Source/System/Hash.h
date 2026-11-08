#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <deque>
#include <functional>
#include <type_traits>

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
	/// Mixes a number, bool or enum into a hash. Order matters, and floating point numbers go by their bits, so 1.2 and 1.7 hash differently.
	/// @param hash The hash to mix into.
	/// @param value The value to mix in.
	template <typename Type> void AddToHash(uint64_t& hash, Type value) {
		uint64_t bits;
		if constexpr (std::is_floating_point_v<Type>) {
			bits = std::bit_cast<uint64_t>(static_cast<double>(value));
		} else {
			bits = static_cast<uint64_t>(value);
		}
		// FNV-1a's prime, which is also what DeterminismChecker's StateHasher uses.
		hash = (hash ^ bits) * 0x100000001B3ULL;
	}

	/// A preset's hash, along with what its Write needs to compare its parts against. Hash fills these in the order that Write takes them back out.
	struct HashingData {
		/// Constructor method used to instantiate a HashingData object in system memory.
		/// @param hash The hash which this will initially store.
		HashingData(uint64_t hash) :
		    m_Hash(hash) {}

		/// The hash of the object this represents.
		uint64_t m_Hash;
		/// The hashes of the parts of the object this represents.
		std::deque<uint64_t> m_Constituents;
		/// How many parts each optional property or list had, so that Write knows how many to take from the constituents.
		std::deque<size_t> m_ParseValues;
	};
} // namespace RTE