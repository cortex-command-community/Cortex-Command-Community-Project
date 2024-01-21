#pragma once

/// Header file for the Deployment class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "RTETools.h"
#include "SceneObject.h"
#include "Vector.h"
#include "SceneMan.h"
// #include "MovableMan.h"

namespace RTE {

	class ContentFile;

	/// A special SceneObject that specifies a Loadout of whatever Tech is
	/// relevant to be placed in a specific location in a Scene.
	class Deployment : public SceneObject {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(Deployment);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a Deployment object in system
		/// memory. Create() should be called before using the object.
		Deployment();

		/// Destructor method used to clean up a Deployment object before deletion
		/// from system memory.
		~Deployment() override;

		/// Makes the Deployment object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Makes the Deployment object ready for use.
		/// @param loadoutName The name of the Loadout that this should invoke at this' position.
		/// @param icon Icon that represents this graphically.
		/// @param spawnRadius The radius around this deployment that gets checked if another
		/// actor/item of the same type and name already exists and will block
		/// re-spawning a new one by this.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(std::string loadoutName, const Icon& icon, float spawnRadius);

		/// Creates a Deployment to be identical to another, by deep copy.
		/// @param reference A reference to the Deployment to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const Deployment& reference);

		/// Resets the entire Deployment, including its inherited members, to
		/// their default settings or values.
		void Reset() override {
			Clear();
			SceneObject::Reset();
		}

		/// Destroys and resets (through Clear()) the Deployment object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets a bitmap showing a good identifyable icon of this, for use in
		/// GUI lists etc.
		/// @return A good identifyable graphical representation of this in a BITMAP, if
		/// available. If not, 0 is returned. Ownership is NOT TRANSFERRED!
		BITMAP* GetGraphicalIcon() const override { return !m_Icon.GetBitmaps8().empty() ? m_Icon.GetBitmaps8()[0] : nullptr; }

		/// Gets the name of the Loadout that this Deployment spawns.
		/// @return The name of the Loadout preset that this Deployment spawns.
		const std::string& GetLoadoutName() { return m_LoadoutName; }

		/// Gets a bitmap showing a good identifyable icon of this.
		/// @return The Icon that represents this graphically.
		Icon GetIcon() { return m_Icon; }

		/// Gets the radius around this deployment that gets checked if another
		/// actor/item of the same type and name already exists and will block
		/// re-spawning a new one by this
		/// @return The radius this Deployment will be checking within.
		float GetSpawnRadius() const { return m_SpawnRadius; }

		/// Indicates whether this' current graphical representation overlaps
		/// a point in absolute scene coordinates.
		/// @param scenePoint The point in absolute scene coordinates.
		/// @return Whether this' graphical rep overlaps the scene point.
		bool IsOnScenePoint(Vector& scenePoint) const override;

		/// Creates and returns the Actor that this Deployment dictates should
		/// spawn here. Ownership IS transferred!! All items of the Loadout of
		/// this Deployment will be added to the Actor's inventory as well (and
		/// also owned by it)
		/// @param player Which in-game player to create the delivery for.
		/// @param costTally A float which will be added to with the cost of the stuff returned here.
		/// @return The Actor instance, if any, that this Deployment is supposed to spawn.
		/// OWNERSHIP IS TRANSFERRED!
		Actor* CreateDeployedActor(int player, float& costTally);

		/// Creates and returns the Actor that this Deployment dictates should
		/// spawn here. Ownership IS transferred!! All items of the Loadout of
		/// this Deployment will be added to the Actor's inventory as well (and
		/// also owned by it)
		/// @param Which in-game player to create the delivery for.
		/// @return The Actor instance, if any, that this Deployment is supposed to spawn.
		/// OWNERSHIP IS TRANSFERRED!
		Actor* CreateDeployedActor();

		/// Creates and returns the first Device that Deployment dictates should
		/// spawn here. Ownership IS transferred!! Only the first Device is created.
		/// @param player Which in-game player to create the delivery for.
		/// @param costTally A float which will be added to with the cost of the stuff returned here.
		/// @return The Actor instance, if any, that this Deployment is supposed to spawn.
		/// OWNERSHIP IS TRANSFERRED!
		SceneObject* CreateDeployedObject(int player, float& costTally);

