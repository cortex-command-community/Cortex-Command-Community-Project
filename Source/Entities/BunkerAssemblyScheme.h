#pragma once

/// Header file for the BunkerAssemblyScheme class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "SceneObject.h"
#include "SceneMan.h"

#define ICON_WIDTH 69
#define AREA_PER_DEPLOYMENT 64

namespace RTE {

	class ContentFile;

	/// A feature of the terrain, which includes foreground color layer,
	/// material layer and optional background layer.
	class BunkerAssemblyScheme : public SceneObject {

		/// Public member variable, method and friend function declarations
	public:
		// Different scheme properties are encoded on colors of scheme bitmap
		enum SchemeColor {
			SCHEME_COLOR_EMPTY = g_MaskColor, // Empty sections, MUST BE ALWAYS EMPTY
			SCHEME_COLOR_PASSABLE = 5, // Passable sections, MUST BE ALWAYS PASSBLE, I.E. HAVE ONLY BACKGROUNDS
			SCHEME_COLOR_VARIABLE = 4, // May be passable or not. Expect air.
			SCHEME_COLOR_WALL = 3 // Always impassable, but may be empty. Expect terrain.
		};

		// Scheme properties, when drawed in game UIs
		enum PresentationColor {
			PAINT_COLOR_PASSABLE = 5,
			PAINT_COLOR_VARIABLE = 48,
			PAINT_COLOR_WALL = 13
		};

		const static int ScaleX = 24;
		const static int ScaleY = 24;
		const static int SchemeWidth = 2;

		// Concrete allocation and cloning definitions
		EntityAllocation(BunkerAssemblyScheme);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a BunkerAssemblyScheme object in system
		/// memory. Create() should be called before using the object.
		BunkerAssemblyScheme();

		/// Destructor method used to clean up a BunkerAssemblyScheme object before deletion
		/// from system memory.
		~BunkerAssemblyScheme() override;

		/// Makes the BunkerAssemblyScheme object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a BunkerAssemblyScheme to be identical to another, by deep copy.
		/// @param reference A reference to the BunkerAssemblyScheme to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const BunkerAssemblyScheme& reference);

		/// Resets the entire BunkerAssemblyScheme, including its inherited members, to
		/// their default settings or values.
		void Reset() override {
			Clear();
			SceneObject::Reset();
		}

		/// Destroys and resets (through Clear()) the BunkerAssemblyScheme object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the width this' bitmap.
		/// @return Width of bitmap.
		const int GetBitmapWidth() const { return m_pPresentationBitmap->w; }

		/// Gets the height this' material bitmap.
		/// @return Height of 'material' bitmap.
		const int GetBitmapHeight() const { return m_pPresentationBitmap->h; }

		/// Gets the offset of presentation bitmap
		/// @return Offset of bitmap
		const Vector GetBitmapOffset() const { return m_BitmapOffset; }

		/// Gets the BITMAP object that this BunkerAssemblyScheme uses for its fore-
		/// ground color representation.
		/// @return A pointer to the foreground color BITMAP object. Ownership is not
		/// transferred.
		BITMAP* GetBitmap() const { return m_pPresentationBitmap; }

		/// Gets the area of the structure bitmap.
		int GetArea() const { return m_pBitmap->h * m_pBitmap->w; }

		/// Gets a bitmap showing a good identifyable icon of this, for use in
		/// GUI lists etc.
		/// @return A good identifyable graphical representation of this in a BITMAP, if
		/// available. If not, 0 is returned. Ownership is NOT TRANSFERRED!
		BITMAP* GetGraphicalIcon() const override;

		/// Sets which team this belongs to.
		/// @param team The assigned team number.
		void SetTeam(int team) override;

		/// Indicates whether this' current graphical representation overlaps
		/// a point in absolute scene coordinates.
		/// @param scenePoint The point in absolute scene coordinates.
		/// @return Whether this' graphical rep overlaps the scene point.
		bool IsOnScenePoint(Vector& scenePoint) const override;

		/// Draws this BunkerAssemblyScheme's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// like indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Returns whether sceneman should select just a single assembly for this scheme
		/// and use it everywhere on the scene.
		/// @return Whether we allowed to use just one type of assembly for this scheme
		bool IsOneTypePerScene() { return m_IsOneTypePerScene; }

		/// Gets the name of the scheme symmetric to this one.
		/// @return Symmetric scheme name.
		std::string GetSymmetricSchemeName() const { return m_SymmetricScheme; }

		/// Gets the name of group to which assemblies linked with this scheme must be added.
		/// @return Assembly group name.
		std::string GetAssemblyGroup() const { return m_AssemblyGroup; }

		/// Returns the limit of these schemes per scene. 0 - no limit.
		/// @return Scheme limit.
		int GetLimit() { return m_Limit; }

		/// Returns the number of deployments this scheme is allowed to place.
		/// @return Deployments limit.
		int GetMaxDeployments() const { return m_MaxDeployments; }

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;

		ContentFile m_BitmapFile;

		// Not owned by this
		BITMAP* m_pBitmap;
		BITMAP* m_pPresentationBitmap;
		BITMAP* m_pIconBitmap;

		// The objects that are placed along with this in the scene
		std::list<SOPlacer> m_ChildObjects;

		// If this is true then sceneman must select a single assembly for this scheme and use it everywhere on the scene
		bool m_IsOneTypePerScene;
		// How many assemblies can placed on one scene?
		int m_Limit;
		// Drawable bitmap offset of this bunker assembly
		Vector m_BitmapOffset;
		// How many deployments should be selected during placement
		int m_MaxDeployments;
		// Scheme symmetric to this one
		std::string m_SymmetricScheme;
		// To which group we should add assemblies linked to this scheme
		std::string m_AssemblyGroup;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this BunkerAssemblyScheme, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		BunkerAssemblyScheme(const BunkerAssemblyScheme& reference) = delete;
		void operator=(const BunkerAssemblyScheme& rhs) = delete;
	};

} // namespace RTE
