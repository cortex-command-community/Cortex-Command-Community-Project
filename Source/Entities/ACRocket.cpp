#include "ACRocket.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "Leg.h"
#include "Controller.h"
#include "Matrix.h"
#include "AEmitter.h"
#include "SettingsMan.h"
#include "PresetMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

using namespace RTE;

ConcreteClassInfo(ACRocket, ACraft, 10);

ACRocket::ACRocket() {
	Clear();
}

ACRocket::~ACRocket() {
	Destroy(true);
}

void ACRocket::Clear() {
	//    m_pCapsule = 0;
	m_pRLeg = 0;
	m_pLLeg = 0;
	m_pBodyAG = 0;
	m_pRFootGroup = 0;
	m_pLFootGroup = 0;
	m_pMThruster = 0;
	m_pRThruster = 0;
	m_pLThruster = 0;
	m_pURThruster = 0;
	m_pULThruster = 0;
	m_GearState = RAISED;
	for (int i = 0; i < GearStateCount; ++i) {
		m_Paths[RIGHT][i].Reset();
		m_Paths[LEFT][i].Reset();
		m_Paths[RIGHT][i].Terminate();
		m_Paths[LEFT][i].Terminate();
	}
	m_MaxGimbalAngle = 0;
}

int ACRocket::Create() {
	// Read all the properties
	if (ACraft::Create() < 0)
		return -1;

	// Save the AtomGroup read in by MOSRotating, as we are going to make it
	// into a composite group, and want to have the base body stored for reference.
	m_pBodyAG = dynamic_cast<AtomGroup*>(m_pAtomGroup->Clone());

	// Mirror the limb paths
	for (int i = 0; i < GearStateCount; ++i) {
		m_Paths[LEFT][i].Create(m_Paths[RIGHT][i]);
	}

	// Override the body animation mode
	m_SpriteAnimMode = PINGPONGOPENCLOSE;

	return 0;
}

int ACRocket::Create(const ACRocket& reference) {
	if (reference.m_pRLeg) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRLeg->GetUniqueID());
	}
	if (reference.m_pLLeg) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLLeg->GetUniqueID());
	}
	if (reference.m_pMThruster) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pMThruster->GetUniqueID());
	}
	if (reference.m_pRThruster) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRThruster->GetUniqueID());
	}
	if (reference.m_pLThruster) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLThruster->GetUniqueID());
	}
	if (reference.m_pURThruster) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pURThruster->GetUniqueID());
	}
	if (reference.m_pULThruster) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pULThruster->GetUniqueID());
	}

	ACraft::Create(reference);

	if (reference.m_pRLeg) {
		SetRightLeg(dynamic_cast<Leg*>(reference.m_pRLeg->Clone()));
	}
	if (reference.m_pLLeg) {
		SetLeftLeg(dynamic_cast<Leg*>(reference.m_pLLeg->Clone()));
	}
	if (reference.m_pMThruster) {
		SetMainThruster(dynamic_cast<AEmitter*>(reference.m_pMThruster->Clone()));
	}
	if (reference.m_pRThruster) {
		SetRightThruster(dynamic_cast<AEmitter*>(reference.m_pRThruster->Clone()));
	}
	if (reference.m_pLThruster) {
		SetLeftThruster(dynamic_cast<AEmitter*>(reference.m_pLThruster->Clone()));
	}
	if (reference.m_pURThruster) {
		SetURightThruster(dynamic_cast<AEmitter*>(reference.m_pURThruster->Clone()));
	}
	if (reference.m_pULThruster) {
		SetULeftThruster(dynamic_cast<AEmitter*>(reference.m_pULThruster->Clone()));
	}

	m_pBodyAG = dynamic_cast<AtomGroup*>(reference.m_pBodyAG->Clone());
	m_pBodyAG->SetOwner(this);

	if (reference.m_pLFootGroup) {
		m_pLFootGroup = dynamic_cast<AtomGroup*>(reference.m_pLFootGroup->Clone());
	} else if (m_pLLeg) {
		m_pLFootGroup = m_pLLeg->GetFootGroupFromFootAtomGroup();
	}
	if (m_pLFootGroup) {
		m_pLFootGroup->SetOwner(this);
	}

	if (reference.m_pRFootGroup) {
		m_pRFootGroup = dynamic_cast<AtomGroup*>(reference.m_pRFootGroup->Clone());
	} else if (m_pRLeg) {
		m_pRFootGroup = m_pRLeg->GetFootGroupFromFootAtomGroup();
	}
	if (m_pRFootGroup) {
		m_pRFootGroup->SetOwner(this);
	}

	m_GearState = reference.m_GearState;

	for (int i = 0; i < GearStateCount; ++i) {
		m_Paths[RIGHT][i].Create(reference.m_Paths[RIGHT][i]);
		m_Paths[LEFT][i].Create(reference.m_Paths[LEFT][i]);
	}

	m_MaxGimbalAngle = reference.m_MaxGimbalAngle;

	return 0;
}

