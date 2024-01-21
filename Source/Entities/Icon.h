#pragma once

#include "Entity.h"
#include "ContentFile.h"

namespace RTE {

	/// Represents an Icon in the interface that can be loaded and stored from different data modules etc.
	class Icon : public Entity {

	public:
		EntityAllocation(Icon);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate an Icon object in system memory. Create() should be called before using the object.
		Icon();

		/// Copy constructor method used to instantiate an Icon object identical to an already existing one.
		/// @param reference An Icon object which is passed in by reference.
		Icon(const Icon& reference);

		/// Makes the Icon object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates an Icon to be identical to another, by deep copy.
		/// @param reference A reference to the Icon to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Icon& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an Icon object before deletion from system memory.
		~Icon() override;

		/// Destroys and resets (through Clear()) the Icon object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Resets the entire Icon, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the number of frames in this Icon's animation.
		/// @return The number of frames in the animation.
		unsigned int GetFrameCount() const { return m_FrameCount; }

		/// Gets the array of 8-bit bitmaps of this Icon, as many as GetFrameCount says. Neither the array nor the BITMAPs are transferred ownership!
		/// @return The BITMAPs in 8bpp of this Icon.
		std::vector<BITMAP*> GetBitmaps8() const { return m_BitmapsIndexed; }

		/// Gets the array of 32-bit bitmaps of this Icon, as many as GetFrameCount says. Neither the array nor the BITMAPs are transferred ownership!
		/// @return The BITMAPs in 32bpp of this Icon.
		std::vector<BITMAP*> GetBitmaps32() const { return m_BitmapsTrueColor; }
#pragma endregion

#pragma region Operator Overloads
		/// An assignment operator for setting one Icon equal to another.
		/// @param rhs An Icon reference.
		/// @return A reference to the changed Icon.
		Icon& operator=(const Icon& rhs) {
			if (this != &rhs) {
				Destroy();
				Create(rhs);
			}
			return *this;
		}
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		ContentFile m_BitmapFile; //!< ContentFile containing the bitmap file of this Icon.
		unsigned int m_FrameCount; //!< Number of frames in this Icon's animation.

		std::vector<BITMAP*> m_BitmapsIndexed; //!< Vector containing the 8bpp BITMAPs of this Icon. BITMAPs are NOT owned!
		std::vector<BITMAP*> m_BitmapsTrueColor; //!< Vector containing the 32bpp BITMAPs of this Icon. BITMAPs are NOT owned!

	private:
		/// Clears all the member variables of this Icon, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
