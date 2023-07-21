// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

#include "MovableMan.h"
#include "MetaPlayer.h"
#include "PieMenu.h"
#include "PieSlice.h"
#include "Actor.h"
#include "MovableObject.h"
#include "SoundContainer.h"
#include "GlobalScript.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, AlarmEvent) {
		auto luaType = SimpleTypeLuaClassDefinition(AlarmEvent);

		luaType.set(sol::call_constructor, sol::constructors<
			AlarmEvent(), 
			AlarmEvent(const Vector&, int, float)
		>());

		luaType["ScenePos"] = &AlarmEvent::m_ScenePos;
		luaType["Team"] = &AlarmEvent::m_Team;
		luaType["Range"] = &AlarmEvent::m_Range;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, Directions) {
		EnumTypeLuaClassDefinition(Directions, "Directions", {
			{ "None", Directions::None },
			{ "Up", Directions::Up },
			{ "Down", Directions::Down },
			{ "Left", Directions::Left },
			{ "Right", Directions::Right },
			{ "Any", Directions::Any }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, DrawBlendMode) {
		EnumTypeLuaClassDefinition(DrawBlendMode, "DrawBlendMode", {
			{ "NoBlend", DrawBlendMode::NoBlend },
			{ "Burn", DrawBlendMode::BlendBurn },
			{ "Color", DrawBlendMode::BlendColor },
			{ "Difference", DrawBlendMode::BlendDifference },
			{ "Dissolve", DrawBlendMode::BlendDissolve },
			{ "Dodge", DrawBlendMode::BlendDodge },
			{ "Invert", DrawBlendMode::BlendInvert },
			{ "Luminance", DrawBlendMode::BlendLuminance },
			{ "Multiply", DrawBlendMode::BlendMultiply },
			{ "Saturation", DrawBlendMode::BlendSaturation },
			{ "Screen", DrawBlendMode::BlendScreen },
			{ "Transparency", DrawBlendMode::BlendTransparency },
			{ "BlendModeCount", DrawBlendMode::BlendModeCount }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MetaPlayer) {
		auto luaType = SimpleTypeLuaClassDefinition(MetaPlayer);

		luaType.set(sol::call_constructor, sol::constructors<
			MetaPlayer()
		>());

		luaType["NativeTechModule"] = sol::property(&MetaPlayer::GetNativeTechModule);
		luaType["ForeignCostMultiplier"] = sol::property(&MetaPlayer::GetForeignCostMultiplier);
		luaType["NativeCostMultiplier"] = sol::property(&MetaPlayer::GetNativeCostMultiplier);
		luaType["InGamePlayer"] = sol::property(&MetaPlayer::GetInGamePlayer);
		luaType["BrainPoolCount"] = sol::property(&MetaPlayer::GetBrainPoolCount, &MetaPlayer::SetBrainPoolCount);

		luaType["ChangeBrainPoolCount"] = &MetaPlayer::ChangeBrainPoolCount;
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieSlice) {
		auto luaType = ConcreteTypeLuaClassDefinition(PieSlice, Entity);

		luaType["Type"] = sol::property(&PieSlice::GetType, &PieSlice::SetType);
		luaType["Direction"] = sol::property(&PieSlice::GetDirection, &PieSlice::SetDirection);
		luaType["CanBeMiddleSlice"] = sol::property(&PieSlice::GetCanBeMiddleSlice, &PieSlice::SetCanBeMiddleSlice);
		luaType["OriginalSource"] = sol::property(&PieSlice::GetOriginalSource);
		luaType["Enabled"] = sol::property(&PieSlice::IsEnabled, &PieSlice::SetEnabled);

		luaType["ScriptPath"] = sol::property(&PieSlice::GetScriptPath, &PieSlice::SetScriptPath);
		luaType["FunctionName"] = sol::property(&PieSlice::GetFunctionName, &PieSlice::SetFunctionName);
		luaType["SubPieMenu"] = sol::property(&PieSlice::GetSubPieMenu, &PieSlice::SetSubPieMenu);

		luaType["DrawFlippedToMatchAbsoluteAngle"] = sol::property(&PieSlice::GetDrawFlippedToMatchAbsoluteAngle, &PieSlice::SetDrawFlippedToMatchAbsoluteAngle);

		luaType["ReloadScripts"] = &PieSlice::ReloadScripts;

		{
			sol::table enumTable = LegacyEnumTypeTable("SliceType");
			enumTable["NoType"] = PieSlice::SliceType::NoType;
			enumTable["Pickup"] = PieSlice::SliceType::Pickup;
			enumTable["Drop"] = PieSlice::SliceType::Drop;
			enumTable["NextItem"] = PieSlice::SliceType::NextItem;
			enumTable["PreviousItem"] = PieSlice::SliceType::PreviousItem;
			enumTable["Reload"] = PieSlice::SliceType::Reload;
			enumTable["BuyMenu"] = PieSlice::SliceType::BuyMenu;
			enumTable["Stats"] = PieSlice::SliceType::Stats;
			enumTable["Map"] = PieSlice::SliceType::Map;
			enumTable["FormSquad"] = PieSlice::SliceType::FormSquad;
			enumTable["Ceasefire"] = PieSlice::SliceType::Ceasefire;
			enumTable["Sentry"] = PieSlice::SliceType::Sentry;
			enumTable["Patrol"] = PieSlice::SliceType::Patrol;
			enumTable["BrainHunt"] = PieSlice::SliceType::BrainHunt;
			enumTable["GoldDig"] = PieSlice::SliceType::GoldDig;
			enumTable["GoTo"] = PieSlice::SliceType::GoTo;
			enumTable["Return"] = PieSlice::SliceType::Return;
			enumTable["Stay"] = PieSlice::SliceType::Stay;
			enumTable["Deliver"] = PieSlice::SliceType::Deliver;
			enumTable["Scuttle"] = PieSlice::SliceType::Scuttle;
			enumTable["Done"] = PieSlice::SliceType::EditorDone;
			enumTable["Load"] = PieSlice::SliceType::EditorLoad;
			enumTable["Save"] = PieSlice::SliceType::EditorSave;
			enumTable["New"] = PieSlice::SliceType::EditorNew;
			enumTable["Pick"] = PieSlice::SliceType::EditorPick;
			enumTable["Move"] = PieSlice::SliceType::EditorMove;
			enumTable["Remove"] = PieSlice::SliceType::EditorRemove;
			enumTable["InFront"] = PieSlice::SliceType::EditorInFront;
			enumTable["Behind"] = PieSlice::SliceType::EditorBehind;
			enumTable["ZoomIn"] = PieSlice::SliceType::EditorZoomIn;
			enumTable["ZoomOut"] = PieSlice::SliceType::EditorZoomOut;
			enumTable["Team1"] = PieSlice::SliceType::EditorTeam1;
			enumTable["Team2"] = PieSlice::SliceType::EditorTeam2;
			enumTable["Team3"] = PieSlice::SliceType::EditorTeam3;
			enumTable["Team4"] = PieSlice::SliceType::EditorTeam4;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieMenu) {
		auto luaType = ConcreteTypeLuaClassDefinition(PieMenu, Entity);

		luaType["Owner"] = sol::property(&PieMenu::GetOwner);
		luaType["Controller"] = sol::property(&PieMenu::GetController);
		luaType["AffectedObject"] = sol::property(&PieMenu::GetAffectedObject);
		luaType["Pos"] = sol::property(&PieMenu::GetPos);
		luaType["RotAngle"] = sol::property(&PieMenu::GetRotAngle, &PieMenu::SetRotAngle);
		luaType["FullInnerRadius"] = sol::property(&PieMenu::GetFullInnerRadius, &PieMenu::SetFullInnerRadius);

		luaType["PieSlices"] = sol::property(&PieMenu::GetPieSlices);

		luaType["IsSubPieMenu"] = &PieMenu::IsSubPieMenu;

		luaType["IsEnabled"] = &PieMenu::IsEnabled;
		luaType["IsEnabling"] = &PieMenu::IsEnabling;
		luaType["IsDisabling"] = &PieMenu::IsDisabling;
		luaType["IsEnablingOrDisabling"] = &PieMenu::IsEnablingOrDisabling;
		luaType["IsVisible"] = &PieMenu::IsVisible;
		luaType["HasSubPieMenuOpen"] = &PieMenu::HasSubPieMenuOpen;

		luaType["SetAnimationModeToNormal"] = &PieMenu::SetAnimationModeToNormal;
		luaType["DoDisableAnimation"] = &PieMenu::DoDisableAnimation;
		luaType["Wobble"] = &PieMenu::Wobble;
		luaType["FreezeAtRadius"] = &PieMenu::FreezeAtRadius;

		luaType["GetPieCommand"] = &PieMenu::GetPieCommand;
		luaType["GetFirstPieSliceByPresetName"] = &PieMenu::GetFirstPieSliceByPresetName;
		luaType["GetFirstPieSliceByType"] = &PieMenu::GetFirstPieSliceByType;
		luaType["AddPieSlice"] = &PieMenu::AddPieSlice; //, luabind::adopt(_2);
		luaType["AddPieSlice"] = &LuaAdaptersPieMenu::AddPieSlice; //, luabind::adopt(_2);
		luaType["AddPieSliceIfPresetNameIsUnique"] = &PieMenu::AddPieSliceIfPresetNameIsUnique; //, luabind::adopt(_2);
		luaType["AddPieSliceIfPresetNameIsUnique"] = &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique1; //, luabind::adopt(_2);
		luaType["AddPieSliceIfPresetNameIsUnique"] = &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique2; //, luabind::adopt(_2);
		luaType["RemovePieSlice"] = &PieMenu::RemovePieSlice; //, luabind::adopt(luabind::return_value);
		luaType["RemovePieSlicesByPresetName"] = &PieMenu::RemovePieSlicesByPresetName;
		luaType["RemovePieSlicesByType"] = &PieMenu::RemovePieSlicesByType;
		luaType["RemovePieSlicesByOriginalSource"] = &PieMenu::RemovePieSlicesByOriginalSource;
		luaType["ReplacePieSlice"] = &PieMenu::ReplacePieSlice; //, luabind::adopt(luabind::result) + luabind::adopt(_3);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundContainer) {
		auto luaType = ConcreteTypeLuaClassDefinition(SoundContainer, Entity);

		luaType.set(sol::call_constructor, sol::constructors<
			SoundContainer()
		>());

		luaType["SoundOverlapMode"] = sol::property(&SoundContainer::GetSoundOverlapMode, &SoundContainer::SetSoundOverlapMode);
		luaType["Immobile"] = sol::property(&SoundContainer::IsImmobile, &SoundContainer::SetImmobile);
		luaType["AttenuationStartDistance"] = sol::property(&SoundContainer::GetAttenuationStartDistance, &SoundContainer::SetAttenuationStartDistance);
		luaType["Loops"] = sol::property(&SoundContainer::GetLoopSetting, &SoundContainer::SetLoopSetting);
		luaType["Priority"] = sol::property(&SoundContainer::GetPriority, &SoundContainer::SetPriority);
		luaType["AffectedByGlobalPitch"] = sol::property(&SoundContainer::IsAffectedByGlobalPitch, &SoundContainer::SetAffectedByGlobalPitch);
		luaType["Pos"] = sol::property(&SoundContainer::GetPosition, &SoundContainer::SetPosition);
		luaType["Volume"] = sol::property(&SoundContainer::GetVolume, &SoundContainer::SetVolume);
		luaType["Pitch"] = sol::property(&SoundContainer::GetPitch, &SoundContainer::SetPitch);
		luaType["PitchVariation"] = sol::property(&SoundContainer::GetPitchVariation, &SoundContainer::SetPitchVariation);

		luaType["HasAnySounds"] = &SoundContainer::HasAnySounds;
		luaType["GetTopLevelSoundSet"] = &SoundContainer::GetTopLevelSoundSet;
		luaType["SetTopLevelSoundSet"] = &SoundContainer::SetTopLevelSoundSet;
		luaType["IsBeingPlayed"] = &SoundContainer::IsBeingPlayed;
		luaType["Play"] = (bool (SoundContainer::*)()) & SoundContainer::Play;
		luaType["Play"] = (bool (SoundContainer::*)(const int player)) & SoundContainer::Play;
		luaType["Play"] = (bool (SoundContainer::*)(const Vector & position)) & SoundContainer::Play;
		luaType["Play"] = (bool (SoundContainer::*)(const Vector & position, int player)) & SoundContainer::Play;
		luaType["Stop"] = (bool (SoundContainer::*)()) & SoundContainer::Stop;
		luaType["Stop"] = (bool (SoundContainer::*)(int player)) & SoundContainer::Stop;
		luaType["Restart"] = (bool (SoundContainer::*)()) & SoundContainer::Restart;
		luaType["Restart"] = (bool (SoundContainer::*)(int player)) & SoundContainer::Restart;
		luaType["FadeOut"] = &SoundContainer::FadeOut;

		{
			sol::table enumTable = LegacyEnumTypeTable("SoundOverlapMode");
			enumTable["OVERLAP"] = SoundContainer::SoundOverlapMode::OVERLAP;
			enumTable["RESTART"] = SoundContainer::SoundOverlapMode::RESTART;
			enumTable["IGNORE_PLAY"] = SoundContainer::SoundOverlapMode::IGNORE_PLAY;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundSet) {
		auto luaType = SimpleTypeLuaClassDefinition(SoundSet);

		luaType.set(sol::call_constructor, sol::constructors<
			SoundSet()
		>());

		luaType["SoundSelectionCycleMode"] = sol::property(&SoundSet::GetSoundSelectionCycleMode, &SoundSet::SetSoundSelectionCycleMode);

		luaType["SubSoundSets"] = sol::readonly(&SoundSet::m_SubSoundSets);

		luaType["HasAnySounds"] = &SoundSet::HasAnySounds;
		luaType["SelectNextSounds"] = &SoundSet::SelectNextSounds;
		luaType["AddSound"] = (void (SoundSet::*)(const std::string & soundFilePath)) & SoundSet::AddSound;
		luaType["AddSound"] = (void (SoundSet::*)(const std::string & soundFilePath, const Vector & offset, float minimumAudibleDistance, float attenuationStartDistance)) & SoundSet::AddSound;
		luaType["RemoveSound"] = (bool (SoundSet::*)(const std::string & soundFilePath)) & SoundSet::RemoveSound;
		luaType["RemoveSound"] = (bool (SoundSet::*)(const std::string & soundFilePath, bool removeFromSubSoundSets)) & SoundSet::RemoveSound;
		luaType["AddSoundSet"] = &SoundSet::AddSoundSet;

		{
			sol::table enumTable = LegacyEnumTypeTable("SoundSelectionCycleMode");
			enumTable["RANDOM"] = SoundSet::SoundSelectionCycleMode::RANDOM;
			enumTable["FORWARDS"] = SoundSet::SoundSelectionCycleMode::FORWARDS;
			enumTable["ALL"] = SoundSet::SoundSelectionCycleMode::ALL;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, GlobalScript) {
		auto luaType = AbstractTypeLuaClassDefinition(GlobalScript, Entity);

		luaType["Deactivate"] = &LuaAdaptersGlobalScript::Deactivate;
	}
}