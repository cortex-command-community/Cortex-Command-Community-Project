#pragma once

/// Header file for the BunkerAssembly class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "TerrainObject.h"

namespace RTE {

	class BunkerAssemblyScheme;
	class Deployment;

	/// An assembly of a few terrain objects.
	/// material layer and optional background layer.
	class BunkerAssembly : public TerrainObject {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(BunkerAssembly);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a BunkerAssembly object in system
		/// memory. Create() should be called before using the object.
		BunkerAssembly();

		/// Destructor method used to clean up a BunkerAssembly object before deletion
		/// from system memory.
		~BunkerAssembly() override;

		/// Makes the BunkerAssembly object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Makes the BunkerAssembly object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(BunkerAssemblyScheme* scheme);

		/// Creates a BunkerAssembly to be identical to another, by deep copy.
		/// @param reference A reference to the BunkerAssembly to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const BunkerAssembly& reference);

		/// Resets the entire BunkerAssembly, including its inherited members, to
		/// their default settings or values.
		void Reset() override {
			Clear();
			SceneObject::Reset();
		}

		/// Destroys and resets (through Clear()) the BunkerAssembly object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Description:
		/// Return value:
		std::string GetParentAssemblySchemeName() const { return m_ParentAssemblyScheme; }

		/// Indicates whether this' current graphical representation overlaps
		/// a point in absolute scene coordinates.
		/// @param scenePoint The point in absolute scene coordinates.
		/// @return Whether this' graphical rep overlaps the scene point.
		bool IsOnScenePoint(Vector& scenePoint) const override;

		/// Retrieves the list of random deployemtns selected to be deployed by this assembly
		/// based on it's parent scheme MaxDeployments value. This list will always include all
		/// brain deployments so it can be longer that MaxDeployments.
		/// @return List of deployments.
		std::vector<Deployment*> GetDeployments();

		/// Sets which team this belongs to.
		/// @param team The assigned team number.
		void SetTeam(int team) override;

		/// Gets the list of SceneObject:s which are placed in this assembly on loading.
		/// @return The list of of placed objects. Ownership is NOT transferred!
		const std::list<SceneObject*>* GetPlacedObjects() const { return &m_PlacedObjects; }

		/// Adds placed object to the internallist of placed objects for this assembly,
		/// applies it's image to presentation bitmap and sets assembly price accordingly.
		/// Added scene object MUST have coordinates relative to this assembly.
		/// @param pSO Object to add.
		void AddPlacedObject(SceneObject* pSO);

		/// Gets a bitmap showing a good identifyable icon of this, for use in
		/// GUI lists etc.
		/// @return A good identifyable graphical representation of this in a BITMAP, if
		/// available. If not, 0 is returned. Ownership is NOT TRANSFERRED!
		BITMAP* GetGraphicalIcon() const override { return m_pPresentationBitmap; };

		/// Gets the name of an assembly symmetric to this one.
		/// @return Symmetric assembly name.
		std::string GetSymmetricAssemblyName() const { return m_SymmetricAssembly; };

		/// Sets the name of an assembly symmetric to this one.
		/// @param newSymmetricAssembly Symmetric assembly name.
		void SetSymmetricAssemblyName(std::string newSymmetricAssembly) { m_SymmetricAssembly = newSymmetricAssembly; };

		/// Draws this TerrainObject's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// like indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;
		// SceneObject:s to be placed in the scene, OWNED HERE
		std::list<SceneObject*> m_PlacedObjects;
		// Parent bunker assembly scheme
		std::string m_ParentAssemblyScheme;
		// Group proveded by parent scheme to which this assembly was added
		std::string m_ParentSchemeGroup;
		// Bitmap shown during draw and icon creation
		BITMAP* m_pPresentationBitmap;
		// Assembly symmetric to this one
		std::string m_SymmetricAssembly;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this BunkerAssembly, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		BunkerAssembly(const BunkerAssembly& reference) = delete;
		void operator=(const BunkerAssembly& rhs) = delete;
	};

} // namespace RTE