		/// Creates and returns the first Device that Deployment dictates should
		/// spawn here. Ownership IS transferred!! Only the first Device is created.
		/// @param Which in-game player to create the delivery for.
		/// @return The Actor instance, if any, that this Deployment is supposed to spawn.
		/// OWNERSHIP IS TRANSFERRED!
		SceneObject* CreateDeployedObject();

		/// Tests whether the Object this is supposed to spawn/deploy is blocked
		/// by an already exiting object in the a list being positioned within the
		/// spawn radius of this.
		/// @param player Which in-game player to create the delivery for.
		/// @param existingObjects A list of SceneObject:s that will be tested against to see if any
		/// sufficiently similar Object is positioned within the spawn radius of
		/// this.
		/// @return Whether the deployment spawning is blocked by one of the Objects in
		/// the list.
		bool DeploymentBlocked(int player, const std::list<SceneObject*>& existingObjects);

		/// Gets the cost to purchase this item, in oz's of gold.
		/// @param nativeModule If this is supposed to be adjusted for a specific Tech's subjective (default: 0)
		/// value, then pass in the native DataModule ID of that tech. 0 means
		/// no Tech is specified and the base value is returned.
		/// @param foreignMult How much to multiply the value if this happens to be a foreign Tech. (default: 1.0)
		/// @return The cost, in oz of gold.
		float GetGoldValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const override { return GetTotalValue(nativeModule, foreignMult, nativeMult); }

		/// DOES THE SAME THING AS GetGoldValue, USED ONLY TO PRESERVE LUA COMPATIBILITY
		float GetGoldValueOld(int nativeModule = 0, float foreignMult = 1.0) const override { return GetTotalValue(nativeModule, foreignMult, 1.0); }

		/// Gets the total liquidation value of a spawn of this, including
		/// everything carried by it.
		/// @param nativeModule If this is supposed to be adjusted for a specific Tech's subjective (default: 0)
		/// value, then pass in the native DataModule ID of that tech. 0 means
		/// no Tech is specified and the base value is returned.
		/// @param foreignMult How much to multiply the value if this happens to be a foreign Tech. (default: 1.0)
		/// @return The current value of this and all contained assets.
		float GetTotalValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const override;

		/// Return this deployment's unique ID
		/// @return This deployment's ID
		unsigned int GetID() const { return m_ID; };

		/// Clones id from the specified deployment
		/// @param from Deployment to clone Id from.
		void CloneID(Deployment* from) {
			if (from)
				m_ID = from->GetID();
		};

		/// Generates new random ID for this deployment.
		void NewID() { m_ID = RandomNum(1, 0xFFFF); };

		/// Draws this Deployment's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// like indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Returns whether this MOSprite is being drawn flipped horizontally
		/// (along the vertical axis), or not.
		/// @return Whether flipped or not.
		bool IsHFlipped() const override { return m_HFlipped; }

		/// Sets whether this should be drawn flipped horizontally (around the
		/// vertical axis).
		/// @param flipped A bool with the new value.
		void SetHFlipped(const bool flipped) override { m_HFlipped = flipped; }

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;

		// Name of the Loadout that shuold be placed at this' location in the Scene.
		std::string m_LoadoutName;
		// The Icon that graphically represents this
		Icon m_Icon;
		// The radius around this deployment that gets checked if another actor/item of the same type and name already exists and will block re-spawning a new one by this
		float m_SpawnRadius;
		// The radius around this deployment that gets checked if an actor spawned by this deployment is present. If it is, deployment is blocked.
		float m_WalkRadius;
		// Unique deployment id, assigned to units deployed by this deployment
		unsigned int m_ID;
		// Whether the deployment and it's loadout is flipped
		bool m_HFlipped;
		// Shared HFlipped arrow bitmaps
		static std::vector<BITMAP*> m_apArrowLeftBitmap;
		static std::vector<BITMAP*> m_apArrowRightBitmap;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Deployment, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Deployment(const Deployment& reference) = delete;
		void operator=(const Deployment& rhs) = delete;
	};

} // namespace RTE
