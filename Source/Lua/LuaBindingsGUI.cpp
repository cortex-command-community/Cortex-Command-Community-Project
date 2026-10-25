// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

using namespace RTE;

LuaBindingRegisterFunctionDefinitionForType(GUILuaBindings, GUIBanner) {
	registry.Class<GUIBanner>("GUIBanner")

	    .addProperty("BannerText", &GUIBanner::GetBannerText)
	    .addProperty("AnimState", &GUIBanner::GetAnimState)
	    .addProperty("Kerning", &GUIBanner::GetKerning, &GUIBanner::SetKerning)

	    .addFunction("IsVisible", &GUIBanner::IsVisible)
	    .addFunction("ShowText", &GUIBanner::ShowText)
	    .addFunction("HideText", &GUIBanner::HideText)
	    .addFunction("ClearText", &GUIBanner::ClearText)

	    .addEnum({{"BLINKING", GUIBanner::AnimMode::BLINKING},
	              {"FLYBYLEFTWARD", GUIBanner::AnimMode::FLYBYLEFTWARD},
	              {"FLYBYRIGHTWARD", GUIBanner::AnimMode::FLYBYRIGHTWARD},
	              {"ANIMMODECOUNT", GUIBanner::AnimMode::ANIMMODECOUNT}})
	    .addEnum({{"NOTSTARTED", GUIBanner::AnimState::NOTSTARTED},
	              {"SHOWING", GUIBanner::AnimState::SHOWING},
	              {"SHOW", GUIBanner::AnimState::SHOW},
	              {"HIDING", GUIBanner::AnimState::HIDING},
	              {"OVER", GUIBanner::AnimState::OVER},
	              {"ANIMSTATECOUNT", GUIBanner::AnimState::ANIMSTATECOUNT}})
	    .addEnum({{"RED", GameActivity::BannerColor::RED},
	              {"YELLOW", GameActivity::BannerColor::YELLOW}});
}

