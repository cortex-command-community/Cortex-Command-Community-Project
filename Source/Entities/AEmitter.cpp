#include "AEmitter.h"
#include "Atom.h"
#include "Emission.h"
#include "PresetMan.h"
#include "SoundContainer.h"
#include "PostProcessMan.h"

using namespace RTE;

ConcreteClassInfo(AEmitter, Attachable, 100);

AEmitter::AEmitter() {
	Clear();
}

AEmitter::~AEmitter() {
	Destroy(true);
}

void AEmitter::Clear() {
	m_EmissionList.clear();
	m_EmissionSound = nullptr;
	m_BurstSound = nullptr;
	m_EndSound = nullptr;
	m_EmitEnabled = false;
	m_WasEmitting = false;
	m_EmitCount = 0;
	m_EmitCountLimit = 0;
	m_NegativeThrottleMultiplier = 1.0F;
	m_PositiveThrottleMultiplier = 1.0F;
	m_Throttle = 0;
	m_EmissionsIgnoreThis = false;
	m_BurstScale = 1.0F;
	m_BurstDamage = 0;
	m_EmitterDamageMultiplier = 1.0F;
	m_BurstTriggered = false;
	m_BurstSpacing = 0;
	// Set this to really long so an initial burst will be possible
	m_BurstTimer.SetElapsedSimTimeS(50000);
	m_BurstTimer.SetElapsedRealTimeS(50000);
	m_PlayBurstSound = true;
	m_EmitAngle.Reset();
	m_EmissionOffset.Reset();
	m_EmitDamage = 0;
	m_LastEmitTmr.Reset();
	m_pFlash = 0;
	m_FlashScale = 1.0F;
	m_AvgBurstImpulse = -1.0F;
	m_AvgImpulse = -1.0F;
	m_FlashOnlyOnBurst = true;
	m_SustainBurstSound = false;
	m_BurstSoundFollowsEmitter = true;
	m_LoudnessOnEmit = 1.0F;
}

int AEmitter::Create(const AEmitter& reference) {
	if (reference.m_pFlash) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFlash->GetUniqueID());
	}

	Attachable::Create(reference);

	if (reference.m_pFlash) {
		SetFlash(dynamic_cast<Attachable*>(reference.m_pFlash->Clone()));
	}

	for (Emission* emission: reference.m_EmissionList) {
		m_EmissionList.push_back(static_cast<Emission*>(emission->Clone()));
	}
	if (reference.m_EmissionSound) {
		m_EmissionSound = dynamic_cast<SoundContainer*>(reference.m_EmissionSound->Clone());
	}
	if (reference.m_BurstSound) {
		m_BurstSound = dynamic_cast<SoundContainer*>(reference.m_BurstSound->Clone());
	}
	if (reference.m_EndSound) {
		m_EndSound = dynamic_cast<SoundContainer*>(reference.m_EndSound->Clone());
	}
	m_EmitEnabled = reference.m_EmitEnabled;
	m_EmitCount = reference.m_EmitCount;
	m_EmitCountLimit = reference.m_EmitCountLimit;
	m_NegativeThrottleMultiplier = reference.m_NegativeThrottleMultiplier;
	m_PositiveThrottleMultiplier = reference.m_PositiveThrottleMultiplier;
	m_Throttle = reference.m_Throttle;
	m_EmissionsIgnoreThis = reference.m_EmissionsIgnoreThis;
	m_BurstScale = reference.m_BurstScale;
	m_BurstDamage = reference.m_BurstDamage;
	m_EmitterDamageMultiplier = reference.m_EmitterDamageMultiplier;
	m_BurstSpacing = reference.m_BurstSpacing;
	m_BurstTriggered = reference.m_BurstTriggered;
	m_PlayBurstSound = reference.m_PlayBurstSound;
	m_EmitAngle = reference.m_EmitAngle;
	m_EmissionOffset = reference.m_EmissionOffset;
	m_EmitDamage = reference.m_EmitDamage;
	m_FlashScale = reference.m_FlashScale;
	m_FlashOnlyOnBurst = reference.m_FlashOnlyOnBurst;
	m_SustainBurstSound = reference.m_SustainBurstSound;
	m_BurstSoundFollowsEmitter = reference.m_BurstSoundFollowsEmitter;
	m_LoudnessOnEmit = reference.m_LoudnessOnEmit;

	return 0;
}

