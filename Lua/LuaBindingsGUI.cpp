// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "GUIBanner.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"
#include "GameActivity.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(GUILuaBindings, GUIBanner) {
		auto luaType = SimpleTypeLuaClassDefinition(GUIBanner);

		luaType["BannerText"] = sol::property(&GUIBanner::GetBannerText);
		luaType["AnimState"] = sol::property(&GUIBanner::GetAnimState);
		luaType["Kerning"] = sol::property(&GUIBanner::GetKerning, &GUIBanner::SetKerning);

		luaType["IsVisible"] = &GUIBanner::IsVisible;
		luaType["ShowText"] = &GUIBanner::ShowText;
		luaType["HideText"] = &GUIBanner::HideText;
		luaType["ClearText"] = &GUIBanner::ClearText;

		luaType.new_enum<GUIBanner::AnimMode>("AnimMode", {
			{ "BLINKING", GUIBanner::AnimMode::BLINKING },
			{ "FLYBYLEFTWARD", GUIBanner::AnimMode::FLYBYLEFTWARD },
			{ "FLYBYRIGHTWARD", GUIBanner::AnimMode::FLYBYRIGHTWARD },
			{ "ANIMMODECOUNT", GUIBanner::AnimMode::ANIMMODECOUNT }
		});
		luaType.new_enum<GUIBanner::AnimState>("AnimState", {
			{ "NOTSTARTED", GUIBanner::AnimState::NOTSTARTED },
			{ "SHOWING", GUIBanner::AnimState::SHOWING },
			{ "SHOW", GUIBanner::AnimState::SHOW },
			{ "HIDING", GUIBanner::AnimState::HIDING },
			{ "OVER", GUIBanner::AnimState::OVER },
			{ "ANIMSTATECOUNT", GUIBanner::AnimState::ANIMSTATECOUNT }
		});
		luaType.new_enum<GameActivity::BannerColor>("BannerColor", {
			{ "RED", GameActivity::BannerColor::RED },
			{ "YELLOW", GameActivity::BannerColor::YELLOW }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(GUILuaBindings, BuyMenuGUI) {
		auto luaType = SimpleTypeLuaClassDefinition(BuyMenuGUI);

		luaType["ShowOnlyOwnedItems"] = sol::property(&BuyMenuGUI::GetOnlyShowOwnedItems, &BuyMenuGUI::SetOnlyShowOwnedItems);
		luaType["EnforceMaxPassengersConstraint"] = sol::property(&BuyMenuGUI::EnforceMaxPassengersConstraint, &BuyMenuGUI::SetEnforceMaxPassengersConstraint);
		luaType["EnforceMaxMassConstraint"] = sol::property(&BuyMenuGUI::EnforceMaxMassConstraint, &BuyMenuGUI::SetEnforceMaxMassConstraint);

		luaType["SetMetaPlayer"] = &BuyMenuGUI::SetMetaPlayer;
		luaType["SetNativeTechModule"] = &BuyMenuGUI::SetNativeTechModule;
		luaType["SetForeignCostMultiplier"] = &BuyMenuGUI::SetForeignCostMultiplier;
		luaType["SetModuleExpanded"] = &BuyMenuGUI::SetModuleExpanded;
		luaType["LoadAllLoadoutsFromFile"] = &BuyMenuGUI::LoadAllLoadoutsFromFile;
		luaType["AddAllowedItem"] = &BuyMenuGUI::AddAllowedItem;
		luaType["RemoveAllowedItem"] = &BuyMenuGUI::RemoveAllowedItem;
		luaType["ClearAllowedItems"] = &BuyMenuGUI::ClearAllowedItems;
		luaType["AddAlwaysAllowedItem"] = &BuyMenuGUI::AddAlwaysAllowedItem;
		luaType["RemoveAlwaysAllowedItem"] = &BuyMenuGUI::RemoveAlwaysAllowedItem;
		luaType["ClearAlwaysAllowedItems"] = &BuyMenuGUI::ClearAlwaysAllowedItems;
		luaType["AddProhibitedItem"] = &BuyMenuGUI::AddProhibitedItem;
		luaType["RemoveProhibitedItem"] = &BuyMenuGUI::RemoveProhibitedItem;
		luaType["ClearProhibitedItems"] = &BuyMenuGUI::ClearProhibitedItems;
		luaType["ForceRefresh"] = &BuyMenuGUI::ForceRefresh;
		luaType["SetOwnedItemsAmount"] = &BuyMenuGUI::SetOwnedItemsAmount;
		luaType["GetOwnedItemsAmount"] = &BuyMenuGUI::GetOwnedItemsAmount;
		luaType["SetBannerImage"] = &BuyMenuGUI::SetBannerImage;
		luaType["SetLogoImage"] = &BuyMenuGUI::SetLogoImage;
		luaType["ClearCartList"] = &BuyMenuGUI::ClearCartList;
		luaType["LoadDefaultLoadoutToCart"] = &BuyMenuGUI::LoadDefaultLoadoutToCart;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(GUILuaBindings, SceneEditorGUI) {
		auto luaType = SimpleTypeLuaClassDefinition(SceneEditorGUI);

		luaType["EditorMode"] = sol::property(&SceneEditorGUI::GetEditorGUIMode, &SceneEditorGUI::SetEditorGUIMode);

		luaType["SetCursorPos"] = &SceneEditorGUI::SetCursorPos;
		luaType["GetCurrentObject"] = &SceneEditorGUI::GetCurrentObject;
		luaType["SetCurrentObject"] = &SceneEditorGUI::SetCurrentObject;
		luaType["SetModuleSpace"] = &SceneEditorGUI::SetModuleSpace;
		luaType["SetNativeTechModule"] = &SceneEditorGUI::SetNativeTechModule;
		luaType["SetForeignCostMultiplier"] = &SceneEditorGUI::SetForeignCostMultiplier;
		luaType["TestBrainResidence"] = &SceneEditorGUI::TestBrainResidence;
		luaType["Update"] = &SceneEditorGUI::Update; //Gacyr Note: I hate this being here but it's necessary for some metagame bullshit.

		luaType.new_enum<SceneEditorGUI::EditorGUIMode>("EditorGUIMode", {
			{ "INACTIVE", SceneEditorGUI::EditorGUIMode::INACTIVE },
			{ "PICKINGOBJECT", SceneEditorGUI::EditorGUIMode::PICKINGOBJECT },
			{ "ADDINGOBJECT", SceneEditorGUI::EditorGUIMode::ADDINGOBJECT },
			{ "INSTALLINGBRAIN", SceneEditorGUI::EditorGUIMode::INSTALLINGBRAIN },
			{ "PLACINGOBJECT", SceneEditorGUI::EditorGUIMode::PLACINGOBJECT },
			{ "MOVINGOBJECT", SceneEditorGUI::EditorGUIMode::MOVINGOBJECT },
			{ "DELETINGOBJECT", SceneEditorGUI::EditorGUIMode::DELETINGOBJECT },
			{ "PLACEINFRONT", SceneEditorGUI::EditorGUIMode::PLACEINFRONT },
			{ "PLACEBEHIND", SceneEditorGUI::EditorGUIMode::PLACEBEHIND },
			{ "DONEEDITING", SceneEditorGUI::EditorGUIMode::DONEEDITING },
			{ "EDITORGUIMODECOUNT", SceneEditorGUI::EditorGUIMode::EDITORGUIMODECOUNT }
		});
	}
}