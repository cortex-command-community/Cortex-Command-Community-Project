#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>

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
	/// FNV-1a's prime, which is also what DeterminismChecker's StateHasher uses.
	static constexpr uint64_t c_FNVPrime = 0x100000001B3ULL;

	/// Mixes one value's bits into a hash. Order matters.
	/// @param hash The hash to mix into.
	/// @param bits The bits to mix in.
	constexpr void AddToHash(uint64_t& hash, uint64_t bits) {
		hash = (hash ^ bits) * c_FNVPrime;
	}

	/// Hashes a string in a cross-compiler/platform safe way (std::hash gives different results on different compilers).
	/// It can be worked out at compile time, which is how property names are hashed.
	/// From https://stackoverflow.com/a/66764681, under license https://creativecommons.org/licenses/by-sa/4.0/. Minor modifications.
	/// @param text Text string to hash.
	/// @return The hash result value.
	constexpr uint64_t Hash(std::string_view text) {
		uint64_t hash = 14695981039346656037ULL;
		for (char c: text) {
			hash ^= static_cast<uint64_t>(c);
			hash *= c_FNVPrime;
		}
		return hash;
	}
} // namespace RTE