int ACRocket::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return ACraft::ReadProperty(propName, reader));

	MatchForwards("RLeg") MatchProperty("RightLeg", { SetRightLeg(dynamic_cast<Leg*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchForwards("LLeg") MatchProperty("LeftLeg", { SetLeftLeg(dynamic_cast<Leg*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchForwards("RFootGroup") MatchProperty("RightFootGroup", {
		delete m_pRFootGroup;
		m_pRFootGroup = new AtomGroup();
		reader >> m_pRFootGroup;
		m_pRFootGroup->SetOwner(this);
	});
	MatchForwards("LFootGroup") MatchProperty("LeftFootGroup", {
		delete m_pLFootGroup;
		m_pLFootGroup = new AtomGroup();
		reader >> m_pLFootGroup;
		m_pLFootGroup->SetOwner(this);
	});
	MatchForwards("MThruster") MatchProperty("MainThruster", { SetMainThruster(dynamic_cast<AEmitter*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchForwards("RThruster") MatchProperty("RightThruster", { SetRightThruster(dynamic_cast<AEmitter*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchForwards("LThruster") MatchProperty("LeftThruster", { SetLeftThruster(dynamic_cast<AEmitter*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchForwards("URThruster") MatchProperty("UpRightThruster", { SetURightThruster(dynamic_cast<AEmitter*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchForwards("ULThruster") MatchProperty("UpLeftThruster", { SetULeftThruster(dynamic_cast<AEmitter*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("RaisedGearLimbPath", { reader >> m_Paths[RIGHT][RAISED]; });
	MatchProperty("LoweredGearLimbPath", { reader >> m_Paths[RIGHT][LOWERED]; });
	MatchProperty("LoweringGearLimbPath", { reader >> m_Paths[RIGHT][LOWERING]; });
	MatchProperty("RaisingGearLimbPath", { reader >> m_Paths[RIGHT][RAISING]; });
	MatchProperty("MaxGimbalAngle", {
		reader >> m_MaxGimbalAngle;
		m_MaxGimbalAngle *= (c_PI / 180.0F);
	});

	EndPropertyList;
}

int ACRocket::Save(Writer& writer) const {
	ACraft::Save(writer);

	writer.NewPropertyWithValue("RLeg", m_pRLeg);
	writer.NewPropertyWithValue("LLeg", m_pLLeg);
	writer.NewPropertyWithValue("RFootGroup", m_pRFootGroup);
	writer.NewPropertyWithValue("LFootGroup", m_pLFootGroup);
	writer.NewPropertyWithValue("MThruster", m_pMThruster);
	writer.NewPropertyWithValue("RThruster", m_pRThruster);
	writer.NewPropertyWithValue("LThruster", m_pLThruster);
	writer.NewPropertyWithValue("URThruster", m_pURThruster);
	writer.NewPropertyWithValue("ULThruster", m_pULThruster);
	writer.NewPropertyWithValue("RaisedGearLimbPath", m_Paths[RIGHT][RAISED]);
	writer.NewPropertyWithValue("LoweredGearLimbPath", m_Paths[RIGHT][LOWERED]);
	writer.NewPropertyWithValue("LoweringGearLimbPath", m_Paths[RIGHT][LOWERING]);
	writer.NewPropertyWithValue("RaisingGearLimbPath", m_Paths[RIGHT][RAISING]);
	writer.NewPropertyWithValue("MaxGimbalAngle", m_MaxGimbalAngle / (c_PI / 180.0F));

	return 0;
}

size_t ACRocket::Write(Writer& writer, const Entity& entityReference, const HashingData& hashData) const {
	size_t constituentsConsumed = ACraft::Write(writer, entityReference, hashData); // 15

	const ACRocket& reference = static_cast<const ACRocket&>(entityReference);

	if (m_pRLeg != nullptr) {
		if (reference.m_pRLeg == nullptr || m_pRLeg->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("RLeg");
			if (const Entity* preset = m_pRLeg->GetPreset()) {
				m_pRLeg->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pRLeg;
			}
		}
	} else if (reference.m_pRLeg != nullptr) {
		writer.NewProperty("RLeg");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(12);
	
	if (m_pLLeg != nullptr) {
		if (reference.m_pLLeg == nullptr || m_pLLeg->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("LLeg");
			if (const Entity* preset = m_pLLeg->GetPreset()) {
				m_pLLeg->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pLLeg;
			}
		}
	} else if (reference.m_pLLeg != nullptr) {
		writer.NewProperty("LLeg");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(13);
	
	if (m_pRFootGroup != nullptr) {
		if (reference.m_pRFootGroup == nullptr || m_pRFootGroup->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("RFootGroup");
			if (const Entity* preset = m_pRFootGroup->GetPreset()) {
				m_pRFootGroup->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pRFootGroup;
			}
		}
	} else if (reference.m_pRFootGroup != nullptr) {
		writer.NewProperty("RFootGroup");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(14);
	
	if (m_pLFootGroup != nullptr) {
		if (reference.m_pLFootGroup == nullptr || m_pLFootGroup->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("LFootGroup");
			if (const Entity* preset = m_pLFootGroup->GetPreset()) {
				m_pLFootGroup->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pLFootGroup;
			}
		}
	} else if (reference.m_pLFootGroup != nullptr) {
		writer.NewProperty("LFootGroup");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(15);
	
	if (m_pMThruster != nullptr) {
		if (reference.m_pMThruster == nullptr || m_pMThruster->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("MThruster");
			if (const Entity* preset = m_pMThruster->GetPreset()) {
				m_pMThruster->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pMThruster;
			}
		}
	} else if (reference.m_pMThruster != nullptr) {
		writer.NewProperty("MThruster");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(16);
	
	if (m_pRThruster != nullptr) {
		if (reference.m_pRThruster == nullptr || m_pRThruster->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("RThruster");
			if (const Entity* preset = m_pRThruster->GetPreset()) {
				m_pRThruster->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pRThruster;
			}
		}
	} else if (reference.m_pRThruster != nullptr) {
		writer.NewProperty("RThruster");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(17);
	
	if (m_pLThruster != nullptr) {
		if (reference.m_pLThruster == nullptr || m_pLThruster->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("LThruster");
			if (const Entity* preset = m_pLThruster->GetPreset()) {
				m_pLThruster->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pLThruster;
			}
		}
	} else if (reference.m_pLThruster != nullptr) {
		writer.NewProperty("LThruster");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(18);
	
	if (m_pURThruster != nullptr) {
		if (reference.m_pURThruster == nullptr || m_pURThruster->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("URThruster");
			if (const Entity* preset = m_pURThruster->GetPreset()) {
				m_pURThruster->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pURThruster;
			}
		}
	} else if (reference.m_pURThruster != nullptr) {
		writer.NewProperty("URThruster");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(19);
	
	if (m_pULThruster != nullptr) {
		if (reference.m_pULThruster == nullptr || m_pULThruster->Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed)) {
			writer.NewProperty("ULThruster");
			if (const Entity* preset = m_pULThruster->GetPreset()) {
				m_pULThruster->Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
				writer.ObjectEnd();
			} else {
				writer << m_pULThruster;
			}
		}
	} else if (reference.m_pULThruster != nullptr) {
		writer.NewProperty("ULThruster");
		writer << "None";
	}
	
	constituentsConsumed += hashData.m_ParseValues.at(20);
	
	if (m_Paths[RIGHT][RAISED].Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed++)) {
		writer.NewProperty("RaisedGearLimbPath");
		if (const Entity* preset = m_Paths[RIGHT][RAISED].GetPreset()) {
			m_Paths[RIGHT][RAISED].Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
			writer.ObjectEnd();
		} else {
			writer << m_Paths[RIGHT][RAISED];
		}
	}
	
	if (m_Paths[RIGHT][LOWERED].Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed++)) {
		writer.NewProperty("LoweredGearLimbPath");
		if (const Entity* preset = m_Paths[RIGHT][LOWERED].GetPreset()) {
			m_Paths[RIGHT][LOWERED].Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
			writer.ObjectEnd();
		} else {
			writer << m_Paths[RIGHT][LOWERED];
		}
	}
	
	if (m_Paths[RIGHT][LOWERING].Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed++)) {
		writer.NewProperty("LoweringGearLimbPath");
		if (const Entity* preset = m_Paths[RIGHT][LOWERING].GetPreset()) {
			m_Paths[RIGHT][LOWERING].Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
			writer.ObjectEnd();
		} else {
			writer << m_Paths[RIGHT][LOWERING];
		}
	}
	
	if (m_Paths[RIGHT][RAISING].Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed++)) {
		writer.NewProperty("RaisingGearLimbPath");
		if (const Entity* preset = m_Paths[RIGHT][RAISING].GetPreset()) {
			m_Paths[RIGHT][RAISING].Write(writer, *preset, *g_PresetMan.GetEntityHash(preset->GetClassName(), preset->GetPresetName(), preset->GetModuleID()));
			writer.ObjectEnd();
		} else {
			writer << m_Paths[RIGHT][RAISING];
		}
	}
	
	if (m_MaxGimbalAngle != reference.m_MaxGimbalAngle)
		writer.NewPropertyWithValue("MaxGimbalAngle", m_MaxGimbalAngle / (c_PI / 180.0F));

	return constituentsConsumed;
}

HashingData ACRocket::Hash() const {
	HashingData hashData = ACraft::Hash();
	uint64_t& hash = hashData.m_Hash;
	
	bool rLegDef = m_pRLeg != nullptr;
	hashData.m_ParseValues.push_back(rLegDef);
	if (rLegDef) {
		uint64_t rLegHash = m_pRLeg->Hash().m_Hash;
		hashData.m_Constituents.push_back(rLegHash);
		hash ^= rLegHash << 1;
	}

	bool lLegDef = m_pLLeg != nullptr;
	hashData.m_ParseValues.push_back(lLegDef);
	if (lLegDef) {
		uint64_t lLegHash = m_pLLeg->Hash().m_Hash;
		hashData.m_Constituents.push_back(lLegHash);
		hash ^= lLegHash << 2;
	}

	bool rFootGroupDef = m_pRFootGroup != nullptr;
	hashData.m_ParseValues.push_back(rFootGroupDef);
	if (rFootGroupDef) {
		uint64_t rFootGroupHash = m_pRFootGroup->Hash().m_Hash;
		hashData.m_Constituents.push_back(rFootGroupHash);
		hash ^= rFootGroupHash << 3;
	}

	bool lFootGroupDef = m_pLFootGroup != nullptr;
	hashData.m_ParseValues.push_back(lFootGroupDef);
	if (lFootGroupDef) {
		uint64_t lFootGroupHash = m_pLFootGroup->Hash().m_Hash;
		hashData.m_Constituents.push_back(lFootGroupHash);
		hash ^= lFootGroupHash << 4;
	}

	bool mThrusterDef = m_pMThruster != nullptr;
	hashData.m_ParseValues.push_back(mThrusterDef);
	if (mThrusterDef) {
		uint64_t mThrusterHash = m_pMThruster->Hash().m_Hash;
		hashData.m_Constituents.push_back(mThrusterHash);
		hash ^= mThrusterHash << 5;
	}

	bool rThrusterDef = m_pRThruster != nullptr;
	hashData.m_ParseValues.push_back(rThrusterDef);
	if (rThrusterDef) {
		uint64_t rThrusterHash = m_pRThruster->Hash().m_Hash;
		hashData.m_Constituents.push_back(rThrusterHash);
		hash ^= rThrusterHash << 6;
	}

	bool lThrusterDef = m_pLThruster != nullptr;
	hashData.m_ParseValues.push_back(lThrusterDef);
	if (lThrusterDef) {
		uint64_t lThrusterHash = m_pLThruster->Hash().m_Hash;
		hashData.m_Constituents.push_back(lThrusterHash);
		hash ^= lThrusterHash << 7;
	}

	bool urThrusterDef = m_pURThruster != nullptr;
	hashData.m_ParseValues.push_back(urThrusterDef);
	if (urThrusterDef) {
		uint64_t urFootGroupHash = m_pURThruster->Hash().m_Hash;
		hashData.m_Constituents.push_back(urFootGroupHash);
		hash ^= urFootGroupHash << 8;
	}

	bool ulThrusterDef = m_pULThruster != nullptr;
	hashData.m_ParseValues.push_back(ulThrusterDef);
	if (ulThrusterDef) {
		uint64_t ulFootGroupHash = m_pULThruster->Hash().m_Hash;
		hashData.m_Constituents.push_back(ulFootGroupHash);
		hash ^= ulFootGroupHash << 9;
	}

	uint64_t rightRaisedPathHash = m_Paths[RIGHT][RAISED].Hash().m_Hash;
	hashData.m_Constituents.push_back(rightRaisedPathHash);
	hash ^= rightRaisedPathHash << 10;

	uint64_t rightLoweredPathHash = m_Paths[RIGHT][LOWERED].Hash().m_Hash;
	hashData.m_Constituents.push_back(rightLoweredPathHash);
	hash ^= rightLoweredPathHash << 11;

	uint64_t rightLoweringPathHash = m_Paths[RIGHT][LOWERING].Hash().m_Hash;
	hashData.m_Constituents.push_back(rightLoweringPathHash);
	hash ^= rightLoweringPathHash << 12;

	uint64_t rightRaisingHash = m_Paths[RIGHT][RAISING].Hash().m_Hash;
	hashData.m_Constituents.push_back(rightRaisingHash);
	hash ^= rightRaisingHash << 13;

	hash ^= std::hash<float>{}(m_MaxGimbalAngle) << 14;

	return hashData;
}

void ACRocket::Destroy(bool notInherited) {
	delete m_pBodyAG;
	delete m_pRFootGroup;
	delete m_pLFootGroup;

	//    for (deque<LimbPath *>::iterator itr = m_WalkPaths.begin();
	//         itr != m_WalkPaths.end(); ++itr)
	//        delete *itr;

	if (!notInherited)
		ACraft::Destroy();
	Clear();
}

float ACRocket::GetAltitude(int max, int accuracy) {
	// Use the main thruster's position as the position ot measure from
	Vector pos;
	if (m_pMThruster && m_pMThruster->IsAttached())
		pos = m_Pos + RotateOffset(m_pMThruster->GetParentOffset());
	else
		pos = m_Pos;

	return g_SceneMan.FindAltitude(pos, max, accuracy, true);
}

void ACRocket::PreControllerUpdate() {
	ACraft::PreControllerUpdate();

	float deltaTime = g_TimerMan.GetDeltaTimeSecs();

	// Look/aim update, make the scanner point aftward if the rocket is falling
	m_AimAngle = m_Vel.m_Y < 0 ? c_HalfPI : -c_HalfPI;

	/////////////////////////////////
	// Controller update and handling

	if ((m_Status == STABLE || m_Status == UNSTABLE) && !m_Controller.IsDisabled()) {
		if (m_pMThruster) {
			if (m_MaxGimbalAngle != 0) {
				m_pMThruster->SetInheritedRotAngleOffset(std::sin(m_Rotation.GetRadAngle()) * m_MaxGimbalAngle - c_HalfPI);
			}

			if (m_Controller.IsState(MOVE_UP) || m_Controller.IsState(AIM_UP)) {
				if (!m_pMThruster->IsEmitting()) {
					m_pMThruster->TriggerBurst();
					m_ForceDeepCheck = true;
				}
				m_pMThruster->EnableEmission(true);
				m_pMThruster->AlarmOnEmit(m_Team);

				if (m_HatchState == OPEN) {
					CloseHatch();
					m_HatchTimer.Reset();
				}
			} else {
				m_pMThruster->EnableEmission(false);
			}
		}
		if (m_pULThruster || m_pURThruster) {
			if (m_Controller.IsState(MOVE_DOWN) || m_Controller.IsState(AIM_DOWN)) {
				if (m_pURThruster) {
					if (!m_pURThruster->IsEmitting()) {
						m_pURThruster->TriggerBurst();
					}
					m_pURThruster->EnableEmission(true);
				}
				if (m_pULThruster) {
					if (!m_pULThruster->IsEmitting()) {
						m_pULThruster->TriggerBurst();
					}
					m_pULThruster->EnableEmission(true);
				}
			} else {
				if (m_pURThruster) {
					m_pURThruster->EnableEmission(false);
				}
				if (m_pULThruster) {
					m_pULThruster->EnableEmission(false);
				}
			}
		}
		if (m_pLThruster) {
			if (m_Controller.IsState(MOVE_RIGHT)) {
				if (!m_pLThruster->IsEmitting()) {
					m_pLThruster->TriggerBurst();
				}
				m_pLThruster->EnableEmission(true);
			} else {
				m_pLThruster->EnableEmission(false);
			}
		}
		if (m_pRThruster) {
			if (m_Controller.IsState(MOVE_LEFT)) {
				if (!m_pRThruster->IsEmitting()) {
					m_pRThruster->TriggerBurst();
				}
				m_pRThruster->EnableEmission(true);
			} else {
				m_pRThruster->EnableEmission(false);
			}
		}
		if (m_Controller.IsState(PRESS_FACEBUTTON)) {
			if (m_HatchState == CLOSED) {
				DropAllInventory();
			} else if (m_HatchState == OPEN) {
				CloseHatch();
			}
		}
	} else {
		if (m_pMThruster) {
			m_pMThruster->EnableEmission(false);
		}
		if (m_pRThruster) {
			m_pRThruster->EnableEmission(false);
		}
		if (m_pLThruster) {
			m_pLThruster->EnableEmission(false);
		}
		if (m_pURThruster) {
			m_pURThruster->EnableEmission(false);
		}
		if (m_pULThruster) {
			m_pULThruster->EnableEmission(false);
		}
	}

	///////////////////////////////////////////////////
	// Travel the landing gear AtomGroup:s

	if (m_pMThruster) {
		m_GearState = m_pMThruster->IsEmitting() ? LandingGearState::RAISED : LandingGearState::LOWERED;

		m_Paths[RIGHT][m_GearState].SetHFlip(m_HFlipped);
		m_Paths[LEFT][m_GearState].SetHFlip(!m_HFlipped);

		if (!m_LimbPushForcesAndCollisionsDisabled) {
			if (m_pRLeg) {
				m_pRFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pRLeg->GetParentOffset()), m_pRLeg->GetMaxLength(), m_Vel, m_Rotation, m_Paths[RIGHT][m_GearState], deltaTime, nullptr, true);
			}
			if (m_pLLeg) {
				m_pLFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pLLeg->GetParentOffset()), m_pLLeg->GetMaxLength(), m_Vel, m_Rotation, m_Paths[LEFT][m_GearState], deltaTime, nullptr, true);
			}
		} else {
			if (m_pRLeg) {
				m_pRFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pRLeg->GetParentOffset()), m_pRLeg->GetMaxLength(), m_Vel, m_Rotation, m_Paths[RIGHT][m_GearState], deltaTime, nullptr, true);
			}
			if (m_pLLeg) {
				m_pLFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pLLeg->GetParentOffset()), m_pLLeg->GetMaxLength(), m_Vel, m_Rotation, m_Paths[LEFT][m_GearState], deltaTime, nullptr, true);
			}
		}
	}

	/////////////////////////////////
	// Manage Attachable:s
	if (m_pRLeg && m_pRLeg->IsAttached()) {
		m_pRLeg->SetTargetPosition(m_pRFootGroup->GetLimbPos(m_HFlipped));
	}

	if (m_pLLeg && m_pLLeg->IsAttached()) {
		m_pLLeg->SetTargetPosition(m_pLFootGroup->GetLimbPos(!m_HFlipped));
	}
}

void ACRocket::Update() {
	ACraft::Update();

	////////////////////////////////////////
	// Hatch Operation

	unsigned int lastFrame = m_FrameCount - 1;

	if (m_HatchState == OPENING) {
		if (m_HatchTimer.GetElapsedSimTimeMS() <= m_HatchDelay && m_HatchDelay)
			m_Frame = static_cast<unsigned int>(static_cast<double>(lastFrame) * (m_HatchTimer.GetElapsedSimTimeMS() / static_cast<double>(m_HatchDelay)));
		else {
			m_Frame = lastFrame;
			m_HatchState = OPEN;
			DropAllInventory();
		}
	} else if (m_HatchState == CLOSING) {
		if (m_HatchTimer.GetElapsedSimTimeMS() <= m_HatchDelay && m_HatchDelay)
			m_Frame = lastFrame - static_cast<unsigned int>(static_cast<double>(lastFrame) * (m_HatchTimer.GetElapsedSimTimeMS() / static_cast<double>(m_HatchDelay)));
		else {
			m_Frame = 0;
			m_HatchState = CLOSED;
		}
	}
}

void ACRocket::SetRightLeg(Leg* newLeg) {
	if (m_pRLeg && m_pRLeg->IsAttached()) {
		RemoveAndDeleteAttachable(m_pRLeg);
	}
	if (newLeg == nullptr) {
		m_pRLeg = nullptr;
	} else {
		m_pRLeg = newLeg;
		AddAttachable(newLeg);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 Leg* castedAttachable = dynamic_cast<Leg*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetRightLeg");
			                                                 dynamic_cast<ACRocket*>(parent)->SetRightLeg(castedAttachable);
		                                                 }});

		if (m_pRLeg->HasNoSetDamageMultiplier()) {
			m_pRLeg->SetDamageMultiplier(1.0F);
		}
	}
}

void ACRocket::SetLeftLeg(Leg* newLeg) {
	if (m_pLLeg && m_pLLeg->IsAttached()) {
		RemoveAndDeleteAttachable(m_pLLeg);
	}
	if (newLeg == nullptr) {
		m_pLLeg = nullptr;
	} else {
		m_pLLeg = newLeg;
		AddAttachable(newLeg);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 Leg* castedAttachable = dynamic_cast<Leg*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetLeftLeg");
			                                                 dynamic_cast<ACRocket*>(parent)->SetLeftLeg(castedAttachable);
		                                                 }});

		if (m_pLLeg->HasNoSetDamageMultiplier()) {
			m_pLLeg->SetDamageMultiplier(1.0F);
		}
		m_pLLeg->SetInheritsHFlipped(-1);
	}
}

void ACRocket::SetMainThruster(AEmitter* newThruster) {
	if (m_pMThruster && m_pMThruster->IsAttached()) {
		RemoveAndDeleteAttachable(m_pMThruster);
	}
	if (newThruster == nullptr) {
		m_pMThruster = nullptr;
	} else {
		m_pMThruster = newThruster;
		AddAttachable(newThruster);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 AEmitter* castedAttachable = dynamic_cast<AEmitter*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetMainThruster");
			                                                 dynamic_cast<ACRocket*>(parent)->SetMainThruster(castedAttachable);
		                                                 }});

		if (m_pMThruster->HasNoSetDamageMultiplier()) {
			m_pMThruster->SetDamageMultiplier(1.0F);
		}
		m_pMThruster->SetInheritedRotAngleOffset(-c_HalfPI);
	}
}

void ACRocket::SetRightThruster(AEmitter* newThruster) {
	if (m_pRThruster && m_pRThruster->IsAttached()) {
		RemoveAndDeleteAttachable(m_pRThruster);
	}
	if (newThruster == nullptr) {
		m_pRThruster = nullptr;
	} else {
		m_pRThruster = newThruster;
		AddAttachable(newThruster);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 AEmitter* castedAttachable = dynamic_cast<AEmitter*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetRightThruster");
			                                                 dynamic_cast<ACRocket*>(parent)->SetRightThruster(castedAttachable);
		                                                 }});

		if (m_pRThruster->HasNoSetDamageMultiplier()) {
			m_pRThruster->SetDamageMultiplier(1.0F);
		}
		m_pRThruster->SetInheritedRotAngleOffset(c_EighthPI);
	}
}

void ACRocket::SetLeftThruster(AEmitter* newThruster) {
	if (m_pLThruster && m_pLThruster->IsAttached()) {
		RemoveAndDeleteAttachable(m_pLThruster);
	}
	if (newThruster == nullptr) {
		m_pLThruster = nullptr;
	} else {
		m_pLThruster = newThruster;
		AddAttachable(newThruster);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 AEmitter* castedAttachable = dynamic_cast<AEmitter*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetLeftThruster");
			                                                 dynamic_cast<ACRocket*>(parent)->SetLeftThruster(castedAttachable);
		                                                 }});

		if (m_pLThruster->HasNoSetDamageMultiplier()) {
			m_pLThruster->SetDamageMultiplier(1.0F);
		}
		m_pLThruster->SetInheritedRotAngleOffset(c_PI - c_EighthPI);
	}
}

