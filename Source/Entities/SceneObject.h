#pragma once

/// Header file for the SceneObject class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files, forward declarations, namespace stuff
#include "Entity.h"
#include "Matrix.h"

struct BITMAP;

namespace RTE {

	/// The base class shared by Both TerrainObject:s and MovableObject:s, ie
	/// anything that can be places in a scene.
	class SceneObject : public Entity {

		/// Public member variable, method and friend function declarations
	public:
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Enumeration for the different buyable modes of this SceneObject.
		enum class BuyableMode {
			NoRestrictions,
			BuyMenuOnly,
			ObjectPickerOnly,
			ScriptOnly
		};

		/// Acts as a small memory object that only holds a pointer to a reference
		/// instance and the most essential properties to eventually place a copy
		/// of that reference when needed.
		class SOPlacer :
		    public Serializable {

			/// Public member variable, method and friend function declarations
		public:
			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// Constructor method used to instantiate a SOPlacer object in system
			/// memory. Create() should be called before using the object.
			SOPlacer() { Clear(); }

			/// Creates a SOPlacer to be identical to another, by deep copy.
			/// @param reference A reference to the SOPlacer to deep copy.
			/// @return An error return value signaling sucess or any particular failure.
			/// Anything below 0 is an error signal.
			int Create(const SOPlacer& reference);

			/// Resets the entire Serializable, including its inherited members, to their
			/// default settings or values.
			void Reset() override { Clear(); }

			/// Gets the object reference to be placed. Owenership is NOT transferred!
			/// @return A pointer to the reference object to be copied and placed. Not transferred!
			const SceneObject* GetObjectReference() { return m_pObjectReference; }

			/// Gets the place offset from the parent's position/origin. If in a scene
			/// this will yield the absolute scene coordinates.
			/// @return The offset in pixels from the parent's position where this gets spawned.
			Vector GetOffset() const { return m_Offset; }

			/// Sets the place offset from the parent's position/origin.
			/// @param newOffset New offset.
			void SetOffset(Vector newOffset) { m_Offset = newOffset; }

			/// Gets the rotation angle of the object to be placed, in radians.
			/// @return The placement rotational angle, in radians.
			float GetRotation() const { return m_RotAngle; }

			/// Gets whether the placement is horizontally flipped or not.
			/// @return The horizontal flipping of the placement.
			bool GetHFlipped() const { return m_HFlipped; }

			/// Gets which team this is to be assigned to when placed.
			/// @return The team number this is to be assigned to when placed.
			int GetTeam() const { return m_Team; }

			/// Sets which team this is to be assigned to when placed.
			/// @param team The team number this is to be assigned to when placed.
			void SetTeam(int team) { m_Team = team; }

			/// Makes a copy of the preset instance, and applies the placement
			/// properties of this to it, finally returning it WITH OWNERSHIP.
			/// @param pParent The parent to place as offset from. If 0 is passed, the placement (default: 0)
			/// properties will be applied as absolutes instead of relative.
			/// @return The new copy with correct placement applied. OWNERSHIP IS TRANSFERRED!
			SceneObject* GetPlacedCopy(const SceneObject* pParent = 0) const;

			/// Protected member variable and method declarations
		protected:
			// The pointer to the preset instance, that copies of which will be placed. Not Owned!
			const SceneObject* m_pObjectReference;
			// Offset placement position from owner/parent's position/origin.
			Vector m_Offset;
			// The placement's rotational angle in radians.
			float m_RotAngle;
			// Whether horizontal flipping is part of the placement.
			bool m_HFlipped;
			// The team of the placed object
			int m_Team;

			/// Private member variable and method declarations
		private:
			static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

			/// Clears all the member variables of this SOPlacer, effectively
			/// resetting the members of this abstraction level only.
			void Clear();
		};

		/// Constructor method used to instantiate a SceneObject object in system
		/// memory. Create() should be called before using the object.
		SceneObject();

		/// Destructor method used to clean up a SceneObject object before deletion
		/// from system memory.
		~SceneObject() override;

		/// Makes the SceneObject object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override { return 0; }

		/// Creates an SceneObject to be identical to another, by deep copy.
		/// @param reference A reference to the SceneObject to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const SceneObject& reference);

		/// Resets the entire SceneObject, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}

		/// Destroys and resets (through Clear()) the SceneObject object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the absolute position of this SceneObject.
		/// @return A Vector describing the current absolute position in pixels.
		const Vector& GetPos() const { return m_Pos; }

		/// Sets the absolute position of this SceneObject in the scene.
		/// @param newPos A Vector describing the current absolute position in pixels.
		void SetPos(const Vector& newPos) { m_Pos = newPos; }

		/// Returns whether this is being drawn flipped horizontally (around the
		/// vertical axis), or not.
		/// @return Whether flipped or not.
		virtual bool IsHFlipped() const { return false; }

		/// Gets the current rotational Matrix of of this MovableObject.
		/// @return The rotational Matrix of this MovableObject.
		virtual Matrix GetRotMatrix() const { return Matrix(); }

		/// Gets the current rotational angle of of this, in radians.
		/// @return The rotational angle of this, in radians.
		virtual float GetRotAngle() const { return 0; }

		/// Sets whether this should be drawn flipped horizontally (around the
		/// vertical axis).
		/// @param flipped A bool with the new value.
		virtual void SetHFlipped(const bool flipped) {}

		/// Sets the current absolute angle of rotation of this.
		/// @param newAngle The new absolute angle in radians.
		virtual void SetRotAngle(float newAngle) {}

		/// Sets which team this belongs to.
		/// @param team The assigned team number.
		virtual void SetTeam(int team) { m_Team = team; }

