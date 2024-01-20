// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "GUIBanner.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"
#include "GameActivity.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(GUILuaBindings, GUIBanner) {
		auto luaType = SimpleTypeLuaClassDefinition(GUIBanner);

		luaType["BannerText"] = sol::property(&GUIBanner::GetBannerText);
		luaType["AnimState"] = sol::property(&GUIBanner::GetAnimState);
		luaType["Kerning"] = sol::property(&GUIBanner::GetKerning, &GUIBanner::SetKerning);

		luaType["IsVisible"] = &GUIBanner::IsVisible;
		luaType["ShowText"] = &GUIBanner::ShowText;
		luaType["HideText"] = &GUIBanner::HideText;
		luaType["ClearText"] = &GUIBanner::ClearText;

		{
			sol::table enumTable = LegacyEnumTypeTable("AnimMode");
			enumTable["BLINKING"] = GUIBanner::AnimMode::BLINKING;
			enumTable["FLYBYLEFTWARD"] = GUIBanner::AnimMode::FLYBYLEFTWARD;
			enumTable["FLYBYRIGHTWARD"] = GUIBanner::AnimMode::FLYBYRIGHTWARD;
			enumTable["ANIMMODECOUNT"] = GUIBanner::AnimMode::ANIMMODECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("AnimState");
			enumTable["NOTSTARTED"] = GUIBanner::AnimState::NOTSTARTED;
			enumTable["SHOWING"] = GUIBanner::AnimState::SHOWING;
			enumTable["SHOW"] = GUIBanner::AnimState::SHOW;
			enumTable["HIDING"] = GUIBanner::AnimState::HIDING;
			enumTable["OVER"] = GUIBanner::AnimState::OVER;
			enumTable["ANIMSTATECOUNT"] = GUIBanner::AnimState::ANIMSTATECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("BannerColor");
			enumTable["RED"] = GameActivity::BannerColor::RED;
			enumTable["YELLOW"] = GameActivity::BannerColor::YELLOW;
		}
	}

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
		luaType["GetOrderList"] = &BuyMenuGUI::GetOrderList; // luabind::adopt(luabind::return_value) + luabind::return_stl_iterator
		luaType["GetTotalCartCost"] = &BuyMenuGUI::GetTotalCartCost;
		luaType["GetTotalOrderCost"] = &BuyMenuGUI::GetTotalOrderCost;
		luaType["GetTotalOrderMass"] = &BuyMenuGUI::GetTotalOrderMass;
		luaType["GetTotalOrderPassengers"] = &BuyMenuGUI::GetTotalOrderPassengers;
	}

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
		luaType["Update"] = &SceneEditorGUI::Update; // Gacyr Note: I hate this being here but it's necessary for some metagame bullshit.

		{
			sol::table enumTable = LegacyEnumTypeTable("EditorGUIMode");
			enumTable["INACTIVE"] = SceneEditorGUI::EditorGUIMode::INACTIVE;
			enumTable["PICKINGOBJECT"] = SceneEditorGUI::EditorGUIMode::PICKINGOBJECT;
			enumTable["ADDINGOBJECT"] = SceneEditorGUI::EditorGUIMode::ADDINGOBJECT;
			enumTable["INSTALLINGBRAIN"] = SceneEditorGUI::EditorGUIMode::INSTALLINGBRAIN;
			enumTable["PLACINGOBJECT"] = SceneEditorGUI::EditorGUIMode::PLACINGOBJECT;
			enumTable["MOVINGOBJECT"] = SceneEditorGUI::EditorGUIMode::MOVINGOBJECT;
			enumTable["DELETINGOBJECT"] = SceneEditorGUI::EditorGUIMode::DELETINGOBJECT;
			enumTable["PLACEINFRONT"] = SceneEditorGUI::EditorGUIMode::PLACEINFRONT;
			enumTable["PLACEBEHIND"] = SceneEditorGUI::EditorGUIMode::PLACEBEHIND;
			enumTable["DONEEDITING"] = SceneEditorGUI::EditorGUIMode::DONEEDITING;
			enumTable["EDITORGUIMODECOUNT"] = SceneEditorGUI::EditorGUIMode::EDITORGUIMODECOUNT;
		}
	}
} // namespace RTE