void ACRocket::SetURightThruster(AEmitter* newThruster) {
	if (m_pURThruster && m_pURThruster->IsAttached()) {
		RemoveAndDeleteAttachable(m_pURThruster);
	}
	if (newThruster == nullptr) {
		m_pURThruster = nullptr;
	} else {
		m_pURThruster = newThruster;
		AddAttachable(newThruster);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 AEmitter* castedAttachable = dynamic_cast<AEmitter*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetURightThruster");
			                                                 dynamic_cast<ACRocket*>(parent)->SetURightThruster(castedAttachable);
		                                                 }});

		if (m_pURThruster->HasNoSetDamageMultiplier()) {
			m_pURThruster->SetDamageMultiplier(1.0F);
		}
		m_pURThruster->SetInheritedRotAngleOffset(c_HalfPI - c_EighthPI);
	}
}

void ACRocket::SetULeftThruster(AEmitter* newThruster) {
	if (m_pULThruster && m_pULThruster->IsAttached()) {
		RemoveAndDeleteAttachable(m_pULThruster);
	}
	if (newThruster == nullptr) {
		m_pULThruster = nullptr;
	} else {
		m_pULThruster = newThruster;
		AddAttachable(newThruster);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 AEmitter* castedAttachable = dynamic_cast<AEmitter*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetULeftThruster");
			                                                 dynamic_cast<ACRocket*>(parent)->SetULeftThruster(castedAttachable);
		                                                 }});

		if (m_pULThruster->HasNoSetDamageMultiplier()) {
			m_pULThruster->SetDamageMultiplier(1.0F);
		}
		m_pULThruster->SetInheritedRotAngleOffset(c_HalfPI + c_EighthPI);
	}
}

void ACRocket::Draw(BITMAP* pTargetBitmap, const Vector& targetPos, DrawMode mode, bool onlyPhysical) const {
	ACraft::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

	if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawHandAndFootGroupVisualizations()) {
		m_pRFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
		m_pLFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
	}
}
