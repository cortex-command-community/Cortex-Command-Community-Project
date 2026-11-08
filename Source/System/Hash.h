#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
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

	uint64_t Hash(const std::string& text);

	/// A preset's hash, along with what its Write needs to compare its parts against. Hash fills these in the order that Write takes them back out.
	/// Each Add method below fills in what the Writer method of the same name takes out, so a Hash function reads the same as its Write.
	struct HashingData {
		/// Constructor method used to instantiate a HashingData object in system memory.
		/// @param hash The hash which this will initially store.
		HashingData(uint64_t hash) :
		    m_Hash(hash) {}

		/// Mixes in a part, and keeps its hash for Writer::NewDistinctHashedProperty to compare.
		/// @param part The part, which has a Hash function of its own.
		template <typename Type> void AddHashed(const Type& part) { AddConstituent(part.Hash().m_Hash); }

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
			AddEach(items, [](const auto& item) { return item.Hash().m_Hash; }, [](const auto&) { return true; });
		}

		/// Mixes in a list of parts held by pointer, and keeps how many and their hashes for Writer::NewPointerSequence to compare.
		/// @param items The parts.
		template <typename Collection> void AddPointerSequence(const Collection& items) {
			AddPointerSequenceConditional(items, [](const auto&) { return true; });
		}

		/// Mixes in the parts of a list held by pointer that are included, and keeps how many and their hashes for Writer::NewPointerSequenceConditional to compare.
		/// @param items The parts.
		/// @param isIncluded Whether a part is one to consider at all, the same as is given to the Writer.
		template <typename Collection, typename IsIncluded> void AddPointerSequenceConditional(const Collection& items, IsIncluded&& isIncluded) {
			AddEach(items, [](const auto& item) { return item->Hash().m_Hash; }, isIncluded);
		}

		/// Mixes in a list of presets by which preset each is, and keeps how many and which for Writer::NewPresetCopySequence or Writer::NewPresetNameSequence to compare.
		/// @param items The presets, or unaltered copies of them.
		template <typename Collection> void AddPresetSequence(const Collection& items) {
			AddEach(items, [](const auto& item) { return Hash(item->GetEntityCharacteristic()); }, [](const auto&) { return true; });
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
		template <typename Collection, typename GetHash, typename IsIncluded> void AddEach(const Collection& items, GetHash&& getHash, IsIncluded&& isIncluded) {
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