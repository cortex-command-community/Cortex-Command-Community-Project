#include "Turret.h"

#include "HeldDevice.h"
#include "PresetMan.h"

namespace RTE {

	ConcreteClassInfo(Turret, Attachable, 20);

	void Turret::Clear() {
		m_MountedDevices.clear();
		m_MountedDeviceRotationOffset = 0;
	}

	int Turret::Create(const Turret& reference) {
		if (!reference.m_MountedDevices.empty()) {
			for (const HeldDevice* referenceMountedDevice: reference.m_MountedDevices) {
				m_ReferenceHardcodedAttachableUniqueIDs.insert(referenceMountedDevice->GetUniqueID());
				AddMountedDevice(dynamic_cast<HeldDevice*>(referenceMountedDevice->Clone()));
			}
		}
		Attachable::Create(reference);

		m_MountedDeviceRotationOffset = reference.m_MountedDeviceRotationOffset;

		return 0;
	}

	void Turret::Destroy(bool notInherited) {
		if (!notInherited) {
			Attachable::Destroy();
		}
		for (const HeldDevice* mountedDevice: m_MountedDevices) {
			m_HardcodedAttachableUniqueIDsAndRemovers.erase(mountedDevice->GetUniqueID());
		}
		Clear();
	}

	int Turret::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return Attachable::ReadProperty(propName, reader));

		MatchProperty("MountedDevice", { SetFirstMountedDevice(dynamic_cast<HeldDevice*>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("AddMountedDevice", { AddMountedDevice(dynamic_cast<HeldDevice*>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("MountedDeviceRotationOffset", { reader >> m_MountedDeviceRotationOffset; });

		EndPropertyList;
	}

	int Turret::Save(Writer& writer) const {
		Attachable::Save(writer);

		for (const HeldDevice* mountedDevice: m_MountedDevices) {
			writer.NewProperty("AddMountedDevice");
			writer << mountedDevice;
		}
		writer.NewProperty("MountedDeviceRotationOffset");
		writer << m_MountedDeviceRotationOffset;

		return 0;
	}

	void Turret::SetFirstMountedDevice(HeldDevice* newMountedDevice) {
		if (HasMountedDevice()) {
			RemoveAndDeleteAttachable(m_MountedDevices[0]);
		}
		if (newMountedDevice != nullptr) {
			m_MountedDevices.emplace(m_MountedDevices.begin(), newMountedDevice);
			AddAttachable(newMountedDevice);

			m_HardcodedAttachableUniqueIDsAndRemovers.insert({newMountedDevice->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
				                                                  HeldDevice* castedAttachable = dynamic_cast<HeldDevice*>(attachable);
				                                                  RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to RemoveMountedDevice.");
				                                                  dynamic_cast<Turret*>(parent)->RemoveMountedDevice(castedAttachable);
			                                                  }});

			m_MountedDevices[0]->SetInheritsRotAngle(false);
			m_MountedDevices[0]->SetUnPickupable(true);
			m_MountedDevices[0]->SetGibWithParentChance(1.0F);
			// Force weapons mounted on turrets to never be removed due to forces. This doesn't affect them gibbing from hitting their impulse limits though.
			m_MountedDevices[0]->SetJointStrength(0.0F);
			m_MountedDevices[0]->SetSupportable(true);
			m_MountedDevices[0]->SetSupportAvailable(true);
		}
	}

	void Turret::AddMountedDevice(HeldDevice* newMountedDevice) {
		if (newMountedDevice == nullptr) {
			return;
		}
		m_MountedDevices.emplace_back(newMountedDevice);
		AddAttachable(newMountedDevice);

		m_HardcodedAttachableUniqueIDsAndRemovers.insert({newMountedDevice->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                  HeldDevice* castedAttachable = dynamic_cast<HeldDevice*>(attachable);
			                                                  RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to RemoveMountedDevice.");
			                                                  dynamic_cast<Turret*>(parent)->RemoveMountedDevice(castedAttachable);
		                                                  }});

		newMountedDevice->SetInheritsRotAngle(false);
		newMountedDevice->SetUnPickupable(true);
		newMountedDevice->SetGibWithParentChance(1.0F);
		// Force weapons mounted on turrets to never be removed due to forces. This doesn't affect them gibbing from hitting their impulse limits though.
		newMountedDevice->SetJointStrength(0.0F);
		newMountedDevice->SetSupportable(true);
		newMountedDevice->SetSupportAvailable(true);
	}

	void Turret::Update() {
		for (HeldDevice* mountedDevice: m_MountedDevices) {
			mountedDevice->SetRotAngle(m_Rotation.GetRadAngle() + m_MountedDeviceRotationOffset);
		}
		Attachable::Update();
	}

	void Turret::Draw(BITMAP* pTargetBitmap, const Vector& targetPos, DrawMode mode, bool onlyPhysical) const {
		Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
		// TODO replace this with a relative draw order property or something that lets you organize attachable drawing so it doesn't need special hardcoding crap. Use this for ahuman limbs and arm held mo if possible.
		for (HeldDevice* mountedDevice: m_MountedDevices) {
			if (mountedDevice->IsDrawnAfterParent()) {
				mountedDevice->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
			}
		}
	}

	void Turret::SetParent(MOSRotating* newParent) {
		Attachable::SetParent(newParent);
		for (HeldDevice* mountedDevice: m_MountedDevices) {
			mountedDevice->Deactivate();
		}
	}

	void Turret::RemoveMountedDevice(const HeldDevice* mountedDeviceToRemove) {
		std::vector<HeldDevice*>::iterator mountedDeviceIterator = std::find_if(m_MountedDevices.begin(), m_MountedDevices.end(), [&mountedDeviceToRemove](const HeldDevice* mountedDevice) {
			return mountedDevice == mountedDeviceToRemove;
		});
		m_MountedDevices.erase(mountedDeviceIterator);
	}
} // namespace RTE
