// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

#include "Entity.h"
#include "Emission.h"
#include "Gib.h"
#include "MOSprite.h"
#include "AEmitter.h"
#include "Attachable.h"
#include "PEmitter.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Emission) {
		auto luaType = AbstractTypeLuaClassDefinition(Emission, Entity);

		luaType["ParticlesPerMinute"] = sol::property(&Emission::GetRate, &Emission::SetRate);
		luaType["MinVelocity"] = sol::property(&Emission::GetMinVelocity, &Emission::SetMinVelocity);
		luaType["MaxVelocity"] = sol::property(&Emission::GetMaxVelocity, &Emission::SetMaxVelocity);
		luaType["PushesEmitter"] = sol::property(&Emission::PushesEmitter, &Emission::SetPushesEmitter);
		luaType["LifeVariation"] = sol::property(&Emission::GetLifeVariation, &Emission::SetLifeVariation);
		luaType["BurstSize"] = sol::property(&Emission::GetBurstSize, &Emission::SetBurstSize);
		luaType["Spread"] = sol::property(&Emission::GetSpread, &Emission::SetSpread);
		luaType["Offset"] = sol::property(&Emission::GetOffset, &Emission::SetOffset);

		luaType["ResetEmissionTimers"] = &Emission::ResetEmissionTimers;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Gib) {
		auto luaType = SimpleTypeLuaClassDefinition(Gib);

		luaType["ParticlePreset"] = sol::property(&Gib::GetParticlePreset, &Gib::SetParticlePreset);
		luaType["MinVelocity"] = sol::property(&Gib::GetMinVelocity, &Gib::SetMinVelocity);
		luaType["MaxVelocity"] = sol::property(&Gib::GetMaxVelocity, &Gib::SetMaxVelocity);
		luaType["SpreadMode"] = sol::property(&Gib::GetSpreadMode, &Gib::SetSpreadMode);

		luaType["Offset"] = &Gib::m_Offset;
		luaType["Count"] = &Gib::m_Count;
		luaType["Spread"] = &Gib::m_Spread;
		luaType["LifeVariation"] = &Gib::m_LifeVariation;
		luaType["InheritsVel"] = &Gib::m_InheritsVel;
		luaType["IgnoresTeamHits"] = &Gib::m_IgnoresTeamHits;

		luaType.new_enum("SpreadMode", EnumList(Gib::SpreadMode) {
			{ "SpreadRandom", Gib::SpreadMode::SpreadRandom },
			{ "SpreadEven", Gib::SpreadMode::SpreadEven },
			{ "SpreadSpiral", Gib::SpreadMode::SpreadSpiral }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AEmitter) {
		auto luaType = ConcreteTypeLuaClassDefinition(AEmitter, Attachable);

		luaType["EmissionSound"] = sol::property(&AEmitter::GetEmissionSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEmissionSound);
		luaType["BurstSound"] = sol::property(&AEmitter::GetBurstSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetBurstSound);
		luaType["EndSound"] = sol::property(&AEmitter::GetEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEndSound);
		luaType["BurstScale"] = sol::property(&AEmitter::GetBurstScale, &AEmitter::SetBurstScale);
		luaType["EmitAngle"] = sol::property(&AEmitter::GetEmitAngle, &AEmitter::SetEmitAngle);
		luaType["GetThrottle"] = sol::property(&AEmitter::GetThrottle, &AEmitter::SetThrottle);
		luaType["Throttle"] = sol::property(&AEmitter::GetThrottle, &AEmitter::SetThrottle);
		luaType["ThrottleFactor"] = sol::property(&AEmitter::GetThrottleFactor);
		luaType["NegativeThrottleMultiplier"] = sol::property(&AEmitter::GetNegativeThrottleMultiplier, &AEmitter::SetNegativeThrottleMultiplier);
		luaType["PositiveThrottleMultiplier"] = sol::property(&AEmitter::GetPositiveThrottleMultiplier, &AEmitter::SetPositiveThrottleMultiplier);
		luaType["BurstSpacing"] = sol::property(&AEmitter::GetBurstSpacing, &AEmitter::SetBurstSpacing);
		luaType["BurstDamage"] = sol::property(&AEmitter::GetBurstDamage, &AEmitter::SetBurstDamage);
		luaType["EmitterDamageMultiplier"] = sol::property(&AEmitter::GetEmitterDamageMultiplier, &AEmitter::SetEmitterDamageMultiplier);
		luaType["EmitCount"] = sol::property(&AEmitter::GetEmitCount);
		luaType["EmitCountLimit"] = sol::property(&AEmitter::GetEmitCountLimit, &AEmitter::SetEmitCountLimit);
		luaType["EmitDamage"] = sol::property(&AEmitter::GetEmitDamage, &AEmitter::SetEmitDamage);
		luaType["EmitOffset"] = sol::property(&AEmitter::GetEmitOffset, &AEmitter::SetEmitOffset);
		luaType["Flash"] = sol::property(&AEmitter::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetFlash);
		luaType["FlashScale"] = sol::property(&AEmitter::GetFlashScale, &AEmitter::SetFlashScale);
		luaType["TotalParticlesPerMinute"] = sol::property(&AEmitter::GetTotalParticlesPerMinute);
		luaType["TotalBurstSize"] = sol::property(&AEmitter::GetTotalBurstSize);

		luaType["Emissions"] = &AEmitter::m_EmissionList;

		luaType["IsEmitting"] = &AEmitter::IsEmitting;
		luaType["EnableEmission"] = &AEmitter::EnableEmission;
		luaType["GetEmitVector"] = &AEmitter::GetEmitVector;
		luaType["GetRecoilVector"] = &AEmitter::GetRecoilVector;
		luaType["EstimateImpulse"] = &AEmitter::EstimateImpulse;
		luaType["TriggerBurst"] = &AEmitter::TriggerBurst;
		luaType["IsSetToBurst"] = &AEmitter::IsSetToBurst;
		luaType["CanTriggerBurst"] = &AEmitter::CanTriggerBurst;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Attachable) {
		auto luaType = ConcreteTypeLuaClassDefinition(Attachable, MOSRotating);

		luaType["ParentOffset"] = sol::property(&Attachable::GetParentOffset, &Attachable::SetParentOffset);
		luaType["JointStrength"] = sol::property(&Attachable::GetJointStrength, &Attachable::SetJointStrength);
		luaType["JointStiffness"] = sol::property(&Attachable::GetJointStiffness, &Attachable::SetJointStiffness);
		luaType["JointOffset"] = sol::property(&Attachable::GetJointOffset, &Attachable::SetJointOffset);
		luaType["JointPos"] = sol::property(&Attachable::GetJointPos);
		luaType["DeleteWhenRemovedFromParent"] = sol::property(&Attachable::GetDeleteWhenRemovedFromParent, &Attachable::SetDeleteWhenRemovedFromParent);
		luaType["GibWhenRemovedFromParent"] = sol::property(&Attachable::GetGibWhenRemovedFromParent, &Attachable::SetGibWhenRemovedFromParent);
		luaType["ApplyTransferredForcesAtOffset"] = sol::property(&Attachable::GetApplyTransferredForcesAtOffset, &Attachable::SetApplyTransferredForcesAtOffset);
		luaType["BreakWound"] = sol::property(&Attachable::GetBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetBreakWound);
		luaType["ParentBreakWound"] = sol::property(&Attachable::GetParentBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetParentBreakWound);
		luaType["InheritsHFlipped"] = sol::property(&Attachable::InheritsHFlipped, &Attachable::SetInheritsHFlipped);
		luaType["InheritsRotAngle"] = sol::property(&Attachable::InheritsRotAngle, &Attachable::SetInheritsRotAngle);
		luaType["InheritedRotAngleOffset"] = sol::property(&Attachable::GetInheritedRotAngleOffset, &Attachable::SetInheritedRotAngleOffset);
		luaType["AtomSubgroupID"] = sol::property(&Attachable::GetAtomSubgroupID);
		luaType["CollidesWithTerrainWhileAttached"] = sol::property(&Attachable::GetCollidesWithTerrainWhileAttached, &Attachable::SetCollidesWithTerrainWhileAttached);
		luaType["IgnoresParticlesWhileAttached"] = sol::property(&Attachable::GetIgnoresParticlesWhileAttached, &Attachable::SetIgnoresParticlesWhileAttached);
		luaType["CanCollideWithTerrain"] = sol::property(&Attachable::CanCollideWithTerrain);
		luaType["DrawnAfterParent"] = sol::property(&Attachable::IsDrawnAfterParent, &Attachable::SetDrawnAfterParent);
		luaType["InheritsFrame"] = sol::property(&Attachable::InheritsFrame, &Attachable::SetInheritsFrame);

		luaType["IsAttached"] = &Attachable::IsAttached;
		luaType["IsAttachedTo"] = &Attachable::IsAttachedTo;

		luaType["RemoveFromParent"] = &LuaAdaptersAttachable::RemoveFromParent1;//; //, luabind::adopt(luabind::return_value);
		luaType["RemoveFromParent"] = &LuaAdaptersAttachable::RemoveFromParent2;//; //, luabind::adopt(luabind::return_value);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PEmitter) {
		auto luaType = ConcreteTypeLuaClassDefinition(PEmitter, MOSParticle);

		luaType["BurstScale"] = sol::property(&PEmitter::GetBurstScale, &PEmitter::SetBurstScale);
		luaType["EmitAngle"] = sol::property(&PEmitter::GetEmitAngle, &PEmitter::SetEmitAngle);
		luaType["GetThrottle"] = sol::property(&PEmitter::GetThrottle, &PEmitter::SetThrottle);
		luaType["Throttle"] = sol::property(&PEmitter::GetThrottle, &PEmitter::SetThrottle);
		luaType["ThrottleFactor"] = sol::property(&PEmitter::GetThrottleFactor);
		luaType["BurstSpacing"] = sol::property(&PEmitter::GetBurstSpacing, &PEmitter::SetBurstSpacing);
		luaType["EmitCountLimit"] = sol::property(&PEmitter::GetEmitCountLimit, &PEmitter::SetEmitCountLimit);
		luaType["FlashScale"] = sol::property(&PEmitter::GetFlashScale, &PEmitter::SetFlashScale);

		luaType["Emissions"] = &PEmitter::m_EmissionList;

		luaType["IsEmitting"] = &PEmitter::IsEmitting;
		luaType["EnableEmission"] = &PEmitter::EnableEmission;
		luaType["GetEmitVector"] = &PEmitter::GetEmitVector;
		luaType["GetRecoilVector"] = &PEmitter::GetRecoilVector;
		luaType["EstimateImpulse"] = &PEmitter::EstimateImpulse;
		luaType["TriggerBurst"] = &PEmitter::TriggerBurst;
		luaType["IsSetToBurst"] = &PEmitter::IsSetToBurst;
		luaType["CanTriggerBurst"] = &PEmitter::CanTriggerBurst;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSprite) {
		auto luaType = AbstractTypeLuaClassDefinition(MOSprite, MovableObject);

		luaType["Diameter"] = sol::property(&MOSprite::GetDiameter);
		luaType["BoundingBox"] = sol::property(&MOSprite::GetBoundingBox);
		luaType["FrameCount"] = sol::property(&MOSprite::GetFrameCount);
		luaType["SpriteOffset"] = sol::property(&MOSprite::GetSpriteOffset, &MOSprite::SetSpriteOffset);
		luaType["HFlipped"] = sol::property(&MOSprite::IsHFlipped, &MOSprite::SetHFlipped);
		luaType["FlipFactor"] = sol::property(&MOSprite::GetFlipFactor);
		luaType["RotAngle"] = sol::property(&MOSprite::GetRotAngle, &MOSprite::SetRotAngle);
		luaType["PrevRotAngle"] = sol::property(&MOSprite::GetPrevRotAngle);
		luaType["AngularVel"] = sol::property(&MOSprite::GetAngularVel, &MOSprite::SetAngularVel);
		luaType["Frame"] = sol::property(&MOSprite::GetFrame, &MOSprite::SetFrame);
		luaType["SpriteAnimMode"] = sol::property(&MOSprite::GetSpriteAnimMode, &MOSprite::SetSpriteAnimMode);
		luaType["SpriteAnimDuration"] = sol::property(&MOSprite::GetSpriteAnimDuration, &MOSprite::SetSpriteAnimDuration);

		luaType["SetNextFrame"] = &MOSprite::SetNextFrame;
		luaType["IsTooFast"] = &MOSprite::IsTooFast;
		luaType["IsOnScenePoint"] = &MOSprite::IsOnScenePoint;
		luaType["RotateOffset"] = &MOSprite::RotateOffset;
		luaType["UnRotateOffset"] = &MOSprite::UnRotateOffset;
		luaType["FacingAngle"] = &MOSprite::FacingAngle;
		luaType["GetSpriteWidth"] = &MOSprite::GetSpriteWidth;
		luaType["GetSpriteHeight"] = &MOSprite::GetSpriteHeight;
		luaType["GetIconWidth"] = &MOSprite::GetIconWidth;
		luaType["GetIconHeight"] = &MOSprite::GetIconHeight;
		luaType["SetEntryWound"] = &MOSprite::SetEntryWound;
		luaType["SetExitWound"] = &MOSprite::SetExitWound;
		luaType["GetEntryWoundPresetName"] = &MOSprite::GetEntryWoundPresetName;
		luaType["GetExitWoundPresetName"] = &MOSprite::GetExitWoundPresetName;

		luaType.new_enum("SpriteAnimMode", EnumList(SpriteAnimMode) {
			{ "NOANIM", SpriteAnimMode::NOANIM },
			{ "ALWAYSLOOP", SpriteAnimMode::ALWAYSLOOP },
			{ "ALWAYSRANDOM", SpriteAnimMode::ALWAYSRANDOM },
			{ "ALWAYSPINGPONG", SpriteAnimMode::ALWAYSPINGPONG },
			{ "LOOPWHENACTIVE", SpriteAnimMode::LOOPWHENACTIVE },
			{ "LOOPWHENOPENCLOSE", SpriteAnimMode::LOOPWHENOPENCLOSE },
			{ "PINGPONGOPENCLOSE", SpriteAnimMode::PINGPONGOPENCLOSE },
			{ "OVERLIFETIME", SpriteAnimMode::OVERLIFETIME },
			{ "ONCOLLIDE", SpriteAnimMode::ONCOLLIDE }
		});
	}
}