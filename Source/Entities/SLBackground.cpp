#include "SLBackground.h"
#include "FrameMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include <algorithm>

#include "raylib/raylib.h"
#include "raylib/rlgl.h"

using namespace RTE;

ConcreteClassInfo(SLBackground, StaticSceneLayer, 0);

SLBackground::SLBackground() {
	Clear();
}

SLBackground::~SLBackground() {
	Destroy(true);
}

void SLBackground::Clear() {
	m_Bitmaps.clear();
	m_FrameCount = 1;
	m_Frame = 0;
	m_SpriteAnimMode = SpriteAnimMode::NOANIM;
	m_SpriteAnimDuration = 1000;
	m_SpriteAnimIsReversingFrames = false;
	m_SpriteAnimTimer.Reset();
	m_IsAnimatedManually = false;
	m_CanAutoScrollX = false;
	m_CanAutoScrollY = false;
	m_AutoScrollStep.Reset();
	m_AutoScrollStepInterval = 0;
	m_AutoScrollStepTimer.Reset();
	m_AutoScrollOffset.Reset();
	m_FillColorLeft = ColorKeys::g_MaskColor;
	m_FillColorRight = ColorKeys::g_MaskColor;
	m_FillColorUp = ColorKeys::g_MaskColor;
	m_FillColorDown = ColorKeys::g_MaskColor;
	m_ZOrder = c_BackgroundDepth;

	m_IgnoreAutoScale = false;
}

int SLBackground::Create() {
	StaticSceneLayer::Create();

	m_Bitmaps.clear();
	m_BitmapFile.GetAsAnimation(m_Bitmaps, m_FrameCount);
	m_MainBitmap = m_Bitmaps[0];

	if (m_FrameCount == 1) {
		m_SpriteAnimMode = SpriteAnimMode::NOANIM;
	} else if (m_FrameCount == 2 && m_SpriteAnimMode != SpriteAnimMode::NOANIM) {
		m_SpriteAnimMode = SpriteAnimMode::ALWAYSLOOP;
	}

	if (!m_WrapX) {
		m_FillColorLeft = _getpixel(m_MainBitmap, 0, m_MainBitmap->h / 2);
		m_FillColorRight = _getpixel(m_MainBitmap, m_MainBitmap->w - 1, m_MainBitmap->h / 2);
	}
	if (!m_WrapY) {
		m_FillColorUp = _getpixel(m_MainBitmap, m_MainBitmap->w / 2, 0);
		m_FillColorDown = _getpixel(m_MainBitmap, m_MainBitmap->w / 2, m_MainBitmap->h - 1);
	}
	return 0;
}

int SLBackground::Create(const SLBackground& reference) {
	StaticSceneLayer::Create(reference);

	// The main bitmap is created and owned by SceneLayer because it can be modified. We need to destroy it to avoid a leak because the bitmaps we'll be using here are owned by ContentFile static maps and are unmodifiable.
	destroy_bitmap(m_MainBitmap);
	m_MainBitmapOwned = false;

	m_Bitmaps.clear();
	m_Bitmaps = reference.m_Bitmaps;
	m_MainBitmap = m_Bitmaps[0];

	m_FillColorLeft = reference.m_FillColorLeft;
	m_FillColorRight = reference.m_FillColorRight;
	m_FillColorUp = reference.m_FillColorUp;
	m_FillColorDown = reference.m_FillColorDown;

	m_FrameCount = reference.m_FrameCount;
	m_SpriteAnimMode = reference.m_SpriteAnimMode;
	m_SpriteAnimDuration = reference.m_SpriteAnimDuration;

	m_CanAutoScrollX = reference.m_CanAutoScrollX;
	m_CanAutoScrollY = reference.m_CanAutoScrollY;
	m_AutoScrollStep = reference.m_AutoScrollStep;
	m_AutoScrollStepInterval = reference.m_AutoScrollStepInterval;

	m_IgnoreAutoScale = reference.m_IgnoreAutoScale;

	return 0;
}

