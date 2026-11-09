#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <string_view>
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

	/// The bits that stand for a value in a hash: a number, bool or enum as itself, with floating point numbers going by their bits so 1.2 and 1.7 differ,
	/// a string by its text, a key and value pair by both in order, and anything else by its own Hash function.
	/// The Writer gets an item's hash through this too, so what Hash keeps and what Write compares it against can't drift apart.
	/// @param value The value to hash.
	/// @return The value's hash.
	template <typename Type> uint64_t PartHash(const Type& value) {
		if constexpr (std::is_floating_point_v<Type>) {
			return std::bit_cast<uint64_t>(static_cast<double>(value));
		} else if constexpr (std::is_arithmetic_v<Type> || std::is_enum_v<Type>) {
			return static_cast<uint64_t>(value);
		} else if constexpr (std::is_same_v<Type, std::string>) {
			return Hash(value);
		} else if constexpr (requires { value.first; value.second; }) {
			uint64_t pairHash = 0;
			AddToHash(pairHash, PartHash(value.first));
			AddToHash(pairHash, PartHash(value.second));
			return pairHash;
		} else {
			return value.Hash().m_Hash;
		}
	}

	/// The hash of which preset something is, as a preset reference or an unaltered copy of one is compared, by both Hash and the Writer.
	/// @param preset A pointer to the preset or copy. It can't be null.
	/// @return The hash of which preset it is.
	template <typename Pointer> uint64_t PresetHash(const Pointer& preset) { return Hash(preset->GetEntityCharacteristic()); }

	/// Includes every item of a list, for lists that don't leave any out.
	struct AlwaysIncluded {
		template <typename... Items> bool operator()(const Items&...) const { return true; }
	};

	/// A preset's hash, along with what its Write needs to compare its parts against. Hash fills these in the order that Write takes them back out.
	/// Each Add method below fills in what the Writer method of the same name takes out, so a Hash function reads the same as its Write.
	struct HashingData {
		/// Constructor method used to instantiate a HashingData object in system memory.
		/// @param hash The hash which this will initially store.
		HashingData(uint64_t hash) :
		    m_Hash(hash) {}

		/// Mixes in a value that Write compares directly, so nothing is kept for it: a number, bool or enum, a string, or anything with a Hash function of its own.
		/// @param value The value to mix in.
		template <typename Type> void Add(const Type& value) { AddToHash(m_Hash, PartHash(value)); }

		/// Mixes in a collection whose order means nothing, such as an unordered_set or unordered_map, so that equal collections hash the same however their items happen to be laid out.
		/// @param items The items, each a value Add takes, or a key and value pair of them.
		template <typename Collection> void AddUnordered(const Collection& items) {
			uint64_t itemHashSum = 0;
			for (const auto& item: items) {
				// Addition doesn't care which item comes first.
				itemHashSum += PartHash(item);
			}
			AddToHash(m_Hash, itemHashSum);
			AddToHash(m_Hash, items.size());
		}

		/// Mixes in a part, and keeps its hash for Writer::NewDistinctHashedProperty to compare.
		/// @param part The part, which has a Hash function of its own.
		template <typename Type> void AddHashed(const Type& part) { AddConstituent(PartHash(part)); }

		/// Mixes in a part that's always there, and keeps its hash for Writer::NewEntityPointerProperty to compare.
		/// @param part A pointer to the part, raw or smart. It can't be null.
		template <typename Pointer> void AddEntityPointer(const Pointer& part) { AddHashed(*part); }

		/// Mixes in a part that may be missing, and keeps whether it's there and its hash for Writer::NewOptionalEntityPointerProperty to compare.
		/// @param part A pointer to the part, raw or smart, or nullptr if there isn't one.
		template <typename Pointer> void AddOptionalEntityPointer(const Pointer& part) {
			m_ParseValues.push_back(part != nullptr);
			if (part != nullptr) {
				AddHashed(*part);
			}
		}

		/// Mixes in a list of parts held by value, and keeps how many and their hashes for Writer::NewSequence to compare.
		/// @param items The parts.
		template <typename Collection> void AddSequence(const Collection& items) {
			AddEach(items, [](const auto& item) { return PartHash(item); });
		}

		/// Mixes in a list of parts held by pointer, and keeps how many and their hashes for Writer::NewPointerSequence to compare.
		/// @param items The parts.
		template <typename Collection> void AddPointerSequence(const Collection& items) {
			AddPointerSequenceConditional(items, AlwaysIncluded());
		}

		/// Mixes in the parts of a list held by pointer that are included, and keeps how many and their hashes for Writer::NewPointerSequenceConditional to compare.
		/// @param items The parts.
		/// @param isIncluded Whether a part is one to consider at all, the same as is given to the Writer.
		template <typename Collection, typename IsIncluded> void AddPointerSequenceConditional(const Collection& items, IsIncluded&& isIncluded) {
			AddEach(items, [](const auto& item) { return PartHash(*item); }, isIncluded);
		}

		/// Mixes in which preset a reference points at, which Writer::NewPresetReferenceProperty compares directly.
		/// @param preset A pointer to the preset, or nullptr if there's none.
		template <typename Pointer> void AddPresetReference(const Pointer& preset) { AddToHash(m_Hash, preset != nullptr ? PresetHash(preset) : 0); }

		/// Mixes in a list of presets by which preset each is, and keeps how many and which for Writer::NewPresetCopySequence or Writer::NewPresetNameSequence to compare.
		/// @param items The presets, or unaltered copies of them.
		template <typename Collection> void AddPresetSequence(const Collection& items) {
			AddEach(items, [](const auto& item) { return PresetHash(item); });
		}

		/// The hash of the object this represents.
		uint64_t m_Hash;
		/// The hashes of the parts of the object this represents.
		std::deque<uint64_t> m_Constituents;
		/// How many parts each optional property or list had, so that Write knows how many to take from the constituents.
		std::deque<size_t> m_ParseValues;

	private:
		/// Mixes in a part's hash, and keeps it for Write to compare.
		/// @param partHash The part's hash.
		void AddConstituent(uint64_t partHash) {
			m_Constituents.push_back(partHash);
			AddToHash(m_Hash, partHash);
		}

		/// Mixes in and keeps the hash of each part of a list that's included, then keeps how many there were.
		template <typename Collection, typename GetHash, typename IsIncluded = AlwaysIncluded> void AddEach(const Collection& items, GetHash&& getHash, IsIncluded&& isIncluded = {}) {
			size_t count = 0;
			for (const auto& item: items) {
				if (isIncluded(item)) {
					AddConstituent(getHash(item));
					count++;
				}
			}
			m_ParseValues.push_back(count);
		}
	};
} // namespace RTE