		/// Gets which team this belongs to.
		/// @return The currently assigned team number.
		int GetTeam() const { return m_Team; }

		/// Sets which player placed this object in the scene, if any.
		/// @param player The player responsible for placing this is in the scene, if any.
		void SetPlacedByPlayer(int player) { m_PlacedByPlayer = player; }

		/// Gets which player placed this object in the scene, if any.
		/// @return The player responsible for placing this is in the scene, if any.
		int GetPlacedByPlayer() const { return m_PlacedByPlayer; }

		/// Gets the cost to purchase this item, in oz's of gold.
		/// @param nativeModule If this is supposed to be adjusted for a specific Tech's subjective (default: 0)
		/// value, then pass in the native DataModule ID of that tech. 0 means
		/// no Tech is specified and the base value is returned.
		/// @param foreignMult How much to multiply the value if this happens to be a foreign Tech. (default: 1.0)
		/// @param nativeMult How much to multiply the value if this happens to be a native Tech. (default: 1.0)
		/// @return The cost, in oz of gold.
		virtual float GetGoldValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const;

		/// Sets the cost to purchase this item, in oz's of gold.
		/// @param value The cost, in oz of gold.
		void SetGoldValue(float value) { m_OzValue = value; }

		/// DOES THE SAME THING AS GetGoldValue, USED ONLY TO PRESERVE LUA COMPATIBILITY
		virtual float GetGoldValueOld(int nativeModule, float foreignMult) const { return GetGoldValue(nativeModule, foreignMult, 1.0); }

		/// Gets a descriptive string describing the cost to purchase this item,
		/// in oz's of gold.
		/// @param nativeModule If this is supposed to be adjusted for a specific Tech's subjective (default: 0)
		/// value, then pass in the native DataModule ID of that tech. 0 means
		/// no Tech is specified and the base value is returned.
		/// @param foreignMult How much to multiply the value if this happens to be a foreign Tech. (default: 1.0)
		/// @return The cost, described in a friendly to read string: "100oz", or "Free"
		std::string GetGoldValueString(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const;

		/// Gets the total liquidation value of this, including everything inside.
		/// @param nativeModule If this is supposed to be adjusted for a specific Tech's subjective (default: 0)
		/// value, then pass in the native DataModule ID of that tech. 0 means
		/// no Tech is specified and the base value is returned.
		/// @param foreignMult How much to multiply the value if this happens to be a foreign Tech. (default: 1.0)
		/// @return The current value of this and all contained assets.
		virtual float GetTotalValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const { return GetGoldValue(nativeModule, foreignMult, nativeMult); }

		/// Shows whether this should appear in teh buy menus at all.
		/// @return Buyable or not.
		bool IsBuyable() const { return m_Buyable; }

		/// Gets the BuyableMode of this SceneObject.
		/// @return The BuyableMode of this SceneObject
		BuyableMode GetBuyableMode() const { return m_BuyableMode; }

		/// Gets whether this SceneObject is available only in the BuyMenu list when buyable.
		/// @return Whether this SceneObject is available only in the BuyMenu list when buyable.
		bool IsBuyableInBuyMenuOnly() const { return m_BuyableMode == BuyableMode::BuyMenuOnly; }

		/// Gets whether this SceneObject is available only in the ObjectPicker list when buyable.
		/// @return Whether this SceneObject is available only in the ObjectPicker list when buyable.
		bool IsBuyableInObjectPickerOnly() const { return m_BuyableMode == BuyableMode::ObjectPickerOnly; }

		/// Gets whether this SceneObject is available only by lua functions like CreateRandom
		/// @return Whether this SceneObject is available only in the AI list when buyable.
		bool IsBuyableInScriptOnly() const { return m_BuyableMode == BuyableMode::ScriptOnly; }

		/// Gets a bitmap showing a good identifyable icon of this, for use in
		/// GUI lists etc.
		/// @return A good identifyable graphical representation of this in a BITMAP, if
		/// available. If not, 0 is returned. Ownership is NOT TRANSFERRED!
		virtual BITMAP* GetGraphicalIcon() const { return nullptr; }

		/// Indicates whether this' current graphical representation overlaps
		/// a point in absolute scene coordinates.
		/// @param scenePoint The point in absolute scene coordinates.
		/// @return Whether this' graphical rep overlaps the scene point.
		virtual bool IsOnScenePoint(Vector& scenePoint) const { return false; }

		/// Updates this SceneObject. Supposed to be done every frame.
		virtual void Update() {}

		/// Updates the full state of this object in one call.
		virtual void FullUpdate() { Update(); }

		/// Function called after everything has finished updating.
		virtual void PostUpdate(){};

		/// Draws this SceneObject's current graphical representation to a BITMAP of
		/// choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// like indicator arrows or hovering HUD text and so on.
		virtual void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const = 0;

		/// Draws team sign this terrain object belongs to.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		void DrawTeamMark(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) const;

		/// Protected member variable and method declarations
	protected:
		// Forbidding copying
		SceneObject(const SceneObject& reference) = delete;
		SceneObject& operator=(const SceneObject& rhs) { return *this; }

		// Member variables
		static Entity::ClassInfo m_sClass;
		// Absolute position of the center of this in the scene, in pixels
		Vector m_Pos;
		// How much this SceneObject costs to purchase, in oz's of gold.
		float m_OzValue;
		// Whether this shows up in the buy menu at all
		bool m_Buyable;

		BuyableMode m_BuyableMode; //!< In which buy lists this SceneObject is available when buyable.

		// The team this object belongs to. -1 if none.
		int m_Team;
		// The player this was placed by in edit mode
		int m_PlacedByPlayer;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this SceneObject, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