int SLBackground::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return StaticSceneLayer::ReadProperty(propName, reader));

	MatchProperty("FrameCount", { reader >> m_FrameCount; });
	MatchProperty("SpriteAnimMode", {
		m_SpriteAnimMode = static_cast<SpriteAnimMode>(std::stoi(reader.ReadPropValue()));
		if (m_SpriteAnimMode < SpriteAnimMode::NOANIM || m_SpriteAnimMode > SpriteAnimMode::ALWAYSPINGPONG) {
			reader.ReportError("Invalid SLBackground sprite animation mode!");
		}
	});
	MatchProperty("SpriteAnimDuration", { reader >> m_SpriteAnimDuration; });
	MatchProperty("IsAnimatedManually", { reader >> m_IsAnimatedManually; });
	MatchProperty("AutoScrollStepInterval", { reader >> m_AutoScrollStepInterval; });
	MatchProperty("AutoScrollStep", { reader >> m_AutoScrollStep; });
	MatchProperty("IgnoreAutoScaling", { reader >> m_IgnoreAutoScale; });
	MatchProperty("CanAutoScrollX", { reader >> m_CanAutoScrollX; });
	MatchProperty("CanAutoScrollY", { reader >> m_CanAutoScrollY; });

	EndPropertyList;
}
int SLBackground::Save(Writer& writer) const {
	StaticSceneLayer::Save(writer);

	writer.NewDistinctProperty("FrameCount", m_FrameCount, 1);
	writer.NewDistinctProperty("SpriteAnimMode", m_SpriteAnimMode, SpriteAnimMode::NOANIM);
	writer.NewDistinctProperty("SpriteAnimDuration", m_SpriteAnimDuration, 1000);
	writer.NewDistinctProperty("IsAnimatedManually", m_IsAnimatedManually, false);
	writer.NewDistinctProperty("AutoScrollStepInterval", m_AutoScrollStepInterval, 0);

	if (!m_AutoScrollStep.IsZero())
		writer.NewPropertyWithValue("AutoScrollStep", m_AutoScrollStep);

	writer.NewDistinctProperty("IgnoreAutoScaling", m_IgnoreAutoScale, false);
	writer.NewDistinctProperty("CanAutoScrollX", m_CanAutoScrollX, false);
	writer.NewDistinctProperty("CanAutoScrollY", m_CanAutoScrollY, false);

	return 0;
}

int SLBackground::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	StaticSceneLayer::Write(writer, entityReference, hashData);

	const SLBackground& reference = static_cast<const SLBackground&>(entityReference);

	writer.NewDistinctProperty("FrameCount", m_FrameCount, reference.m_FrameCount);
	writer.NewDistinctProperty("SpriteAnimMode", m_SpriteAnimMode, reference.m_SpriteAnimMode);
	writer.NewDistinctProperty("SpriteAnimDuration", m_SpriteAnimDuration, reference.m_SpriteAnimDuration);
	writer.NewDistinctProperty("IsAnimatedManually", m_IsAnimatedManually, reference.m_IsAnimatedManually);
	writer.NewDistinctProperty("AutoScrollStepInterval", m_AutoScrollStepInterval, reference.m_AutoScrollStepInterval);
	writer.NewDistinctProperty("AutoScrollStep", m_AutoScrollStep, reference.m_AutoScrollStep);
	writer.NewDistinctProperty("IgnoreAutoScaling", m_IgnoreAutoScale, reference.m_IgnoreAutoScale);
	writer.NewDistinctProperty("CanAutoScrollX", m_CanAutoScrollX, reference.m_CanAutoScrollX);
	writer.NewDistinctProperty("CanAutoScrollY", m_CanAutoScrollY, reference.m_CanAutoScrollY);

	return 0;
}

HashingData SLBackground::Hash() const {
	HashingData hashData = StaticSceneLayer::Hash();
	uint64_t& hash = hashData.m_Hash;

	hash ^= std::hash<int>{}(m_FrameCount) << 1;
	hash ^= std::hash<SpriteAnimMode>{}(m_SpriteAnimMode) << 2;
	hash ^= std::hash<int>{}(m_SpriteAnimDuration) << 3;
	hash ^= std::hash<bool>{}(m_IsAnimatedManually) << 4;
	hash ^= std::hash<int>{}(m_AutoScrollStepInterval) << 5;
	hash ^= m_AutoScrollStep.Hash().m_Hash << 6;
	hash ^= std::hash<bool>{}(m_CanAutoScrollX) << 7;
	hash ^= std::hash<bool>{}(m_CanAutoScrollY) << 8;

	return hashData;
}

void SLBackground::InitScaleFactors() {
	if (!m_IgnoreAutoScale) {
		float fitScreenScaleFactor = std::clamp(static_cast<float>(std::min(g_SceneMan.GetSceneHeight(), g_FrameMan.GetPlayerScreenHeight())) / static_cast<float>(m_MainBitmap->h), 1.0F, 2.0F);

		switch (g_SettingsMan.GetSceneBackgroundAutoScaleMode()) {
			case LayerAutoScaleMode::FitScreen:
				SetScaleFactor(Vector(fitScreenScaleFactor, fitScreenScaleFactor));
				break;
			case LayerAutoScaleMode::AlwaysUpscaled:
				SetScaleFactor(Vector(2.0F, 2.0F));
				break;
			default:
				SetScaleFactor(m_ScaleFactor);
				break;
		}
		m_ScrollInfo *= m_ScaleFactor;
		InitScrollRatios();
	}
}

