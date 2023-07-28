// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

#include "Entity.h"
#include "Actor.h"
#include "Material.h"
#include "Scene.h"
#include "SceneObject.h"
#include "SLBackground.h"
#include "SLTerrain.h"
#include "TerrainObject.h"
#include "Deployment.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Deployment) {
		auto luaType = AbstractTypeLuaClassDefinition(Deployment, SceneObject, Entity);

		luaType["ID"] = sol::property(&Deployment::GetID);
		luaType["HFlipped"] = sol::property(&Deployment::IsHFlipped);
		luaType["SpawnRadius"] = sol::property(&Deployment::GetSpawnRadius);

		luaType["GetLoadoutName"] = &Deployment::GetLoadoutName;
		luaType["CreateDeployedActor"] = (Actor * (Deployment::*)())&Deployment::CreateDeployedActor; //, luabind::adopt(luabind::result);
		luaType["CreateDeployedObject"] = (SceneObject * (Deployment::*)())&Deployment::CreateDeployedObject; //, luabind::adopt(luabind::result);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Material) {
		auto luaType = SimpleTypeLuaClassDefinition(Material);

		luaType["ID"] = sol::property(&Material::GetIndex);
		luaType["Restitution"] = sol::property(&Material::GetRestitution);
		luaType["Bounce"] = sol::property(&Material::GetRestitution);
		luaType["Friction"] = sol::property(&Material::GetFriction);
		luaType["Stickiness"] = sol::property(&Material::GetStickiness);
		luaType["Strength"] = sol::property(&Material::GetIntegrity);
		luaType["StructuralIntegrity"] = sol::property(&Material::GetIntegrity);
		luaType["DensityKGPerVolumeL"] = sol::property(&Material::GetVolumeDensity);
		luaType["DensityKGPerPixel"] = sol::property(&Material::GetPixelDensity);
		luaType["SettleMaterial"] = sol::property(&Material::GetSettleMaterial);
		luaType["SpawnMaterial"] = sol::property(&Material::GetSpawnMaterial);
		luaType["TransformsInto"] = sol::property(&Material::GetSpawnMaterial);
		luaType["IsScrap"] = sol::property(&Material::IsScrap);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Scene) {
		auto luaType = ConcreteTypeLuaClassDefinition(Scene, Entity);

		luaType["Location"] = sol::property(&Scene::GetLocation, &Scene::SetLocation);
		//luaType["Terrain"] = sol::property(&Scene::GetTerrain);
		luaType["Dimensions"] = sol::property(&Scene::GetDimensions);
		luaType["Width"] = sol::property(&Scene::GetWidth);
		luaType["Height"] = sol::property(&Scene::GetHeight);
		luaType["WrapsX"] = sol::property(&Scene::WrapsX);
		luaType["WrapsY"] = sol::property(&Scene::WrapsY);
		luaType["TeamOwnership"] = sol::property(&Scene::GetTeamOwnership, &Scene::SetTeamOwnership);
		luaType["GlobalAcc"] = sol::property(&Scene::GetGlobalAcc, &Scene::SetGlobalAcc);
		luaType["ScenePathSize"] = sol::property(&Scene::GetScenePathSize);

		luaType["Deployments"] = &Scene::m_Deployments;

		luaType["BackgroundLayers"] = sol::readonly(&Scene::m_BackLayerList);

		luaType["GetScenePath"] = &Scene::GetScenePath;
		luaType["GetBuildBudget"] = &Scene::GetBuildBudget;
		luaType["SetBuildBudget"] = &Scene::SetBuildBudget;
		luaType["IsScanScheduled"] = &Scene::IsScanScheduled;
		luaType["SetScheduledScan"] = &Scene::SetScheduledScan;
		luaType["ClearPlacedObjectSet"] = &Scene::ClearPlacedObjectSet;
		luaType["PlaceResidentBrain"] = &Scene::PlaceResidentBrain;
		luaType["PlaceResidentBrains"] = &Scene::PlaceResidentBrains;
		luaType["RetrieveResidentBrains"] = &Scene::RetrieveResidentBrains;
		luaType["GetResidentBrain"] = &Scene::GetResidentBrain;
		luaType["SetResidentBrain"] = &Scene::SetResidentBrain;
		luaType["Areas"] = &Scene::m_AreaList;
		luaType["SetArea"] = &Scene::SetArea;
		luaType["HasArea"] = &Scene::HasArea;
		luaType["GetArea"] = (Scene::Area * (Scene:: *)(const std::string &areaName)) &Scene::GetArea;
		luaType["GetOptionalArea"] = &Scene::GetOptionalArea;
		luaType["WithinArea"] = &Scene::WithinArea;
		luaType["ResetPathFinding"] = &Scene::ResetPathFinding;
		luaType["UpdatePathFinding"] = &Scene::UpdatePathFinding;
		luaType["PathFindingUpdated"] = &Scene::PathFindingUpdated;
		luaType["CalculatePath"] = &LuaAdaptersScene::CalculatePath1;
		luaType["CalculatePath"] = &LuaAdaptersScene::CalculatePath2;
		luaType["CalculatePathAsync"] = &LuaAdaptersScene::CalculatePathAsync1;
		luaType["CalculatePathAsync"] = &LuaAdaptersScene::CalculatePathAsync2;

		{
			sol::table enumTable = LegacyEnumTypeTable("PlacedObjectSets");
			enumTable["PLACEONLOAD"] = Scene::PlacedObjectSets::PLACEONLOAD;
			enumTable["BLUEPRINT"] = Scene::PlacedObjectSets::BLUEPRINT;
			enumTable["AIPLAN"] = Scene::PlacedObjectSets::AIPLAN;
			enumTable["PLACEDSETSCOUNT"] = Scene::PlacedObjectSets::PLACEDSETSCOUNT;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneArea) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(Scene::Area, "Area");

		luaType.set(sol::call_constructor, sol::constructors<
			Scene::Area(),
			Scene::Area(std::string),
			Scene::Area(const Scene::Area&)
		>());

		luaType["ClassName"] = sol::property(&Scene::Area::GetClassName);
		luaType["Name"] = sol::property(&Scene::Area::GetName);
		luaType["FirstBox"] = sol::property(&Scene::Area::GetFirstBox);
		luaType["Center"] = sol::property(&Scene::Area::GetCenterPoint);
		luaType["RandomPoint"] = sol::property(&Scene::Area::GetRandomPoint);

		luaType["Reset"] = &Scene::Area::Reset;
		luaType["Boxes"] = &Scene::Area::m_BoxList;
		luaType["AddBox"] = &Scene::Area::AddBox;
		luaType["RemoveBox"] = &Scene::Area::RemoveBox;
		luaType["HasNoArea"] = &Scene::Area::HasNoArea;
		luaType["IsInside"] = &Scene::Area::IsInside;
		luaType["IsInsideX"] = &Scene::Area::IsInsideX;
		luaType["IsInsideY"] = &Scene::Area::IsInsideY;
		luaType["GetBoxInside"] = &Scene::Area::GetBoxInside;
		luaType["RemoveBoxInside"] = &Scene::Area::RemoveBoxInside;
		luaType["GetCenterPoint"] = &Scene::Area::GetCenterPoint;
		luaType["GetRandomPoint"] = &Scene::Area::GetRandomPoint;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneLayer) {
		auto luaType = SimpleTypeLuaClassDefinition(SceneLayer);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneObject) {
		auto luaType = AbstractTypeLuaClassDefinition(SceneObject, Entity);

		luaType["Pos"] = sol::property(&SceneObject::GetPos, &SceneObject::SetPos);
		luaType["HFlipped"] = sol::property(&SceneObject::IsHFlipped, &SceneObject::SetHFlipped);
		luaType["RotAngle"] = sol::property(&SceneObject::GetRotAngle, &SceneObject::SetRotAngle);
		luaType["Team"] = sol::property(&SceneObject::GetTeam, &SceneObject::SetTeam);
		luaType["PlacedByPlayer"] = sol::property(&SceneObject::GetPlacedByPlayer, &SceneObject::SetPlacedByPlayer);
		luaType["IsBuyable"] = sol::property(&SceneObject::IsBuyable);

		luaType["IsOnScenePoint"] = &SceneObject::IsOnScenePoint;
		luaType["GetGoldValue"] = &SceneObject::GetGoldValueOld;
		luaType["GetGoldValue"] = &SceneObject::GetGoldValue;
		luaType["SetGoldValue"] = &SceneObject::SetGoldValue;
		luaType["GetGoldValueString"] = &SceneObject::GetGoldValueString;
		luaType["GetTotalValue"] = &SceneObject::GetTotalValue;

		luaType["GetTotalValue"] = &LuaAdaptersSceneObject::GetTotalValue;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SLBackground) {
		auto luaType = SimpleTypeLuaClassDefinition(SLBackground);

		luaType["Frame"] = sol::property(&SLBackground::GetFrame, &SLBackground::SetFrame);
		luaType["SpriteAnimMode"] = sol::property(&SLBackground::GetSpriteAnimMode, &SLBackground::SetSpriteAnimMode);
		luaType["SpriteAnimDuration"] = sol::property(&SLBackground::GetSpriteAnimDuration, &SLBackground::SetSpriteAnimDuration);
		luaType["IsAnimatedManually"] = sol::property(&SLBackground::IsAnimatedManually, &SLBackground::SetAnimatedManually);
		luaType["AutoScrollX"] = sol::property(&SLBackground::GetAutoScrollX, &SLBackground::SetAutoScrollX);
		luaType["AutoScrollY"] = sol::property(&SLBackground::GetAutoScrollY, &SLBackground::SetAutoScrollY);
		luaType["AutoScrollInterval"] = sol::property(&SLBackground::GetAutoScrollStepInterval, &SLBackground::SetAutoScrollStepInterval);
		luaType["AutoScrollStep"] = sol::property(&SLBackground::GetAutoScrollStep, &SLBackground::SetAutoScrollStep);
		luaType["AutoScrollStepX"] = sol::property(&SLBackground::GetAutoScrollStepX, &SLBackground::SetAutoScrollStepX);
		luaType["AutoScrollStepY"] = sol::property(&SLBackground::GetAutoScrollStepY, &SLBackground::SetAutoScrollStepY);

		luaType["IsAutoScrolling"] = &SLBackground::IsAutoScrolling;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TerrainObject) {
		auto luaType = ConcreteTypeLuaClassDefinition(TerrainObject, SceneObject, Entity);

		luaType["GetBitmapOffset"] = &TerrainObject::GetBitmapOffset;
		luaType["GetBitmapWidth"] = &TerrainObject::GetBitmapWidth;
		luaType["GetBitmapHeight"] = &TerrainObject::GetBitmapHeight;
	}
}