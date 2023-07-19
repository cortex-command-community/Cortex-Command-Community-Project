// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

#include "HDFirearm.h"
#include "HeldDevice.h"
#include "Magazine.h"
#include "Round.h"
#include "TDExplosive.h"
#include "ThrownDevice.h"
#include "Turret.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HDFirearm) {
		auto luaType = ConcreteTypeLuaClassDefinition(HDFirearm, HeldDevice);

		luaType["RateOfFire"] = sol::property(&HDFirearm::GetRateOfFire, &HDFirearm::SetRateOfFire);
		luaType["MSPerRound"] = sol::property(&HDFirearm::GetMSPerRound);
		luaType["FullAuto"] = sol::property(&HDFirearm::IsFullAuto, &HDFirearm::SetFullAuto);
		luaType["Reloadable"] = sol::property(&HDFirearm::IsReloadable, &HDFirearm::SetReloadable);
		luaType["DualReloadable"] = sol::property(&HDFirearm::IsDualReloadable, &HDFirearm::SetDualReloadable);
		luaType["OneHandedReloadTimeMultiplier"] = sol::property(&HDFirearm::GetOneHandedReloadTimeMultiplier, &HDFirearm::SetOneHandedReloadTimeMultiplier);
		luaType["ReloadAngle"] = sol::property(&HDFirearm::GetReloadAngle, &HDFirearm::SetReloadAngle);
		luaType["OneHandedReloadAngle"] = sol::property(&HDFirearm::GetOneHandedReloadAngle, &HDFirearm::SetOneHandedReloadAngle);
		luaType["CurrentReloadAngle"] = sol::property(&HDFirearm::GetCurrentReloadAngle);
		luaType["RoundInMagCount"] = sol::property(&HDFirearm::GetRoundInMagCount);
		luaType["RoundInMagCapacity"] = sol::property(&HDFirearm::GetRoundInMagCapacity);
		luaType["Magazine"] = sol::property(&HDFirearm::GetMagazine, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetMagazine);
		luaType["Flash"] = sol::property(&HDFirearm::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFlash);
		luaType["PreFireSound"] = sol::property(&HDFirearm::GetPreFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetPreFireSound);
		luaType["FireSound"] = sol::property(&HDFirearm::GetFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireSound);
		luaType["FireEchoSound"] = sol::property(&HDFirearm::GetFireEchoSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireEchoSound);
		luaType["ActiveSound"] = sol::property(&HDFirearm::GetActiveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetActiveSound);
		luaType["DeactivationSound"] = sol::property(&HDFirearm::GetDeactivationSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetDeactivationSound);
		luaType["EmptySound"] = sol::property(&HDFirearm::GetEmptySound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetEmptySound);
		luaType["ReloadStartSound"] = sol::property(&HDFirearm::GetReloadStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadStartSound);
		luaType["ReloadEndSound"] = sol::property(&HDFirearm::GetReloadEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadEndSound);
		luaType["ActivationDelay"] = sol::property(&HDFirearm::GetActivationDelay, &HDFirearm::SetActivationDelay);
		luaType["DeactivationDelay"] = sol::property(&HDFirearm::GetDeactivationDelay, &HDFirearm::SetDeactivationDelay);
		luaType["BaseReloadTime"] = sol::property(&HDFirearm::GetBaseReloadTime, &HDFirearm::SetBaseReloadTime);
		luaType["ReloadTime"] = sol::property(&HDFirearm::GetReloadTime);
		luaType["ReloadProgress"] = sol::property(&HDFirearm::GetReloadProgress);
		luaType["ShakeRange"] = sol::property(&HDFirearm::GetShakeRange, &HDFirearm::SetShakeRange);
		luaType["SharpShakeRange"] = sol::property(&HDFirearm::GetSharpShakeRange, &HDFirearm::SetSharpShakeRange);
		luaType["NoSupportFactor"] = sol::property(&HDFirearm::GetNoSupportFactor, &HDFirearm::SetNoSupportFactor);
		luaType["ParticleSpreadRange"] = sol::property(&HDFirearm::GetParticleSpreadRange, &HDFirearm::SetParticleSpreadRange);
		luaType["ShellVelVariation"] = sol::property(&HDFirearm::GetShellVelVariation, &HDFirearm::SetShellVelVariation);
		luaType["FiredOnce"] = sol::property(&HDFirearm::FiredOnce);
		luaType["FiredFrame"] = sol::property(&HDFirearm::FiredFrame);
		luaType["CanFire"] = sol::property(&HDFirearm::CanFire);
		luaType["RoundsFired"] = sol::property(&HDFirearm::RoundsFired);
		luaType["IsAnimatedManually"] = sol::property(&HDFirearm::IsAnimatedManually, &HDFirearm::SetAnimatedManually);
		luaType["RecoilTransmission"] = sol::property(&HDFirearm::GetJointStiffness, &HDFirearm::SetJointStiffness);

		luaType["GetAIFireVel"] = &HDFirearm::GetAIFireVel;
		luaType["GetAIBulletLifeTime"] = &HDFirearm::GetAIBulletLifeTime;
		luaType["GetBulletAccScalar"] = &HDFirearm::GetBulletAccScalar;
		luaType["GetAIBlastRadius"] = &HDFirearm::GetAIBlastRadius;
		luaType["GetAIPenetration"] = &HDFirearm::GetAIPenetration;
		luaType["CompareTrajectories"] = &HDFirearm::CompareTrajectories;
		luaType["GetNextMagazineName"] = &HDFirearm::GetNextMagazineName;
		luaType["SetNextMagazineName"] = &HDFirearm::SetNextMagazineName;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HeldDevice) {
		auto luaType = ConcreteTypeLuaClassDefinition(HeldDevice, Attachable);

		luaType["SupportPos"] = sol::property(&HeldDevice::GetSupportPos);
		luaType["MagazinePos"] = sol::property(&HeldDevice::GetMagazinePos);
		luaType["MuzzlePos"] = sol::property(&HeldDevice::GetMuzzlePos);
		luaType["MuzzleOffset"] = sol::property(&HeldDevice::GetMuzzleOffset, &HeldDevice::SetMuzzleOffset);
		luaType["StanceOffset"] = sol::property(&HeldDevice::GetStanceOffset, &HeldDevice::SetStanceOffset);
		luaType["SharpStanceOffset"] = sol::property(&HeldDevice::GetSharpStanceOffset, &HeldDevice::SetSharpStanceOffset);
		luaType["SharpLength"] = sol::property(&HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength);
		luaType["SharpLength"] = sol::property(&HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength);
		luaType["Supportable"] = sol::property(&HeldDevice::IsSupportable, &HeldDevice::SetSupportable);
		luaType["SupportOffset"] = sol::property(&HeldDevice::GetSupportOffset, &HeldDevice::SetSupportOffset);
		luaType["HasPickupLimitations"] = sol::property(&HeldDevice::HasPickupLimitations);
		luaType["UnPickupable"] = sol::property(&HeldDevice::IsUnPickupable, &HeldDevice::SetUnPickupable);
		luaType["GripStrengthMultiplier"] = sol::property(&HeldDevice::GetGripStrengthMultiplier, &HeldDevice::SetGripStrengthMultiplier);
		luaType["Supported"] = sol::property(&HeldDevice::GetSupported, &HeldDevice::SetSupported);

		luaType["IsWeapon"] = &HeldDevice::IsWeapon;
		luaType["IsTool"] = &HeldDevice::IsTool;
		luaType["IsShield"] = &HeldDevice::IsShield;
		luaType["IsDualWieldable"] = &HeldDevice::IsDualWieldable;
		luaType["SetDualWieldable"] = &HeldDevice::SetDualWieldable;
		luaType["IsOneHanded"] = &HeldDevice::IsOneHanded;
		luaType["SetOneHanded"] = &HeldDevice::SetOneHanded;
		luaType["Activate"] = &HeldDevice::Activate;
		luaType["Deactivate"] = &HeldDevice::Deactivate;
		luaType["Reload"] = &HeldDevice::Reload;
		luaType["IsActivated"] = &HeldDevice::IsActivated;
		luaType["IsReloading"] = &HeldDevice::IsReloading;
		luaType["DoneReloading"] = &HeldDevice::DoneReloading;
		luaType["NeedsReloading"] = &HeldDevice::NeedsReloading;
		luaType["IsFull"] = &HeldDevice::IsFull;
		luaType["IsEmpty"] = &HeldDevice::IsEmpty;
		luaType["IsPickupableBy"] = &HeldDevice::IsPickupableBy;
		luaType["AddPickupableByPresetName"] = &HeldDevice::AddPickupableByPresetName;
		luaType["RemovePickupableByPresetName"] = &HeldDevice::RemovePickupableByPresetName;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Magazine) {
		auto luaType = ConcreteTypeLuaClassDefinition(Magazine, Attachable);

		luaType["NextRound"] = sol::property(&Magazine::GetNextRound);
		luaType["RoundCount"] = sol::property(&Magazine::GetRoundCount, &Magazine::SetRoundCount);
		luaType["IsEmpty"] = sol::property(&Magazine::IsEmpty);
		luaType["IsFull"] = sol::property(&Magazine::IsFull);
		luaType["IsOverHalfFull"] = sol::property(&Magazine::IsOverHalfFull);
		luaType["Capacity"] = sol::property(&Magazine::GetCapacity);
		luaType["Discardable"] = sol::property(&Magazine::IsDiscardable);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Round) {
		auto luaType = ConcreteTypeLuaClassDefinition(Round, Entity);

		luaType["NextParticle"] = sol::property(&Round::GetNextParticle);
		luaType["Shell"] = sol::property(&Round::GetShell);
		luaType["FireVel"] = sol::property(&Round::GetFireVel);
		luaType["InheritsFirerVelocity"] = sol::property(&Round::GetInheritsFirerVelocity);
		luaType["ShellVel"] = sol::property(&Round::GetShellVel);
		luaType["Separation"] = sol::property(&Round::GetSeparation);
		luaType["ParticleCount"] = sol::property(&Round::ParticleCount);
		luaType["AILifeTime"] = sol::property(&Round::GetAILifeTime);
		luaType["AIFireVel"] = sol::property(&Round::GetAIFireVel);
		luaType["IsEmpty"] = sol::property(&Round::IsEmpty);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TDExplosive) {
		auto luaType = ConcreteTypeLuaClassDefinition(TDExplosive, ThrownDevice);

		luaType["IsAnimatedManually"] = sol::property(&TDExplosive::IsAnimatedManually, &TDExplosive::SetAnimatedManually);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ThrownDevice) {
		auto luaType = ConcreteTypeLuaClassDefinition(ThrownDevice, HeldDevice);

		luaType["MinThrowVel"] = sol::property(&ThrownDevice::GetMinThrowVel, &ThrownDevice::SetMinThrowVel);
		luaType["MaxThrowVel"] = sol::property(&ThrownDevice::GetMaxThrowVel, &ThrownDevice::SetMaxThrowVel);
		luaType["StartThrowOffset"] = sol::property(&ThrownDevice::GetStartThrowOffset, &ThrownDevice::SetStartThrowOffset);
		luaType["EndThrowOffset"] = sol::property(&ThrownDevice::GetEndThrowOffset, &ThrownDevice::SetEndThrowOffset);

		luaType["GetCalculatedMaxThrowVelIncludingArmThrowStrength"] = &ThrownDevice::GetCalculatedMaxThrowVelIncludingArmThrowStrength;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Turret) {
		auto luaType = ConcreteTypeLuaClassDefinition(Turret, Attachable);

		luaType["MountedDevice"] = sol::property(&Turret::GetFirstMountedDevice, &LuaAdaptersPropertyOwnershipSafetyFaker::TurretSetFirstMountedDevice);
		luaType["MountedDeviceRotationOffset"] = sol::property(&Turret::GetMountedDeviceRotationOffset, &Turret::SetMountedDeviceRotationOffset);

		luaType["GetMountedDevices"] = &Turret::GetMountedDevices;
		luaType["AddMountedDevice"] = &Turret::AddMountedDevice; //, luabind::adopt(_2);
		luaType["AddMountedDevice"] = &LuaAdaptersTurret::AddMountedFirearm; //, luabind::adopt(_2);
	}
}