void SLBackground::Update() {
	if (!m_IsAnimatedManually && m_SpriteAnimMode != SpriteAnimMode::NOANIM) {
		int prevFrame = m_Frame;

		if (m_SpriteAnimTimer.GetElapsedSimTimeMS() > (m_SpriteAnimDuration / m_FrameCount)) {
			switch (m_SpriteAnimMode) {
				case SpriteAnimMode::ALWAYSLOOP:
					m_Frame = (m_Frame + 1) % m_FrameCount;
					break;
				case SpriteAnimMode::ALWAYSRANDOM:
					while (m_Frame == prevFrame) {
						m_Frame = RandomNum(0, m_FrameCount - 1);
					}
					break;
				case SpriteAnimMode::ALWAYSPINGPONG:
					if (m_Frame == m_FrameCount - 1) {
						m_SpriteAnimIsReversingFrames = true;
					} else if (m_Frame == 0) {
						m_SpriteAnimIsReversingFrames = false;
					}
					m_SpriteAnimIsReversingFrames ? m_Frame-- : m_Frame++;
					break;
				default:
					break;
			}
			m_SpriteAnimTimer.Reset();
		}
	}
	m_MainBitmap = m_Bitmaps.at(m_Frame);

	if (IsAutoScrolling()) {
		if (m_AutoScrollStepTimer.GetElapsedSimTimeMS() > m_AutoScrollStepInterval) {
			if (m_WrapX && m_CanAutoScrollX) {
				m_AutoScrollOffset.SetX(m_AutoScrollOffset.GetX() + m_AutoScrollStep.GetX());
			}
			if (m_WrapY && m_CanAutoScrollY) {
				m_AutoScrollOffset.SetY(m_AutoScrollOffset.GetY() + m_AutoScrollStep.GetY());
			}
			WrapPosition(m_AutoScrollOffset);
			m_AutoScrollStepTimer.Reset();
		}
		m_Offset.SetXY(std::floor((m_Offset.GetX() * m_ScrollRatio.GetX()) + m_AutoScrollOffset.GetX()), std::floor((m_Offset.GetY() * m_ScrollRatio.GetY()) + m_AutoScrollOffset.GetY()));
	}
}

void SLBackground::Draw(const Box& targetDimensions, Box& targetBox, bool offsetNeedsScrollRatioAdjustment) {
	StaticSceneLayer::Draw(targetDimensions, targetBox, !IsAutoScrolling());

	int bitmapWidth = m_ScaledDimensions.GetFloorIntX();
	int bitmapHeight = m_ScaledDimensions.GetFloorIntY();
	int targetBoxCornerX = targetBox.GetCorner().GetFloorIntX();
	int targetBoxCornerY = targetBox.GetCorner().GetFloorIntY();
	int targetBoxWidth = static_cast<int>(targetBox.GetWidth());
	int targetBoxHeight = static_cast<int>(targetBox.GetHeight());

	rlZDepth(m_ZOrder);
	// Detect if non-wrapping layer dimensions can't cover the whole target area with its main bitmap. If so, fill in the gap with appropriate solid color sampled from the hanging edge.
	if (!m_WrapX && bitmapWidth <= targetBoxWidth) {
		if (m_FillColorLeft != ColorKeys::g_MaskColor && m_Offset.GetFloorIntX() != 0) {
			DrawRectangle(targetBoxCornerX, targetBoxCornerY, -m_Offset.m_X, targetBoxHeight, {static_cast<unsigned char>(m_FillColorLeft), 0, 0, 255});
		}
		if (m_FillColorRight != ColorKeys::g_MaskColor) {
			DrawRectangle(targetBoxCornerX + bitmapWidth - m_Offset.m_X, targetBoxCornerY, targetBoxWidth - bitmapWidth + m_Offset.m_X, targetBoxHeight, {static_cast<unsigned char>(m_FillColorRight), 0, 0, 255});
		}
	}
	if (!m_WrapY && bitmapHeight <= targetBoxHeight) {
		if (m_FillColorUp != ColorKeys::g_MaskColor && m_Offset.GetFloorIntY() != 0) {
			DrawRectangle(targetBoxCornerX, targetBoxCornerY, targetBoxWidth, - m_Offset.m_Y, {static_cast<unsigned char>(m_FillColorUp), 0, 0, 255});
		}
		if (m_FillColorDown != ColorKeys::g_MaskColor) {
			DrawRectangle(targetBoxCornerX, targetBoxCornerY + bitmapHeight - m_Offset.m_Y, targetBoxWidth, targetBoxHeight - bitmapHeight + m_Offset.m_Y, {static_cast<unsigned char>(m_FillColorDown), 0, 0, 255});
		}
	}
	rlZDepth(c_DefaultDrawDepth);
}
