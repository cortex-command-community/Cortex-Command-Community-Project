#include "SceneLayer.h"

#include "FrameMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "ActivityMan.h"
#include "ThreadMan.h"
#include "GLResourceMan.h"
#include "BigTexture.h"

#include "Draw.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include <array>
#include <cmath>

using namespace RTE;

ConcreteClassInfo(SceneLayerTracked, Entity, 0);
ConcreteClassInfo(SceneLayer, Entity, 0);
ConcreteClassInfo(StaticSceneLayer, Entity, 0);

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::SceneLayerImpl() {
	Clear();
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::~SceneLayerImpl() {
	Destroy(true);
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::Clear() {
	m_BitmapFile.Reset();
	m_MainBitmap = nullptr;
	m_BackBitmap = nullptr;
	m_MainTexture.reset();
	m_LastClearColor = ColorKeys::g_InvalidColor;
	m_Drawings.clear();
	m_MainBitmapOwned = false;
	m_DrawMasked = true;
	m_WrapX = true;
	m_WrapY = true;
	m_OriginOffset.Reset();
	m_Offset.Reset();
	m_ZOrder = 0.0F;
	m_ScrollInfo.SetXY(1.0F, 1.0F);
	m_ScrollRatio.SetXY(1.0F, 1.0F);
	m_ScaleFactor.SetXY(1.0F, 1.0F);
	m_ScaledDimensions.SetXY(1.0F, 1.0F);
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::Create(const ContentFile& bitmapFile, bool drawMasked, const Vector& offset, bool wrapX, bool wrapY, const Vector& scrollInfo) {
	m_BitmapFile = bitmapFile;
	m_MainBitmap = m_BitmapFile.GetAsBitmap();
	Create(m_MainBitmap, drawMasked, offset, wrapX, wrapY, scrollInfo);

	m_MainBitmapOwned = false;

	return 0;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::Create(BITMAP* bitmap, bool drawMasked, const Vector& offset, bool wrapX, bool wrapY, const Vector& scrollInfo) {
	m_MainBitmap = bitmap;
	RTEAssert(m_MainBitmap, "Null bitmap passed in when creating SceneLayerImpl!");

	m_MainBitmapOwned = true;

	m_BackBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
	m_LastClearColor = ColorKeys::g_InvalidColor;
	if constexpr (!STATIC_TEXTURE) {
		m_MainTexture = std::make_unique<BigTexture>(m_MainBitmap);
	}

	m_DrawMasked = drawMasked;
	m_Offset = offset;
	m_WrapX = wrapX;
	m_WrapY = wrapY;
	m_ScrollInfo = scrollInfo;

	InitScrollRatios();

	return 0;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::Create(const SceneLayerImpl& reference) {
	Entity::Create(reference);

	m_BitmapFile = reference.m_BitmapFile;
	m_DrawMasked = reference.m_DrawMasked;
	m_WrapX = reference.m_WrapX;
	m_WrapY = reference.m_WrapY;
	m_OriginOffset = reference.m_OriginOffset;
	m_ScrollInfo = reference.m_ScrollInfo;
	m_ScrollRatio = reference.m_ScrollRatio;
	m_ScaleFactor = reference.m_ScaleFactor;
	m_ScaledDimensions = reference.m_ScaledDimensions;

	if (reference.m_MainBitmap) {
		// Make a copy of the bitmap because it can be modified in some use cases.
		BITMAP* bitmapToCopy = reference.m_MainBitmap;
		RTEAssert(bitmapToCopy, "Couldn't load the bitmap file specified for SceneLayerImpl!");

		m_MainBitmap = create_bitmap_ex(bitmap_color_depth(bitmapToCopy), bitmapToCopy->w, bitmapToCopy->h);
		RTEAssert(m_MainBitmap, "Failed to allocate BITMAP in SceneLayerImpl::Create");
		blit(bitmapToCopy, m_MainBitmap, 0, 0, 0, 0, bitmapToCopy->w, bitmapToCopy->h);

		m_BackBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
		m_LastClearColor = ColorKeys::g_InvalidColor;

		if constexpr (!STATIC_TEXTURE) {
			m_MainTexture = std::make_unique<BigTexture>(m_MainBitmap);
		}

		InitScrollRatios();

		m_MainBitmapOwned = true;
	} else {
		// If no bitmap to copy, then it has to be loaded with LoadData.
		m_MainBitmapOwned = false;
	}
	return 0;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("WrapX", { reader >> m_WrapX; });
	MatchProperty("WrapY", { reader >> m_WrapY; });
	MatchProperty("BitmapFile", { reader >> m_BitmapFile; });

	EndPropertyList;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewPropertyWithValue("WrapX", m_WrapX);
	writer.NewPropertyWithValue("WrapY", m_WrapY);
	writer.NewPropertyWithValue("BitmapFile", m_BitmapFile);

	return 0;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::Destroy(bool notInherited) {
	if (m_MainBitmapOwned) {
		destroy_bitmap(m_MainBitmap);
	}
	if (m_BackBitmap) {
		destroy_bitmap(m_BackBitmap);
	}
	if (!notInherited) {
		Entity::Destroy();
	}
	Clear();
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::InitScrollRatios(bool initForNetworkPlayer, int player) {
	float mainBitmapWidth = static_cast<float>(m_MainBitmap->w);
	float mainBitmapHeight = static_cast<float>(m_MainBitmap->h);
	float playerScreenWidth = static_cast<float>(initForNetworkPlayer ? g_FrameMan.GetPlayerFrameBufferWidth(player) : g_FrameMan.GetPlayerScreenWidth());
	float playerScreenHeight = static_cast<float>(initForNetworkPlayer ? g_FrameMan.GetPlayerFrameBufferHeight(player) : g_FrameMan.GetPlayerScreenHeight());

	if (m_WrapX) {
		m_ScrollRatio.SetX(m_ScrollInfo.GetX());
	} else {
		if (m_ScrollInfo.GetX() == -1.0F || m_ScrollInfo.GetX() == 1.0F) {
			m_ScrollRatio.SetX(1.0F);
		} else if (m_ScrollInfo.GetX() == playerScreenWidth) {
			m_ScrollRatio.SetX(mainBitmapWidth - playerScreenWidth);
		} else if (mainBitmapWidth == playerScreenWidth) {
			m_ScrollRatio.SetX(1.0F / (m_ScrollInfo.GetX() - playerScreenWidth));
		} else {
			m_ScrollRatio.SetX((mainBitmapWidth - playerScreenWidth) / (m_ScrollInfo.GetX() - playerScreenWidth));
		}
	}
	if (m_WrapY) {
		m_ScrollRatio.SetY(m_ScrollInfo.GetY());
	} else {
		if (m_ScrollInfo.GetY() == -1.0F || m_ScrollInfo.GetY() == 1.0) {
			m_ScrollRatio.SetY(1.0F);
		} else if (m_ScrollInfo.GetY() == playerScreenHeight) {
			m_ScrollRatio.SetY(mainBitmapHeight - playerScreenHeight);
		} else if (mainBitmapHeight == playerScreenHeight) {
			m_ScrollRatio.SetY(1.0F / (m_ScrollInfo.GetY() - playerScreenHeight));
		} else {
			m_ScrollRatio.SetY((mainBitmapHeight - playerScreenHeight) / (m_ScrollInfo.GetY() - playerScreenHeight));
		}
	}
	m_ScaledDimensions.SetXY(mainBitmapWidth * m_ScaleFactor.GetX(), mainBitmapHeight * m_ScaleFactor.GetY());
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::LoadData() {
	if (m_MainBitmapOwned) {
		destroy_bitmap(m_MainBitmap);
		m_MainBitmap = nullptr;
	}

	// Load from disk and take ownership. Don't cache because the bitmap will be modified.
	m_MainBitmap = m_BitmapFile.GetAsBitmap(COLORCONV_NONE, false);
	m_MainBitmapOwned = true;

	m_BackBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
	if constexpr (!STATIC_TEXTURE) {
		m_MainTexture = std::make_unique<BigTexture>(m_MainBitmap);
	}
	m_LastClearColor = ColorKeys::g_InvalidColor;

	InitScrollRatios();
	return 0;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::SaveData(const std::string& bitmapPath) {
	if (bitmapPath.empty()) {
		return -1;
	}

	if (m_MainBitmap) {
		// Make a copy of the bitmap to pass to the thread because the bitmap may be offloaded mid thread and everything will be on fire.
		BITMAP* outputBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
		blit(m_MainBitmap, outputBitmap, 0, 0, 0, 0, m_MainBitmap->w, m_MainBitmap->h);

		m_BitmapFile.SetDataPath(bitmapPath);

		PALETTE palette;
		get_palette(palette);
		if (save_png(bitmapPath.c_str(), outputBitmap, palette) != 0) {
			RTEAbort(std::string("Failed to save SceneLayerImpl bitmap to path and name: " + bitmapPath));
		}
		destroy_bitmap(outputBitmap);
	}
	return 0;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
std::unique_ptr<BITMAP> SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::CopyBitmap() const {
	BITMAP* outputBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
	if (m_MainBitmap) {
		outputBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
		blit(m_MainBitmap, outputBitmap, 0, 0, 0, 0, m_MainBitmap->w, m_MainBitmap->h);
	}
	return std::unique_ptr<BITMAP>(outputBitmap);
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::ClearData() {
	if (m_MainBitmap && m_MainBitmapOwned) {
		destroy_bitmap(m_MainBitmap);
	}
	m_MainBitmap = nullptr;
	m_MainTexture.reset();
	m_MainBitmapOwned = false;

	if (m_BackBitmap) {
		destroy_bitmap(m_BackBitmap);
	}
	m_BackBitmap = nullptr;
	m_LastClearColor = ColorKeys::g_InvalidColor;

	return 0;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::SetScaleFactor(const Vector& newScale) {
	m_ScaleFactor = newScale;
	if (m_MainBitmap) {
		m_ScaledDimensions.SetXY(static_cast<float>(m_MainBitmap->w) * newScale.GetX(), static_cast<float>(m_MainBitmap->h) * newScale.GetY());
	}
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
int SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::GetPixel(int pixelX, int pixelY) const {
	WrapPosition(pixelX, pixelY);
	return (pixelX < 0 || pixelX >= m_MainBitmap->w || pixelY < 0 || pixelY >= m_MainBitmap->h) ? MaterialColorKeys::g_MaterialAir : _getpixel(m_MainBitmap, pixelX, pixelY);
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::SetPixel(int pixelX, int pixelY, int materialID) {
	RTEAssert(m_MainBitmapOwned, "Trying to set a pixel of a SceneLayer's bitmap which isn't owned!");

	WrapPosition(pixelX, pixelY);

	if (pixelX < 0 || pixelX >= m_MainBitmap->w || pixelY < 0 || pixelY >= m_MainBitmap->h) {
		return;
	}
	_putpixel(m_MainBitmap, pixelX, pixelY, materialID);

	RegisterDrawing(pixelX, pixelY, pixelX, pixelY);
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
bool SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::IsWithinBounds(const int pixelX, const int pixelY, const int margin) const {
	return (m_WrapX || (pixelX >= -margin && pixelX < m_MainBitmap->w + margin)) && (m_WrapY || (pixelY >= -margin && pixelY < m_MainBitmap->h + margin));
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::ClearBitmap(ColorKeys clearTo) {
	RTEAssert(m_MainBitmapOwned, "Bitmap not owned! We shouldn't be clearing this!");

	if (m_BitmapClearTask.valid()) {
		m_BitmapClearTask.wait();
	}

	if (m_LastClearColor != clearTo) {
		// Note: We're clearing to a different color than expected, which is expensive! We should always aim to clear to the same color to avoid it as much as possible.
		clear_to_color(m_BackBitmap, clearTo);
		m_LastClearColor = clearTo;
	}

	std::swap(m_MainBitmap, m_BackBitmap);

	// Start a new thread to clear the backbuffer bitmap asynchronously.
	m_BitmapClearTask = g_ThreadMan.GetPriorityThreadPool().submit([this, clearTo](BITMAP* bitmap, std::vector<IntRect> drawings) {
		ZoneScopedN("Clear Tracked Backbuffer");
		ClearDrawings(bitmap, drawings, clearTo);
	}, m_BackBitmap, m_Drawings);

	m_Drawings.clear(); // This was copied into the new thread, so can be safely deleted.
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
bool SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::WrapPosition(int& posX, int& posY) const {
	int oldX = posX;
	int oldY = posY;

	if (m_WrapX) {
		int width = m_ScaledDimensions.GetFloorIntX();
		posX %= width;
		if (posX < 0) {
			posX += width;
		}
	}

	if (m_WrapY) {
		int height = m_ScaledDimensions.GetFloorIntY();
		posY %= height;
		if (posY < 0) {
			posY += height;
		}
	}

	return oldX != posX || oldY != posY;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
bool SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::ForceBounds(int& posX, int& posY) const {
	bool wrapped = false;
	int width = m_ScaledDimensions.GetFloorIntX();
	int height = m_ScaledDimensions.GetFloorIntY();

	if (posX < 0) {
		if (m_WrapX) {
			while (posX < 0) {
				posX += width;
			}
			wrapped = true;
		} else {
			posX = 0;
		}
	}
	if (posY < 0) {
		if (m_WrapY) {
			while (posY < 0) {
				posY += height;
			}
			wrapped = true;
		} else {
			posY = 0;
		}
	}
	if (posX >= width) {
		if (m_WrapX) {
			posX %= width;
			wrapped = true;
		} else {
			posX = width - 1;
		}
	}
	if (posY >= height) {
		if (m_WrapY) {
			posY %= height;
			wrapped = true;
		} else {
			posY = height - 1;
		}
	}
	return wrapped;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
bool SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::ForceBoundsOrWrapPosition(Vector& pos, bool forceBounds) const {
	int posX = pos.GetFloorIntX();
	int posY = pos.GetFloorIntY();
	bool wrapped = forceBounds ? ForceBounds(posX, posY) : WrapPosition(posX, posY);
	pos.SetXY(static_cast<float>(posX) + (pos.GetX() - std::floor(pos.GetX())), static_cast<float>(posY) + (pos.GetY() - std::floor(pos.GetY())));

	return wrapped;
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::RegisterDrawing(int left, int top, int right, int bottom) {
	m_MainBitmapUpdated = true;
	if constexpr (TRACK_DRAWINGS) {
		m_Drawings.emplace_back(left, top, right, bottom);
	}
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::RegisterDrawing(const Vector& center, float radius) {
	if (radius != 0.0F) {
		RegisterDrawing(static_cast<int>(center.GetX() - radius), static_cast<int>(center.GetY() - radius), static_cast<int>(center.GetX() + radius), static_cast<int>(center.GetY() + radius));
	}
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::UpdateTargetRegion(const Box& targetBox) {
	if constexpr (TRACK_DRAWINGS) {
		m_MainTexture->m_Bitmap = m_MainBitmap;
	}
	if constexpr (!STATIC_TEXTURE) {
		RTEAssert(bitmap_color_depth(m_MainBitmap) == 8, "Truecolor scenelayer used for non gpu drawing!");
		std::vector<Box> updateRegions{};
		float bitmapWidth = m_MainBitmap->w;
		float bitmapHeight = m_MainBitmap->h;
		int areaToCoverX = (m_Offset.GetFloorIntX() + targetBox.GetCorner().GetFloorIntX() + targetBox.GetWidth()) / m_ScaleFactor.m_X;
		int areaToCoverY = (m_Offset.GetFloorIntY() + targetBox.GetCorner().GetFloorIntY() + targetBox.GetHeight()) / m_ScaleFactor.m_Y;
		Box scaledTarget(targetBox.m_Corner / m_ScaleFactor, targetBox.m_Width / m_ScaleFactor.m_X, targetBox.m_Height / m_ScaleFactor.m_Y);
		Vector scaledOffset(m_Offset/m_ScaleFactor);
		Box bitmapDimensions(Vector(), bitmapWidth, bitmapHeight);

		for (int tiledOffsetX = 0; tiledOffsetX < areaToCoverX;) {
			float destX = tiledOffsetX - scaledOffset.GetFloorIntX();

			for (int tiledOffsetY = 0; tiledOffsetY < areaToCoverY;) {
				float destY = tiledOffsetY - scaledOffset.GetFloorIntY();
				Box update = bitmapDimensions.GetIntersection({-Vector(destX, destY), scaledTarget.m_Width, scaledTarget.m_Height});
				update.m_Corner = update.m_Corner.GetFloored();
				update.m_Width = std::ceil(update.m_Width) + 1;
				update.m_Height = std::ceil(update.m_Height) + 1;
				updateRegions.emplace_back(update);
				if (!m_WrapY) {
					break;
				}
				tiledOffsetY += bitmapHeight;
			}
			if (!m_WrapX) {
				break;
			}
			tiledOffsetX += bitmapWidth;
		}

		for (auto& region: updateRegions) {
			m_MainTexture->Update(region);
		}
		// g_GLResourceMan.UpdateDynamicBitmap(m_MainBitmap, true, updateRegions);

	} else {}
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::Draw(const Box& targetDimensions, Box& targetBox, bool offsetNeedsScrollRatioAdjustment) {
	RTEAssert(m_MainBitmap, "Data of this SceneLayerImpl has not been loaded before trying to draw!");
	if constexpr(!STATIC_TEXTURE) {
		RTEAssert(m_MainTexture, "Texture of this SceneLayerImpl has not bee created before trying to draw!");
	}
	ZoneScoped;
	TracyGpuZone("SceneLayer::Draw");
	if (offsetNeedsScrollRatioAdjustment) {
		m_Offset.SetXY(std::floor(m_Offset.GetX() * m_ScrollRatio.GetX()), std::floor(m_Offset.GetY() * m_ScrollRatio.GetY()));
	}
	if (targetBox.IsEmpty()) {
		targetBox = targetDimensions;
	}
	if (!m_WrapX && targetDimensions.GetWidth() > targetBox.GetWidth()) {
		m_Offset.SetX(0);
	}
	if (!m_WrapY && targetDimensions.GetHeight() > targetBox.GetHeight()) {
		m_Offset.SetY(0);
	}

	m_Offset -= m_OriginOffset;
	WrapPosition(m_Offset);
	if constexpr (!STATIC_TEXTURE) {
		if (m_MainBitmapOwned) {
			UpdateTargetRegion(targetBox);
		}
	}
	m_MainBitmapUpdated = false;

	bool drawScaled = m_ScaleFactor.GetX() > 1.0F || m_ScaleFactor.GetY() > 1.0F;

	DrawTiled(targetDimensions, targetBox, drawScaled);
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::DrawTiled(const Box& targetDimensions, const Box& targetBox, bool drawScaled) const {
	ZoneScoped;
	TracyGpuZone("SceneLayer::DrawTiled");
	float bitmapWidth = m_ScaledDimensions.m_X;
	float bitmapHeight = m_ScaledDimensions.m_Y;
	int areaToCoverX = m_Offset.GetFloorIntX() + targetBox.GetCorner().GetFloorIntX() + std::min(targetDimensions.GetWidth(), targetBox.GetWidth());
	int areaToCoverY = m_Offset.GetFloorIntY() + targetBox.GetCorner().GetFloorIntY() + std::min(targetDimensions.GetHeight(), targetBox.GetHeight());

	if (!m_DrawMasked) {
		rlDrawRenderBatchActive();
		int maskedUniformLocation = rlGetLocationUniform(rlGetShaderCurrent(), "drawMasked");
		rlEnableShader(rlGetShaderCurrent());
		glUniform1i(maskedUniformLocation, 0);
	}

	rlZDepth(m_ZOrder);

	for (int tiledOffsetX = 0; tiledOffsetX < areaToCoverX;) {
		float destX = targetBox.GetCorner().GetFloorIntX() + tiledOffsetX - m_Offset.GetFloorIntX();

		for (int tiledOffsetY = 0; tiledOffsetY < areaToCoverY;) {
			float destY = targetBox.GetCorner().GetFloorIntY() + tiledOffsetY - m_Offset.GetFloorIntY();
			if constexpr (STATIC_TEXTURE) {
				DrawTexturePro(
				    g_GLResourceMan.GetStaticTextureFromBitmap(m_MainBitmap),
				    {0.0f, 0.0f, static_cast<float>(m_MainBitmap->w), static_cast<float>(m_MainBitmap->h)},
				    {destX, destY, bitmapWidth, bitmapHeight},
				    {0.0f, 0.0f}, 0.0f, {255, 255, 255, 255});
			} else {
				m_MainTexture->Draw(
				    {0.0f, 0.0f, static_cast<float>(m_MainBitmap->w), static_cast<float>(m_MainBitmap->h)},
				    {destX, destY, bitmapWidth, bitmapHeight});
			}
			if (!m_WrapY) {
				break;
			}
			tiledOffsetY += bitmapHeight;
		}
		if (!m_WrapX) {
			break;
		}
		tiledOffsetX += bitmapWidth;
	}

	rlZDepth(c_DefaultDrawDepth);

	if (!m_DrawMasked) {
		rlDrawRenderBatchActive();
		int drawMaskedUniform = rlGetLocationUniform(rlGetShaderCurrent(), "drawMasked");
		rlEnableShader(rlGetShaderCurrent());
		glUniform1i(drawMaskedUniform, 1);
	}
}

template <bool TRACK_DRAWINGS, bool STATIC_TEXTURE>
void SceneLayerImpl<TRACK_DRAWINGS, STATIC_TEXTURE>::ClearDrawings(BITMAP* bitmap, const std::vector<IntRect>& drawings, ColorKeys clearTo) const {
	if constexpr (TRACK_DRAWINGS) {
		for (const IntRect& rect: drawings) {
			int left = rect.m_Left;
			int top = rect.m_Top;
			int bottom = rect.m_Bottom;
			int right = rect.m_Right;

			rectfill(bitmap, left, top, right, bottom, clearTo);

			if (m_WrapX) {
				if (left < 0) {
					int wrapLeft = left + bitmap->w;
					int wrapRight = bitmap->w - 1;
					rectfill(bitmap, wrapLeft, top, wrapRight, bottom, clearTo);
				}

				if (right >= bitmap->w) {
					int wrapLeft = 0;
					int wrapRight = right - bitmap->w;
					rectfill(bitmap, wrapLeft, top, wrapRight, bottom, clearTo);
				}
			}

			if (m_WrapY) {
				if (top < 0) {
					int wrapTop = top + bitmap->h;
					int wrapBottom = bitmap->h - 1;
					rectfill(bitmap, left, wrapTop, right, wrapBottom, clearTo);
				}

				if (bottom >= bitmap->h) {
					int wrapTop = 0;
					int wrapBottom = bottom - bitmap->h;
					rectfill(bitmap, left, wrapTop, right, wrapBottom, clearTo);
				}
			}
		}
	} else {
		clear_to_color(bitmap, clearTo);
	}
}

// Force instantiation
template class RTE::SceneLayerImpl<false>;
template class RTE::SceneLayerImpl<false, true>;
template class RTE::SceneLayerImpl<true>;