LuaBindingRegisterFunctionDefinitionForType(GUILuaBindings, BuyMenuGUI) {
	registry.Class<BuyMenuGUI>("BuyMenuGUI")

	    .addProperty("ShowOnlyOwnedItems", &BuyMenuGUI::GetOnlyShowOwnedItems, &BuyMenuGUI::SetOnlyShowOwnedItems)
	    .addProperty("EnforceMaxPassengersConstraint", &BuyMenuGUI::EnforceMaxPassengersConstraint, &BuyMenuGUI::SetEnforceMaxPassengersConstraint)
	    .addProperty("EnforceMaxMassConstraint", &BuyMenuGUI::EnforceMaxMassConstraint, &BuyMenuGUI::SetEnforceMaxMassConstraint)

	    .addFunction("SetMetaPlayer", &BuyMenuGUI::SetMetaPlayer)
	    .addFunction("SetNativeTechModule", &BuyMenuGUI::SetNativeTechModule)
	    .addFunction("SetForeignCostMultiplier", &BuyMenuGUI::SetForeignCostMultiplier)
	    .addFunction("SetModuleExpanded", &BuyMenuGUI::SetModuleExpanded)
	    .addFunction("LoadAllLoadoutsFromFile", &BuyMenuGUI::LoadAllLoadoutsFromFile)
	    .addFunction("AddAllowedItem", &BuyMenuGUI::AddAllowedItem)
	    .addFunction("RemoveAllowedItem", &BuyMenuGUI::RemoveAllowedItem)
	    .addFunction("ClearAllowedItems", &BuyMenuGUI::ClearAllowedItems)
	    .addFunction("AddAlwaysAllowedItem", &BuyMenuGUI::AddAlwaysAllowedItem)
	    .addFunction("RemoveAlwaysAllowedItem", &BuyMenuGUI::RemoveAlwaysAllowedItem)
	    .addFunction("ClearAlwaysAllowedItems", &BuyMenuGUI::ClearAlwaysAllowedItems)
	    .addFunction("AddProhibitedItem", &BuyMenuGUI::AddProhibitedItem)
	    .addFunction("RemoveProhibitedItem", &BuyMenuGUI::RemoveProhibitedItem)
	    .addFunction("ClearProhibitedItems", &BuyMenuGUI::ClearProhibitedItems)
	    .addFunction("ForceRefresh", &BuyMenuGUI::ForceRefresh)
	    .addFunction("SetOwnedItemsAmount", &BuyMenuGUI::SetOwnedItemsAmount)
	    .addFunction("GetOwnedItemsAmount", &BuyMenuGUI::GetOwnedItemsAmount)
	    .addFunction("SetBannerImage", &BuyMenuGUI::SetBannerImage)
	    .addFunction("SetLogoImage", &BuyMenuGUI::SetLogoImage)
	    .addFunction("ClearCartList", &BuyMenuGUI::ClearCartList)
	    .addFunction("LoadDefaultLoadoutToCart", &BuyMenuGUI::LoadDefaultLoadoutToCart)
	    .addFunction("GetOrderList", &LuaAdaptersBuyMenuGUI::GetOrderList, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetTotalCartCost", &BuyMenuGUI::GetTotalCartCost)
	    .addFunction("GetTotalOrderCost", &BuyMenuGUI::GetTotalOrderCost)
	    .addFunction("GetTotalOrderMass", &BuyMenuGUI::GetTotalOrderMass)
	    .addFunction("GetTotalOrderPassengers", &BuyMenuGUI::GetTotalOrderPassengers);
}

LuaBindingRegisterFunctionDefinitionForType(GUILuaBindings, SceneEditorGUI) {
	registry.Class<SceneEditorGUI>("SceneEditorGUI")

	    .addProperty("EditorMode", &SceneEditorGUI::GetEditorGUIMode, &SceneEditorGUI::SetEditorGUIMode)

	    .addFunction("SetCursorPos", &SceneEditorGUI::SetCursorPos)
	    .addFunction("GetCurrentObject", &SceneEditorGUI::GetCurrentObject)
	    .addFunction("SetCurrentObject", &SceneEditorGUI::SetCurrentObject, LuaBinding::AdoptArg<2>)
	    .addFunction("SetModuleSpace", &SceneEditorGUI::SetModuleSpace)
	    .addFunction("SetNativeTechModule", &SceneEditorGUI::SetNativeTechModule)
	    .addFunction("SetForeignCostMultiplier", &SceneEditorGUI::SetForeignCostMultiplier)
	    .addFunction("TestBrainResidence", &SceneEditorGUI::TestBrainResidence)
	    .addFunction("Update", &SceneEditorGUI::Update) // Gacyr Note: I hate this being here but it's necessary for some metagame bullshit.

	    .addEnum({{"INACTIVE", SceneEditorGUI::EditorGUIMode::INACTIVE},
	              {"PICKINGOBJECT", SceneEditorGUI::EditorGUIMode::PICKINGOBJECT},
	              {"ADDINGOBJECT", SceneEditorGUI::EditorGUIMode::ADDINGOBJECT},
	              {"INSTALLINGBRAIN", SceneEditorGUI::EditorGUIMode::INSTALLINGBRAIN},
	              {"PLACINGOBJECT", SceneEditorGUI::EditorGUIMode::PLACINGOBJECT},
	              {"MOVINGOBJECT", SceneEditorGUI::EditorGUIMode::MOVINGOBJECT},
	              {"DELETINGOBJECT", SceneEditorGUI::EditorGUIMode::DELETINGOBJECT},
	              {"PLACEINFRONT", SceneEditorGUI::EditorGUIMode::PLACEINFRONT},
	              {"PLACEBEHIND", SceneEditorGUI::EditorGUIMode::PLACEBEHIND},
	              {"DONEEDITING", SceneEditorGUI::EditorGUIMode::DONEEDITING},
	              {"EDITORGUIMODECOUNT", SceneEditorGUI::EditorGUIMode::EDITORGUIMODECOUNT}});
}
