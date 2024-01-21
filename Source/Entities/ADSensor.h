#pragma once

#include "Serializable.h"
#include "Matrix.h"

namespace RTE {

	class Actor;

	/// The ray-casting sensor which triggers the door opening or closing, depending on the team of the Actor that broke the ray.
	class ADSensor : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

#pragma region Creation
		/// Constructor method used to instantiate an ADSensor object in system memory. Create() should be called before using the object.
		ADSensor();

		/// Makes the ADSensor object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override { return Serializable::Create(); }

		/// Creates an ADSensor to be identical to another, by deep copy.
		/// @param reference A reference to the ADSensor to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const ADSensor& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an ADSensor object before deletion from system memory.
		~ADSensor();

		/// Destroys and resets (through Clear()) the ADSensor object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets the starting position offset of this ADSensor from the owning ADoor position.
		/// @return The starting coordinates relative to the m_Pos of this' ADoor.
		Vector GetStartOffset() const { return m_StartOffset; }

		/// Sets the starting position offset of this ADSensor from the owning ADoor position.
		/// @param startOffsetValue The new starting coordinates relative to the m_Pos of this' ADoor.
		void SetStartOffset(const Vector& startOffsetValue) { m_StartOffset = startOffsetValue; }

		/// Gets the sensor ray vector out from the start offset's position.
		/// @return The sensor ray vector.
		Vector GetSensorRay() const { return m_SensorRay; }

		/// Sets the sensor ray vector out from the start offset's position.
		/// @param sensorRayValue The new sensor ray vector.
		void SetSensorRay(const Vector& sensorRayValue) { m_SensorRay = sensorRayValue; }
#pragma endregion

#pragma region Concrete Methods
		/// Casts the ray along the sensor vector and returns any Actor that was found along it.
		/// @param doorPos Position of this ADSensor's ADoor.
		/// @param doorRot Rotation of this ADSensor's ADoor.
		/// @param doorHFlipped Flipping of this ADSensor's ADoor.
		/// @param ignoreMOID Which MOID to ignore, if any.
		/// @return The root Actor of the first MOID hit by the sensor ray. 0 if none.
		Actor* SenseActor(const Vector& doorPos, const Matrix& doorRot, bool doorHFlipped = false, MOID ignoreMOID = g_NoMOID);
#pragma endregion

	protected:
		Vector m_StartOffset; //!< The offset of the sensor ray start relative to the position of its ADoor.
		Vector m_SensorRay; //!< The ray out from the offset.

		short m_Skip; //!< How many pixels to skip between sensing pixels.

	private:
		static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.

		/// Clears all the member variables of this ADSensor, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