int AEmitter::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Attachable::ReadProperty(propName, reader));

	MatchProperty("_ClearEmissions", {
		reader.ReadPropValue();
		for (Emission* emission: m_EmissionList) {
			delete emission;
		}
		m_EmissionList.clear();
	});
	MatchForwards("AddEmission") MatchProperty("_AddEmission", {
		Emission* emission = new Emission();
		reader >> *emission;
		m_EmissionList.push_back(emission);
	});
	MatchProperty("EmissionSound", {
		m_EmissionSound = new SoundContainer;
		reader >> m_EmissionSound;
	});
	MatchProperty("BurstSound", {
		m_BurstSound = new SoundContainer;
		reader >> m_BurstSound;
	});
	MatchProperty("EndSound", {
		m_EndSound = new SoundContainer;
		reader >> m_EndSound;
	});
	MatchProperty("EmissionEnabled", { reader >> m_EmitEnabled; });
	MatchProperty("EmissionCount", { reader >> m_EmitCount; });
	MatchProperty("EmissionCountLimit", { reader >> m_EmitCountLimit; });
	MatchProperty("ParticlesPerMinute", {
		float ppm;
		reader >> ppm;
		// Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility.
		for (Emission* emission: m_EmissionList) {
			emission->m_PPM = ppm / static_cast<float>(m_EmissionList.size());
		}
	});
	MatchProperty("NegativeThrottleMultiplier", { reader >> m_NegativeThrottleMultiplier; });
	MatchProperty("PositiveThrottleMultiplier", { reader >> m_PositiveThrottleMultiplier; });
	MatchProperty("Throttle", { reader >> m_Throttle; });
	MatchProperty("EmissionsIgnoreThis", { reader >> m_EmissionsIgnoreThis; });
	MatchProperty("BurstSize", {
		int burstSize;
		reader >> burstSize;
		// Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility.
		for (Emission* emission: m_EmissionList) {
			emission->m_BurstSize = std::ceil(static_cast<float>(burstSize) / static_cast<float>(m_EmissionList.size()));
		}
	});
	MatchProperty("BurstScale", { reader >> m_BurstScale; });
	MatchProperty("BurstDamage", { reader >> m_BurstDamage; });
	MatchProperty("EmitterDamageMultiplier", { reader >> m_EmitterDamageMultiplier; });
	MatchProperty("BurstSpacing", { reader >> m_BurstSpacing; });
	MatchProperty("BurstTriggered", { reader >> m_BurstTriggered; });
	MatchProperty("PlayBurstSound", { reader >> m_PlayBurstSound; });
	MatchProperty("EmissionAngle", { reader >> m_EmitAngle; });
	MatchProperty("EmissionOffset", { reader >> m_EmissionOffset; });
	MatchProperty("EmissionDamage", { reader >> m_EmitDamage; });
	MatchProperty("Flash", { SetFlash(dynamic_cast<Attachable*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("FlashScale", { reader >> m_FlashScale; });
	MatchProperty("FlashOnlyOnBurst", { reader >> m_FlashOnlyOnBurst; });
	MatchProperty("SustainBurstSound", { reader >> m_SustainBurstSound; });
	MatchProperty("BurstSoundFollowsEmitter", { reader >> m_BurstSoundFollowsEmitter; });
	MatchProperty("LoudnessOnEmit", { reader >> m_LoudnessOnEmit; });

	EndPropertyList;
}

int AEmitter::Save(Writer& writer) const {
	Attachable::Save(writer);

	for (Emission* emission: m_EmissionList) {
		writer.NewProperty("AddEmission");
		writer << *emission;
	}
	writer.NewPropertyWithValue("EmissionSound", m_EmissionSound);
	writer.NewPropertyWithValue("BurstSound", m_BurstSound);
	writer.NewPropertyWithValue("EndSound", m_EndSound);
	writer.NewPropertyWithValue("EmissionEnabled", m_EmitEnabled);
	writer.NewPropertyWithValue("EmissionCount", m_EmitCount);
	writer.NewPropertyWithValue("EmissionCountLimit", m_EmitCountLimit);
	writer.NewPropertyWithValue("EmissionsIgnoreThis", m_EmissionsIgnoreThis);
	writer.NewPropertyWithValue("NegativeThrottleMultiplier", m_NegativeThrottleMultiplier);
	writer.NewPropertyWithValue("PositiveThrottleMultiplier", m_PositiveThrottleMultiplier);
	writer.NewPropertyWithValue("Throttle", m_Throttle);
	writer.NewPropertyWithValue("BurstScale", m_BurstScale);
	writer.NewPropertyWithValue("BurstDamage", m_BurstDamage);
	writer.NewPropertyWithValue("EmitterDamageMultiplier", m_EmitterDamageMultiplier);
	writer.NewPropertyWithValue("BurstSpacing", m_BurstSpacing);
	writer.NewPropertyWithValue("BurstTriggered", m_BurstTriggered);
	writer.NewPropertyWithValue("PlayBurstSound", m_PlayBurstSound);
	writer.NewPropertyWithValue("EmissionAngle", m_EmitAngle);
	writer.NewPropertyWithValue("EmissionOffset", m_EmissionOffset);
	writer.NewPropertyWithValue("EmissionDamage", m_EmitDamage);
	writer.NewPropertyWithValue("Flash", m_pFlash);
	writer.NewPropertyWithValue("FlashScale", m_FlashScale);
	writer.NewPropertyWithValue("FlashOnlyOnBurst", m_FlashOnlyOnBurst);
	writer.NewPropertyWithValue("SustainBurstSound", m_SustainBurstSound);
	writer.NewPropertyWithValue("BurstSoundFollowsEmitter", m_BurstSoundFollowsEmitter);
	writer.NewPropertyWithValue("LoudnessOnEmit", m_LoudnessOnEmit);

	return 0;
}

int AEmitter::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Attachable::Write(writer, entityReference, hashData);

	const AEmitter& reference = static_cast<const AEmitter&>(entityReference);

	writer.NewPointerSequence("_ClearEmissions", "_AddEmission", m_EmissionList, hashData);
	writer.NewOptionalEntityPointerProperty("EmissionSound", m_EmissionSound, hashData);
	writer.NewOptionalEntityPointerProperty("BurstSound", m_BurstSound, hashData);
	writer.NewOptionalEntityPointerProperty("EndSound", m_EndSound, hashData);
	writer.NewDistinctProperty("EmissionEnabled", m_EmitEnabled, reference.m_EmitEnabled);
	writer.NewDistinctProperty("EmissionCount", m_EmitCount, reference.m_EmitCount);
	writer.NewDistinctProperty("EmissionCountLimit", m_EmitCountLimit, reference.m_EmitCountLimit);
	writer.NewDistinctProperty("EmissionsIgnoreThis", m_EmissionsIgnoreThis, reference.m_EmissionsIgnoreThis);
	writer.NewDistinctProperty("NegativeThrottleMultiplier", m_NegativeThrottleMultiplier, reference.m_NegativeThrottleMultiplier);
	writer.NewDistinctProperty("PositiveThrottleMultiplier", m_PositiveThrottleMultiplier, reference.m_PositiveThrottleMultiplier);
	writer.NewDistinctProperty("Throttle", m_Throttle, reference.m_Throttle);
	writer.NewDistinctProperty("BurstScale", m_BurstScale, reference.m_BurstScale);
	writer.NewDistinctProperty("BurstDamage", m_BurstDamage, reference.m_BurstDamage);
	writer.NewDistinctProperty("EmitterDamageMultiplier", m_EmitterDamageMultiplier, reference.m_EmitterDamageMultiplier);
	writer.NewDistinctProperty("BurstSpacing", m_BurstSpacing, reference.m_BurstSpacing);
	writer.NewDistinctProperty("BurstTriggered", m_BurstTriggered, reference.m_BurstTriggered);
	writer.NewDistinctProperty("PlayBurstSound", m_PlayBurstSound, reference.m_PlayBurstSound);
	writer.NewDistinctProperty("EmissionAngle", m_EmitAngle, reference.m_EmitAngle);
	writer.NewDistinctProperty("EmissionOffset", m_EmissionOffset, reference.m_EmissionOffset);
	writer.NewDistinctProperty("EmissionDamage", m_EmitDamage, reference.m_EmitDamage);
	writer.NewOptionalEntityPointerProperty("Flash", m_pFlash, hashData);
	writer.NewDistinctProperty("FlashScale", m_FlashScale, reference.m_FlashScale);
	writer.NewDistinctProperty("FlashOnlyOnBurst", m_FlashOnlyOnBurst, reference.m_FlashOnlyOnBurst);
	writer.NewDistinctProperty("SustainBurstSound", m_SustainBurstSound, reference.m_SustainBurstSound);
	writer.NewDistinctProperty("BurstSoundFollowsEmitter", m_BurstSoundFollowsEmitter, reference.m_BurstSoundFollowsEmitter);
	writer.NewDistinctProperty("LoudnessOnEmit", m_LoudnessOnEmit, reference.m_LoudnessOnEmit);

	return 0;
}

HashingData AEmitter::Hash() const {
	HashingData hashData(Attachable::Hash());
	uint64_t& hash = hashData.m_Hash;

	int i = 0;

	for (Emission* emission: m_EmissionList) {
		uint64_t emissionHash = emission->Hash().m_Hash;
		hashData.m_Constituents.push_back(emissionHash);
		hash ^= emissionHash << (i++ % sizeof(uint64_t) * 8);
	}

	hashData.m_ParseValues.push_back(i);

	bool emissionSoundDef = m_EmissionSound != nullptr;
	hashData.m_ParseValues.push_back(emissionSoundDef);
	if (emissionSoundDef) {
		uint64_t emissionSoundHash = m_EmissionSound->Hash().m_Hash;
		hashData.m_Constituents.push_back(emissionSoundHash);
		hash ^= emissionSoundHash << 1;
	}

	bool burstSoundDef = m_BurstSound != nullptr;
	hashData.m_ParseValues.push_back(burstSoundDef);
	if (burstSoundDef) {
		uint64_t burstSoundHash = m_BurstSound->Hash().m_Hash;
		hashData.m_Constituents.push_back(burstSoundHash);
		hash ^= burstSoundHash << 2;
	}

	bool endSoundDef = m_EndSound != nullptr;
	hashData.m_ParseValues.push_back(endSoundDef);
	if (endSoundDef) {
		uint64_t endSoundHash = m_EndSound->Hash().m_Hash;
		hashData.m_Constituents.push_back(endSoundHash);
		hash ^= endSoundHash << 3;
	}

	hash ^= static_cast<uint64_t>(m_EmitEnabled) << 4;
	hash ^= static_cast<uint64_t>(m_EmitCount) << 5;
	hash ^= static_cast<uint64_t>(m_EmitCountLimit) << 6;
	hash ^= static_cast<uint64_t>(m_EmissionsIgnoreThis) << 7;
	hash ^= static_cast<uint64_t>(m_NegativeThrottleMultiplier) << 8;
	hash ^= static_cast<uint64_t>(m_PositiveThrottleMultiplier) << 9;
	hash ^= static_cast<uint64_t>(m_Throttle) << 10;
	hash ^= static_cast<uint64_t>(m_BurstScale) << 11;
	hash ^= static_cast<uint64_t>(m_BurstDamage) << 12;
	hash ^= static_cast<uint64_t>(m_EmitterDamageMultiplier) << 13;
	hash ^= static_cast<uint64_t>(m_BurstSpacing) << 14;
	hash ^= static_cast<uint64_t>(m_BurstTriggered) << 15;
	hash ^= static_cast<uint64_t>(m_PlayBurstSound) << 0;
	hash ^= m_EmitAngle.Hash().m_Hash << 1;
	hash ^= m_EmissionOffset.Hash().m_Hash << 2;
	hash ^= static_cast<uint64_t>(m_EmitDamage) << 3;

	bool flashDef = m_pFlash != nullptr;
	hashData.m_ParseValues.push_back(flashDef);
	if (flashDef) {
		uint64_t flashHash = m_pFlash->Hash().m_Hash;
		hashData.m_Constituents.push_back(flashHash);
		hash ^= flashHash << 4;
	}

	hash ^= static_cast<uint64_t>(m_FlashScale) << 5;
	hash ^= static_cast<uint64_t>(m_FlashOnlyOnBurst) << 6;
	hash ^= static_cast<uint64_t>(m_SustainBurstSound) << 7;
	hash ^= static_cast<uint64_t>(m_BurstSoundFollowsEmitter) << 8;
	hash ^= static_cast<uint64_t>(m_LoudnessOnEmit) << 9;

	return hashData;
}

void AEmitter::Destroy(bool notInherited) {
	// Stop playback of sounds gracefully
	if (m_EmissionSound) {
		if (m_EndSound) {
			m_EmissionSound->IsBeingPlayed() ? m_EndSound->Play(m_Pos) : m_EndSound->Stop();
		}
		m_EmissionSound->Stop();
	}

	for (Emission* emission: m_EmissionList) {
		delete emission;
	}

	delete m_EmissionSound;
	delete m_BurstSound;
	delete m_EndSound;

	//    m_BurstSound.Stop();

	if (!notInherited) {
		Attachable::Destroy();
	}
	Clear();
}

void AEmitter::ResetEmissionTimers() {
	m_LastEmitTmr.Reset();
	for (Emission* emission: m_EmissionList) {
		emission->ResetEmissionTimers();
	}
}

void AEmitter::EnableEmission(bool enable) {
	if (!m_EmitEnabled && enable) {
		m_LastEmitTmr.Reset();
		// Reset counter
		m_EmitCount = 0;
		// Reset animation
		m_Frame = 0;
	}
	m_EmitEnabled = enable;
}

float AEmitter::EstimateImpulse(bool burst) {
	// Calculate the impulse generated by the emissions, once and store the result
	if ((!burst && m_AvgImpulse < 0) || (burst && m_AvgBurstImpulse < 0)) {
		float impulse = 0.0F;

		// Go through all emissions and emit them according to their respective rates
		for (Emission* emission: m_EmissionList) {
			// Only check emissions that push the emitter
			if (emission->PushesEmitter()) {
				// TODO: we're not checking emission start/stop times here, so this will always calculate the impulse as if the emission was active.
				// There's not really an easy way to do this, since the emission rate is not necessarily constant over time.

				// TODO: burst emissions shouldn't be affected by delta time, but they were.
				// However our values were tuned for 60hz, so hack in constant 60Hz deltatime in milliseconds.
				float deltaTimeSecs = burst ? 1.0f / 60.0f : g_TimerMan.GetDeltaTimeSecs();

				float emissions = (emission->GetRate() / 60.0f) * deltaTimeSecs;
				float scale = 1.0F;
				if (burst) {
					emissions *= emission->GetBurstSize();
					scale = m_BurstScale;
				}

				if (emissions > 0) {
				    int extraEmissions = emission->GetParticleCount() - 1;
				    emissions += extraEmissions;
			    }

				float velMin = emission->GetMinVelocity() * scale;
			    float velRange = (emission->GetMaxVelocity() - emission->GetMinVelocity()) * scale * 0.5f;
			    float spread = (std::max(static_cast<float>(c_PI) - (emission->GetSpread() * scale), 0.0F) / c_PI); // A large spread will cause the forces to cancel eachother out

				// Add to accumulative recoil impulse generated, F = m * a.
				impulse += (velMin + velRange) * spread * emission->m_pEmission->GetMass() * emissions;
			}
		}

		if (burst) {
			m_AvgBurstImpulse = impulse;
		} else {
			m_AvgImpulse = impulse;
		}
	}

	// Scale the emission rate up or down according to the appropriate throttle multiplier.
	float throttleFactor = GetThrottleFactor();
	// Apply the throttle factor to the emission rate per update
	if (burst) {
		return m_AvgBurstImpulse * throttleFactor;
	}

	return m_AvgImpulse * throttleFactor;
}

float AEmitter::GetTotalParticlesPerMinute() const {
	float totalPPM = 0;
	for (const Emission* emission: m_EmissionList) {
		totalPPM += emission->m_PPM;
	}
	return totalPPM;
}

int AEmitter::GetTotalBurstSize() const {
	int totalBurstSize = 0;
	for (const Emission* emission: m_EmissionList) {
		totalBurstSize += emission->m_BurstSize;
	}
	return totalBurstSize;
}

float AEmitter::GetScaledThrottle(float throttle, float multiplier) const {
	float throttleFactor = Lerp(-1.0f, 1.0f, m_NegativeThrottleMultiplier, m_PositiveThrottleMultiplier, throttle);
	return Lerp(m_NegativeThrottleMultiplier, m_PositiveThrottleMultiplier, -1.0f, 1.0f, throttleFactor * multiplier);
}

void AEmitter::SetFlash(Attachable* newFlash) {
	if (m_pFlash && m_pFlash->IsAttached()) {
		RemoveAndDeleteAttachable(m_pFlash);
	}
	if (newFlash == nullptr) {
		m_pFlash = nullptr;
	} else {
		// Note - this is done here because setting mass on attached Attachables causes values to be updated on the parent (and its parent, and so on), which isn't ideal. Better to do it before the new flash is attached, so there are fewer calculations.
		newFlash->SetMass(0.0F);

		m_pFlash = newFlash;
		AddAttachable(newFlash);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newFlash->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 dynamic_cast<AEmitter*>(parent)->SetFlash(attachable);
		                                                 }});

		m_pFlash->SetDrawnNormallyByParent(false);
		m_pFlash->SetInheritsRotAngle(false);
		m_pFlash->SetDeleteWhenRemovedFromParent(true);
		m_pFlash->SetCollidesWithTerrainWhileAttached(false);
	}
}

