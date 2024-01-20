#include "TDExplosive.h"

namespace RTE {

	ConcreteClassInfo(TDExplosive, ThrownDevice, 50);

	void TDExplosive::Clear() {
		m_IsAnimatedManually = false;
	}

	int TDExplosive::Create() {
		if (ThrownDevice::Create() < 0) {
			return -1;
		}

		if (IsInGroup("Bombs - Payloads")) {
			m_HUDVisible = false;
		}

		return 0;
	}

	int TDExplosive::Create(const TDExplosive& reference) {
		if (ThrownDevice::Create(reference) < 0) {
			return -1;
		}
		m_IsAnimatedManually = reference.m_IsAnimatedManually;

		return 0;
	}

	int TDExplosive::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return ThrownDevice::ReadProperty(propName, reader));

		// TODO: Consider removing DetonationSound as GibSound already exists and could be used in its place
		MatchProperty("DetonationSound", {
			if (!m_GibSound) {
				m_GibSound = new SoundContainer;
			}
			reader >> m_GibSound;
		});
		MatchProperty("IsAnimatedManually", { reader >> m_IsAnimatedManually; });

		EndPropertyList;
	}

	int TDExplosive::Save(Writer& writer) const {
		ThrownDevice::Save(writer);
		writer.NewProperty("IsAnimatedManually");
		writer << m_IsAnimatedManually;
		return 0;
	}

	void TDExplosive::Update() {
		ThrownDevice::Update();

		if (m_Activated) {
			// Display active frame if no animation mode has been defined
			if (!m_IsAnimatedManually && m_SpriteAnimMode == NOANIM && m_FrameCount > 1) {
				m_Frame = 1;
			}
			m_RestTimer.Reset();
			m_ToSettle = false;
		}
		if (m_Activated && m_ActivationTimer.GetElapsedSimTimeMS() >= m_TriggerDelay) {
			GibThis();
		}
	}

	void TDExplosive::DrawHUD(BITMAP* targetBitmap, const Vector& targetPos, int whichScreen, bool playerControlled) {
		if (m_HUDVisible && !m_Activated) {
			ThrownDevice::DrawHUD(targetBitmap, targetPos, whichScreen);
		}
	}
} // namespace RTE
