#pragma once

#include "Hash.h"

#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace RTE {

	class Serializable;
	class Entity;

	/// A property's name, which is hashed along with its value. Written as a string literal, its hash is worked out at compile time.
	/// A name that's only known at runtime, such as a custom value's key, has to be passed as PropName(text) explicitly.
	struct PropName {
		/// Constructor method used to instantiate a PropName from a string literal, hashing it at compile time.
		/// @param text The name.
		template <size_t Length> consteval PropName(const char (&text)[Length]) :
		    m_Text(text, Length - 1), m_Hash(Hash(std::string_view(text, Length - 1))) {}

		/// Constructor method used to instantiate a PropName from a name that's only known at runtime.
		/// @param text The name. It has to outlive this PropName.
		explicit PropName(std::string_view text) :
		    m_Text(text), m_Hash(Hash(text)) {}

		std::string_view m_Text; //!< The name.
		uint64_t m_Hash; //!< The hash of the name.
	};

	/// Gets the hash of a value as HashWriter writes it, to compare a part against the reference's.
	/// @param value The value. A pointer to a Serializable is followed to what it points at.
	/// @return The value's hash.
	template <typename Type> uint64_t HashOf(const Type& value);

	/// Which object a getter given to a PropertyWriter belongs to: the class of a pointer to a member, or the parameter of a lambda.
	template <typename Getter> struct GetterOwner : GetterOwner<decltype(&std::remove_cvref_t<Getter>::operator())> {};
	template <typename Field, typename Class> struct GetterOwner<Field Class::*> { using Type = Class; };
	template <typename Result, typename Lambda, typename Argument> struct GetterOwner<Result (Lambda::*)(Argument) const> { using Type = std::remove_cvref_t<Argument>; };
	template <typename Getter> using GetterOwnerType = typename GetterOwner<std::remove_cvref_t<Getter>>::Type;

	/// What every writer has in common: a Serializable's WriteTo is written once against this, and each writer decides what writing a property means.
	/// IniWriter writes only what differs from the object's reference, as INI. HashWriter has no reference, so it takes in everything and hashes it.
	///
	/// A Serializable's WriteTo gives each property as a getter, a pointer to the member such as &MOSprite::m_Mass, or a lambda taking the object when the value is worked out.
	/// The writer applies the getter to the object being written and, if there is one, to its reference, so WriteTo never compares anything itself.
	///
	/// Each writer derives from this with itself as Derived, and must provide these, which this calls on it:
	/// NewProperty(PropName), operator<< for each kind of value, ObjectStart(className), ObjectEnd(), NoObject(), NewLine(), NewLineString(text), NewDivider(),
	/// WriteIdentity() for an Entity's CopyOf and name, and WriteUnordered(items, writeItem) for a collection whose order means nothing.
	template <typename Derived> class PropertyWriter {

	public:
		/// What's being written at one level, and what it's written against.
		struct Frame {
			const Serializable* m_Current; //!< The object being written.
			const Serializable* m_Reference; //!< What it's written against, or nullptr if everything about it is to be written.
			bool m_ReferenceIsPreset; //!< Whether the reference is a preset that the object is written as a CopyOf, as opposed to a new object of its class or the value it'll be read into.
		};

#pragma region Writing Operations
		/// Creates a new line and writes the name of the specified property, followed by its value, whatever the reference's is.
		/// @param propName The name of the property to be written.
		/// @param propValue The value of the property.
		template <typename Type> void NewPropertyWithValue(PropName propName, const Type& propValue) {
			Self().NewProperty(propName);
			Self() << propValue;
		}

		/// Gets whether a property of the object being written differs from its reference's, for what can't be written by the methods below. Without a reference, everything differs.
		/// @param getter A pointer to the member, or a lambda taking the object.
		/// @return Whether the property differs from the reference's.
		template <typename Getter> bool IsDistinct(Getter&& getter) {
			using Owner = GetterOwnerType<Getter>;
			const Owner* reference = Reference<Owner>();
			return !reference || Differs(std::invoke(getter, Current<Owner>()), std::invoke(getter, *reference));
		}

		/// Writes a property if it differs from the reference's. A Serializable is written against the reference's, which it'll be read into.
		/// @param propName The name of the property to be written.
		/// @param getter A pointer to the member, or a lambda taking the object.
		template <typename Getter> void NewDistinctProperty(PropName propName, Getter&& getter) {
			using Owner = GetterOwnerType<Getter>;
			const Owner* reference = Reference<Owner>();
			const auto& value = std::invoke(getter, Current<Owner>());

			if (!reference) {
				NewPropertyWithValue(propName, value);
				return;
			}

			const auto& referenceValue = std::invoke(getter, *reference);
			if (Differs(value, referenceValue)) {
				Self().NewProperty(propName);
				if constexpr (std::is_base_of_v<Serializable, std::remove_cvref_t<decltype(value)>>) {
					Self().WriteInto(value, referenceValue);
				} else {
					Self() << value;
				}
			}
		}

		/// Writes a reference to a preset if it points at a different preset than the reference's.
		/// @param propName The name of the property to be written.
		/// @param getter A pointer to the member, or a lambda taking the object, giving a pointer to the preset or nullptr.
		template <typename Getter> void NewPresetReferenceProperty(PropName propName, Getter&& getter) {
			using Owner = GetterOwnerType<Getter>;
			const Owner* reference = Reference<Owner>();
			const auto* preset = std::invoke(getter, Current<Owner>());

			if (!reference || preset != std::invoke(getter, *reference)) {
				NewPropertyWithPresetReference(propName, preset);
			}
		}

		/// Creates a new line and writes the name of the specified property, followed by a reference to a preset, in the form PresetMan::GetEntityPreset reads.
		/// @param propName The name of the property to be written.
		/// @param preset The preset referred to, or nullptr for none.
		template <typename Type> void NewPropertyWithPresetReference(PropName propName, const Type* preset) {
			Self().NewProperty(propName);

			if (preset == nullptr) {
				Self().NoObject();
			} else {
				Self().ObjectStart(preset->GetClassName());
				NewPropertyWithValue("CopyOf", preset->GetModuleAndPresetName());
				Self().ObjectEnd();
			}
		}

		/// Writes a part that may be missing if it differs from the reference's, or None if the reference has one and this doesn't.
		/// @param propName The name of the property to be written.
		/// @param getter A pointer to the member, or a lambda taking the object, giving a pointer to the part or nullptr.
		template <typename Getter> void NewOptionalEntityPointerProperty(PropName propName, Getter&& getter) {
			using Owner = GetterOwnerType<Getter>;
			const Owner* reference = Reference<Owner>();
			const auto& part = std::invoke(getter, Current<Owner>());
			const auto* referencePart = reference ? &*std::invoke(getter, *reference) : nullptr;

			if (part != nullptr) {
				if (!referencePart || HashOf(*part) != HashOf(*referencePart)) {
					NewPropertyWithValue(propName, *part);
				}
			} else if (referencePart) {
				Self().NewProperty(propName);
				Self().NoObject();
			}
		}

		/// Writes a part that's always there if it differs from the reference's.
		/// @param propName The name of the property to be written.
		/// @param getter A pointer to the member, or a lambda taking the object, giving a pointer to the part. It can't be null.
		template <typename Getter> void NewEntityPointerProperty(PropName propName, Getter&& getter) {
			using Owner = GetterOwnerType<Getter>;
			const Owner* reference = Reference<Owner>();
			const auto& part = std::invoke(getter, Current<Owner>());
			const auto* referencePart = reference ? &*std::invoke(getter, *reference) : nullptr;

			if (!referencePart || HashOf(*part) != HashOf(*referencePart)) {
				NewPropertyWithValue(propName, *part);
			}
		}

		/// Writes a list of items where order matters, as only what comes after the reference's list if that's all that differs, or else clears the list and writes all of it.
		/// @param clearPhrase The phrase used to wipe the contents of the list, if the reference's is incompatible.
		/// @param insertionPhrase The phrase used to push an item to the back of the list.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the list.
		template <typename Getter> void NewSequence(PropName clearPhrase, PropName insertionPhrase, Getter&& getter) {
			WriteSequence(clearPhrase, getter, [](const auto& item) { return HashOf(item); }, [&](const auto& item) { NewPropertyWithValue(insertionPhrase, item); }, AlwaysIncluded());
		}

		/// Writes a list of pointers where order matters, as only what comes after the reference's list if that's all that differs, or else clears the list and writes all of it.
		/// @param clearPhrase The phrase used to wipe the contents of the list, if the reference's is incompatible.
		/// @param insertionPhrase The phrase used to push an item to the back of the list.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the list.
		/// @param isIncluded Whether an item is one to consider at all, given the object the list belongs to and the item.
		template <typename Getter, typename IsIncluded = AlwaysIncluded> void NewPointerSequence(PropName clearPhrase, PropName insertionPhrase, Getter&& getter, IsIncluded&& isIncluded = {}) {
			WriteSequence(clearPhrase, getter, [](const auto& item) { return HashOf(*item); }, [&](const auto& item) { NewPropertyWithValue(insertionPhrase, *item); }, isIncluded);
		}

		/// Writes a list of preset references where order matters, comparing only which preset each is.
		/// Each is written as its class with the preset's module and name nested under it as PresetName, the form Loadout reads.
		/// @param clearPhrase The phrase used to wipe the contents of the list, if the reference's is incompatible.
		/// @param insertionPhrase The phrase used to push an item to the back of the list.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the list.
		template <typename Getter> void NewPresetNameSequence(PropName clearPhrase, PropName insertionPhrase, Getter&& getter) {
			WriteSequence(clearPhrase, getter, [](const auto& item) { return Hash(item->GetEntityCharacteristic()); },
			    [&](const auto& item) {
				    Self().NewProperty(insertionPhrase);
				    Self().ObjectStart(item->GetClassName());
				    NewPropertyWithValue("PresetName", item->GetModuleAndPresetName());
				    Self().ObjectEnd();
			    },
			    AlwaysIncluded());
		}

		/// Writes a list of unaltered copies of presets where order matters, comparing only which preset each is.
		/// Each is written as a copy of its preset, for properties which read an object instead of a preset reference.
		/// @param clearPhrase The phrase used to wipe the contents of the list, if the reference's is incompatible.
		/// @param insertionPhrase The phrase used to push an item to the back of the list.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the list.
		template <typename Getter> void NewPresetCopySequence(PropName clearPhrase, PropName insertionPhrase, Getter&& getter) {
			WriteSequence(clearPhrase, getter, [](const auto& item) { return Hash(item->GetEntityCharacteristic()); },
			    [&](const auto& item) {
				    Self().NewProperty(insertionPhrase);
				    Self().ObjectStart(item->GetClassName());
				    NewPropertyWithValue("CopyOf", item->GetModuleAndPresetName());
				    Self().ObjectEnd();
			    },
			    AlwaysIncluded());
		}

		/// If a collection differs at all from the reference's, clears what was copied from the reference, if it had anything, and writes every item.
		/// @param clearPhrase The phrase used to wipe the contents of the collection.
		/// @param insertionPhrase The phrase used to add an item to the collection.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the collection.
		template <typename Getter> void NewDistinctCollection(PropName clearPhrase, PropName insertionPhrase, Getter&& getter) {
			WriteDistinctCollection(clearPhrase, getter, [&](const auto& item) { NewPropertyWithValue(insertionPhrase, item); }, false);
		}

		/// If a collection whose order means nothing differs at all from the reference's, clears what was copied from the reference, if it had anything, and writes every item.
		/// Its items hash the same however they happen to be laid out.
		/// @param clearPhrase The phrase used to wipe the contents of the collection.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the collection.
		/// @param writeItem Writes one item.
		template <typename Getter, typename WriteItem> void NewDistinctUnorderedCollection(PropName clearPhrase, Getter&& getter, WriteItem&& writeItem) {
			WriteDistinctCollection(clearPhrase, getter, writeItem, true);
		}
#pragma endregion

#pragma region Objects
		/// Gets the object being written, as the class whose WriteTo is asking.
		/// @return The object being written.
		template <typename Class> const Class& Current() const { return static_cast<const Class&>(*m_Frames.back().m_Current); }

		/// Gets what the object being written is written against, as the class whose WriteTo is asking.
		/// @return What the object is written against, or nullptr if everything about it is to be written.
		template <typename Class> const Class* Reference() const {
			const Serializable* reference = m_Frames.back().m_Reference;
			return reference ? &static_cast<const Class&>(*reference) : nullptr;
		}

		/// Gets whether the object being written is written as a CopyOf its reference.
		/// @return Whether the reference is a preset the object is a copy of.
		bool IsWrittenAsCopy() const { return m_Frames.back().m_ReferenceIsPreset; }

		/// Writes an object against a reference, through its WriteTo.
		/// @param object The object to write.
		/// @param reference What to write it against, or nullptr to write everything about it.
		/// @param referenceIsPreset Whether the reference is a preset the object is written as a CopyOf.
		void WriteObject(const Serializable& object, const Serializable* reference, bool referenceIsPreset) {
			m_Frames.push_back({&object, reference, referenceIsPreset});
			object.Write(Self());
			m_Frames.pop_back();
		}
#pragma endregion

	protected:
		std::vector<Frame> m_Frames; //!< What's being written at each level, the innermost last.

	private:
		/// Gets this as the writer it really is.
		/// @return This writer.
		Derived& Self() { return static_cast<Derived&>(*this); }

		/// Gets whether two values of a property differ, comparing them directly if they can be, or else by their hashes.
		/// @param value The value of the object being written.
		/// @param referenceValue The value of its reference.
		/// @return Whether they differ.
		template <typename Type> static bool Differs(const Type& value, const Type& referenceValue) {
			if constexpr (std::equality_comparable<Type>) {
				return value != referenceValue;
			} else {
				return HashOf(value) != HashOf(referenceValue);
			}
		}

		/// Writes a list where order matters, as only what comes after the reference's list if that's all that differs, or else clears the list and writes all of it.
		/// @param clearPhrase The phrase used to wipe the contents of the list, if the reference's is incompatible.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the list.
		/// @param getHash Gets the hash of an item, to compare against the reference's.
		/// @param writeItem Writes an item.
		/// @param isIncluded Whether an item has any counterpart in the reference's list at all, given the object the list belongs to and the item.
		template <typename Getter, typename GetHash, typename WriteItem, typename IsIncluded>
		void WriteSequence(PropName clearPhrase, Getter&& getter, GetHash&& getHash, WriteItem&& writeItem, IsIncluded&& isIncluded) {
			using Owner = GetterOwnerType<Getter>;
			const Owner& current = Current<Owner>();
			const Owner* reference = Reference<Owner>();
			const auto& items = std::invoke(getter, current);

			auto itemItr = items.begin();
			bool extendsReference = true;
			bool referenceHasItems = false;

			if (reference) {
				for (const auto& referenceItem: std::invoke(getter, *reference)) {
					if (!isIncluded(*reference, referenceItem)) {
						continue;
					}
					referenceHasItems = true;

					while (itemItr != items.end() && !isIncluded(current, *itemItr)) {
						itemItr++;
					}
					if (itemItr == items.end() || getHash(*itemItr) != getHash(referenceItem)) {
						extendsReference = false;
						break;
					}
					itemItr++;
				}
			}

			if (!extendsReference) {
				if (referenceHasItems) {
					NewPropertyWithValue(clearPhrase, 1);
				}
				itemItr = items.begin();
			}

			for (; itemItr != items.end(); itemItr++) {
				if (isIncluded(current, *itemItr)) {
					writeItem(*itemItr);
				}
			}
		}

		/// If a collection differs at all from the reference's, clears what was copied from the reference, if it had anything, and writes every item.
		/// @param clearPhrase The phrase used to wipe the contents of the collection.
		/// @param getter A pointer to the member, or a lambda taking the object, giving the collection.
		/// @param writeItem Writes one item.
		/// @param isUnordered Whether the collection's order means nothing, so that its items hash the same however they happen to be laid out.
		template <typename Getter, typename WriteItem> void WriteDistinctCollection(PropName clearPhrase, Getter&& getter, WriteItem&& writeItem, bool isUnordered) {
			using Owner = GetterOwnerType<Getter>;
			const Owner* reference = Reference<Owner>();
			const auto& items = std::invoke(getter, Current<Owner>());

			if (reference) {
				const auto& referenceItems = std::invoke(getter, *reference);
				if (items == referenceItems) {
					return;
				}
				if (!referenceItems.empty()) {
					NewPropertyWithValue(clearPhrase, 1);
				}
			}

			if (isUnordered) {
				Self().WriteUnordered(items, writeItem);
			} else {
				for (const auto& item: items) {
					writeItem(item);
				}
			}
		}
	};
} // namespace RTE