void AEmitter::Update() {
	Attachable::PreUpdate();

	if (m_FrameCount > 1) {
		if (m_EmitEnabled && m_SpriteAnimMode == NOANIM) {
			m_SpriteAnimMode = ALWAYSLOOP;
		} else if (!m_EmitEnabled) {
			m_SpriteAnimMode = NOANIM;
			m_Frame = 0;
		}
	}

	// Update and show flash if there is one
	if (m_pFlash && (!m_FlashOnlyOnBurst || m_BurstTriggered)) {
		m_pFlash->SetParentOffset(m_EmissionOffset);
		m_pFlash->SetRotAngle(m_Rotation.GetRadAngle() + (m_EmitAngle.GetRadAngle() * GetFlipFactor()));
		m_pFlash->SetScale(m_FlashScale);
		m_pFlash->SetNextFrame();
	}

	Attachable::Update();

	if (m_BurstSoundFollowsEmitter && m_BurstSound) {
		m_BurstSound->SetPosition(m_Pos);
	}

	if (m_EmitEnabled) {
		if (!m_WasEmitting) {
			// Start playing the sound
			if (m_EmissionSound) {
				m_EmissionSound->Play(m_Pos);
			}

			// Reset the timers of all emissions so they will start/stop at the correct relative offsets from now
			for (Emission* emission: m_EmissionList)
				emission->ResetEmissionTimers();
		}
		// Update the distance attenuation
		else if (m_EmissionSound) {
			m_EmissionSound->SetPosition(m_Pos);
		}

		// Get the parent root of this AEmitter
		// TODO: Potentially get this once outside instead, like in attach/detach")
		MovableObject* pRootParent = GetRootParent();

		float throttleFactor = GetThrottleFactor();
		m_FlashScale = throttleFactor;
		// Check burst triggering against whether the spacing is fulfilled
		if (m_BurstTriggered && CanTriggerBurst()) {
			// Play burst sound
			if (m_BurstSound && m_PlayBurstSound) {
				m_BurstSound->Play(m_Pos);
			}
			// Start timing until next burst
			m_BurstTimer.Reset();
		}
		else {
			// Not enough spacing, cancel the triggering if there was any
			m_BurstTriggered = false;
		}

		int emissionCountTotal = 0;
		float velMin, velRange, spread;
		double currentPPM, SPE;
		MovableObject* pParticle = 0;
		Vector parentVel, emitVel, pushImpulses;
		// Go through all emissions and emit them according to their respective rates
		for (Emission* emission: m_EmissionList) {
			// Make sure the emissions only happen between the start time and end time
			if (emission->IsEmissionTime()) {
				// Apply the throttle factor to the emission rate
				currentPPM = emission->GetRate() * throttleFactor;
				int emissionCount = 0;

				// Only do all this if the PPM is actually above zero
				if (currentPPM > 0) {
					// Calculate secs per emission
					SPE = 60.0 / currentPPM;

					// Add the last elapsed time to the accumulator
					emission->m_Accumulator += m_LastEmitTmr.GetElapsedSimTimeS();

					// Now figure how many full emissions can fit in the current accumulator
					emissionCount = std::floor(emission->m_Accumulator / SPE);
					// Deduct the about to be emitted emissions from the accumulator
					emission->m_Accumulator -= emissionCount * SPE;

					RTEAssert(emission->m_Accumulator >= 0, "Emission accumulator negative!");
				} else {
					emission->m_Accumulator = 0;
				}
				float scale = 1.0F;
				// Add extra emissions if bursting.
				if (m_BurstTriggered) {
					emissionCount += emission->GetBurstSize();
					scale = m_BurstScale;
				}
				emissionCountTotal += emissionCount;
				if (emissionCount > 0) {
					int extraEmissions = emission->GetParticleCount() - 1;
					emissionCount += extraEmissions;
				}
				pParticle = 0;
				emitVel.Reset();
				parentVel = pRootParent->GetVel() * emission->InheritsVelocity();
				Vector rotationalVel = (((RotateOffset(emission->GetOffset()) + (m_Pos - pRootParent->GetPos())) * pRootParent->GetAngularVel()).GetPerpendicular() / c_PPM) * emission->InheritsVelocity();

				for (int i = 0; i < emissionCount; ++i) {
					velMin = emission->GetMinVelocity() * scale;
					velRange = (emission->GetMaxVelocity() - emission->GetMinVelocity()) * scale;
					spread = emission->GetSpread() * scale;
					// Make a copy after the reference particle
					pParticle = dynamic_cast<MovableObject*>(emission->GetEmissionParticlePreset()->Clone());
					// Set up its position and velocity according to the parameters of this.
					// Emission point offset not set

					if (emission->GetOffset().IsZero()) {
						if (m_EmissionOffset.IsZero()) {
							pParticle->SetPos(m_Pos);
						} else {
							pParticle->SetPos(m_Pos + RotateOffset(m_EmissionOffset));
						}
					} else {
						pParticle->SetPos(m_Pos + RotateOffset(emission->GetOffset()));
					}
					// TODO: Optimize making the random angles!")
					emitVel.SetXY(velMin + RandomNum(0.0F, velRange), 0.0F);
					emitVel.RadRotate(m_EmitAngle.GetRadAngle() + spread * RandomNormalNum());
					emitVel = RotateOffset(emitVel);
					pParticle->SetVel(parentVel + rotationalVel + emitVel);
					pParticle->SetRotAngle(emitVel.GetAbsRadAngle() + (m_HFlipped ? -c_PI : 0));
					pParticle->SetAngularVel(pRootParent->GetAngularVel() * emission->InheritsAngularVelocity());
					pParticle->SetHFlipped(m_HFlipped);

					// Scale the particle's lifetime based on life variation and throttle, as long as it's not 0
					if (pParticle->GetLifetime() != 0) {
						pParticle->SetLifetime(std::max(static_cast<int>(static_cast<float>(pParticle->GetLifetime()) * (1.0F + (emission->GetLifeVariation() * RandomNormalNum()))), 1));
						pParticle->SetLifetime(std::max(static_cast<int>(pParticle->GetLifetime() * throttleFactor), 1));
					}
					pParticle->SetTeam(m_Team);
					pParticle->SetIgnoresTeamHits(true);

					// Add to accumulative recoil impulse generated, F = m * a
					// If enabled, that is
					if (emission->PushesEmitter() && (GetParent() || GetMass() > 0)) {
						pushImpulses -= emitVel * pParticle->GetMass();
					}

					// Set the emitted particle to not hit this emitter's parent, if applicable
					if (m_EmissionsIgnoreThis)
						pParticle->SetWhichMOToNotHit(pRootParent);

					// Let particle loose into the world!
					g_MovableMan.AddMO(pParticle);
					pParticle = 0;
				}
			}
		}
		m_LastEmitTmr.Reset();

		// Apply recoil/push effects. Joint stiffness will take effect when these are transferred to the parent.
		static bool enableFakedImpulse = false; // useful for debugging the accuracy of EstimateImpulse();
		if (enableFakedImpulse) {
			Vector fakeImpulse;
			fakeImpulse.SetXY(-EstimateImpulse(m_BurstTriggered), 0.0F);
			fakeImpulse.RadRotate(m_EmitAngle.GetRadAngle());
			fakeImpulse = RotateOffset(fakeImpulse);
			AddImpulseForce(fakeImpulse);
		} else {
			AddImpulseForce(pushImpulses);
		}

		// Count the the damage caused by the emissions, and only if we're not bursting
		if (!m_BurstTriggered) {
			m_DamageCount += static_cast<float>(emissionCountTotal) * m_EmitDamage * m_EmitterDamageMultiplier;
		} else { // Count the the damage caused by the burst
			m_DamageCount += m_BurstDamage * m_EmitterDamageMultiplier;
		}

		// Count the total emissions since enabling, and stop emitting if beyond limit (and limit is also enabled)
		m_EmitCount += emissionCountTotal;
		if (m_EmitCountLimit > 0 && m_EmitCount > m_EmitCountLimit) {
			EnableEmission(false);
		}

		if (m_BurstTriggered) {
			m_BurstTriggered = false;
		}

		m_WasEmitting = true;
	}
	// Do stuff to stop emission
	else {
		if (m_WasEmitting) {
			if (m_EmissionSound) {
				m_EmissionSound->Stop();
			}
			if (m_BurstSound && !m_SustainBurstSound) {
				m_BurstSound->Stop();
			}
			if (m_EndSound) {
				m_EndSound->Play(m_Pos);
			}
			m_WasEmitting = false;
		}
	}

	// Set the screen flash effect to draw at the final post processing stage
	if (m_EmitEnabled && (!m_FlashOnlyOnBurst || m_BurstTriggered) && m_pFlash && m_pFlash->GetScreenEffect()) {
		// Fudge the glow pos forward a bit so it aligns nicely with the flash
		Vector emitPos(m_pFlash->GetScreenEffect()->w * 0.3F * m_FlashScale, 0);
		emitPos.RadRotate(m_HFlipped ? c_PI + m_Rotation.GetRadAngle() - m_EmitAngle.GetRadAngle() : m_Rotation.GetRadAngle() + m_EmitAngle.GetRadAngle());
		emitPos = m_Pos + RotateOffset(m_EmissionOffset) + emitPos;
		if (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(emitPos)) {
			g_PostProcessMan.RegisterPostEffect(emitPos, m_pFlash->GetScreenEffect(), m_pFlash->GetScreenEffectHash(), RandomNum(m_pFlash->GetEffectStopStrength(), m_pFlash->GetEffectStartStrength()) * std::clamp(m_FlashScale, 0.0F, 1.0F), m_pFlash->GetEffectRotAngle());
		}
	}
}

void AEmitter::Draw(BITMAP* pTargetBitmap,
                    const Vector& targetPos,
                    DrawMode mode,
                    bool onlyPhysical) const {
	// Draw flash if there is one
	if (m_pFlash && !m_pFlash->IsDrawnAfterParent() &&
	    !onlyPhysical && mode == g_DrawColor && m_EmitEnabled && (!m_FlashOnlyOnBurst || m_BurstTriggered))
		m_pFlash->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

	Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

	// Update and Draw flash if there is one
	if (m_pFlash && m_pFlash->IsDrawnAfterParent() &&
	    !onlyPhysical && mode == g_DrawColor && m_EmitEnabled && (!m_FlashOnlyOnBurst || m_BurstTriggered))
		m_pFlash->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}
