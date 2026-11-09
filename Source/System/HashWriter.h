#pragma once

#include "PropertyWriter.h"

#include <bit>
#include <string>

namespace RTE {

	/// Hashes RTE objects by writing them with no reference, so that every property's name and value is mixed in, in the order they're written.
	/// Two objects hash the same when IniWriter would write them the same, so a part only needs writing when its hash differs from the reference's part.
	class HashWriter : public PropertyWriter<HashWriter> {

	public:
#pragma region Getters
		/// Gets the hash of everything written so far.
		/// @return The hash.
		uint64_t GetHash() const { return m_Hash; }
#pragma endregion

#pragma region Writing Operations
		/// Mixes in the class name of an object about to be written.
		/// @param className The class name of the object about to be written.
		void ObjectStart(const std::string& className) { AddToHash(m_Hash, Hash(className)); }

		/// Mixes in the end of an object, so that what follows it isn't taken as part of it.
		void ObjectEnd() { AddToHash(m_Hash, c_ObjectEndToken); }

		/// Line breaks and dividers are only layout, so they're not hashed.
		void NewLine(bool = true, int = 1) const {}
		void NewDivider(bool = true, int = 72) const {}

		/// Mixes in a line of text.
		/// @param textString The text.
		void NewLineString(const std::string& textString, bool = true) { AddToHash(m_Hash, Hash(textString)); }

		/// Mixes in the name of a property, whose value comes next.
		/// @param propName The name of the property.
		void NewProperty(PropName propName) { AddToHash(m_Hash, propName.m_Hash); }

		/// Mixes in that there is a null reference to an object here.
		void NoObject() { AddToHash(m_Hash, c_NoObjectToken); }

		/// Mixes in which preset an Entity is, and its name.
		void WriteIdentity();

		/// Mixes in a Serializable held by value. With no reference to compare against, it's hashed the same as any other.
		/// @param value The Serializable to hash.
		void WriteInto(const Serializable& value, const Serializable&) { *this << value; }

		/// Mixes in a collection whose order means nothing, so that equal collections hash the same however their items happen to be laid out.
		/// @param items The collection.
		/// @param writeItem Writes one item.
		template <typename Collection, typename WriteItem> void WriteUnordered(const Collection& items, WriteItem&& writeItem) {
			const uint64_t hashBefore = m_Hash;
			uint64_t itemHashSum = 0;
			for (const auto& item: items) {
				m_Hash = 0;
				writeItem(item);
				// Addition doesn't care which item comes first.
				itemHashSum += m_Hash;
			}
			m_Hash = hashBefore;
			AddToHash(m_Hash, itemHashSum);
			AddToHash(m_Hash, items.size());
		}
#pragma endregion

#pragma region Operator Overloads
		/// Mixes in a number, bool or enum. Floating point numbers go by their bits, so 1.2 and 1.7 hash differently.
		/// @param value The value.
		/// @return A HashWriter reference for further use in an expression.
		template <typename Type> requires(std::is_arithmetic_v<Type> || std::is_enum_v<Type>)
		HashWriter& operator<<(const Type& value) {
			if constexpr (std::is_floating_point_v<Type>) {
				AddToHash(m_Hash, std::bit_cast<uint64_t>(static_cast<double>(value)));
			} else {
				AddToHash(m_Hash, static_cast<uint64_t>(value));
			}
			return *this;
		}

		/// Mixes in a string.
		/// @param text The string.
		/// @return A HashWriter reference for further use in an expression.
		HashWriter& operator<<(const std::string& text) {
			AddToHash(m_Hash, Hash(text));
			return *this;
		}
		HashWriter& operator<<(const char* text) {
			AddToHash(m_Hash, Hash(text));
			return *this;
		}

		/// Mixes in everything about a Serializable.
		/// @param object The Serializable to hash.
		/// @return A HashWriter reference for further use in an expression.
		HashWriter& operator<<(const Serializable& object);

		/// Mixes in everything about a Serializable, or that there isn't one.
		/// @param object A pointer to the Serializable to hash, or nullptr.
		/// @return A HashWriter reference for further use in an expression.
		HashWriter& operator<<(const Serializable* object);
#pragma endregion

	private:
		static constexpr uint64_t c_ObjectEndToken = Hash("ObjectEnd"); //!< Mixed in at the end of each object.
		static constexpr uint64_t c_NoObjectToken = Hash("None"); //!< Mixed in for a null reference to an object.

		uint64_t m_Hash = 0; //!< The hash of everything written so far.
	};

	template <typename Type> uint64_t HashOf(const Type& value) {
		HashWriter writer;
		writer << value;
		return writer.GetHash();
	}
} // namespace RTE
