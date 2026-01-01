#include "ContentFile.h"

#include "AudioMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"
#include "RTETools.h"
#include "System.h"
#include "FrameMan.h"

#include "png.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include <SDL3_image/SDL_image.h>

#include <array>
#include <cstring>

using namespace RTE;

const std::string ContentFile::c_ClassName = "ContentFile";

std::array<std::unordered_map<std::string, BITMAP*>, ContentFile::BitDepths::BitDepthCount> ContentFile::s_LoadedBitmaps;
std::unordered_map<std::string, SDL_Surface*> ContentFile::s_MemoryPNGs;
std::unordered_map<std::string, FMOD::Sound*> ContentFile::s_LoadedSamples;
std::unordered_map<size_t, std::string> ContentFile::s_PathHashes;

void ContentFile::Clear() {
	m_DataPath.clear();
	m_DataPathExtension.clear();
	m_DataPathWithoutExtension.clear();
	m_DataPathIsImageFile = false;
	m_FormattedReaderPosition.clear();
	m_DataPathAndReaderPosition.clear();
	m_DataModuleID = 0;
	m_IsMemoryPNG = false;

	m_ImageFileInfo.fill(-1);
}

int ContentFile::Create(const char* filePath) {
	SetDataPath(filePath);

	return 0;
}

int ContentFile::Create(const ContentFile& reference) {
	m_DataPath = reference.m_DataPath;
	m_DataPathExtension = reference.m_DataPathExtension;
	m_DataPathWithoutExtension = reference.m_DataPathWithoutExtension;
	m_DataModuleID = reference.m_DataModuleID;

	return 0;
}

void ContentFile::FreeAllLoaded() {
	for (int depth = BitDepths::Eight; depth < BitDepths::BitDepthCount; ++depth) {
		for (const auto& [bitmapPath, bitmapPtr]: s_LoadedBitmaps[depth]) {
			destroy_bitmap(bitmapPtr);
		}
	}
}

int ContentFile::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchForwards("FilePath")
	MatchProperty("Path", { SetDataPath(reader.ReadPropValue()); });
	MatchProperty("IsMemoryPNG", { reader >> m_IsMemoryPNG; });

	EndPropertyList;
}

int ContentFile::Save(Writer& writer) const {
	Serializable::Save(writer);

	if (!m_DataPath.empty()) {
		writer.NewPropertyWithValue("FilePath", m_DataPath);
	}
	writer.NewPropertyWithValue("IsMemoryPNG", m_IsMemoryPNG);

	return 0;
}

int ContentFile::GetDataModuleID() const {
	return (m_DataModuleID < 0) ? g_PresetMan.GetModuleIDFromPath(m_DataPath) : m_DataModuleID;
}

void ContentFile::SetDataPath(const std::string& newDataPath) {
	m_DataPath = g_PresetMan.GetFullModulePath(newDataPath);
	m_DataPathExtension = std::filesystem::path(m_DataPath).extension().string();

	RTEAssert(!m_DataPathExtension.empty(), "Failed to find file extension when trying to find file with path and name:\n" + m_DataPath + "\n" + GetFormattedReaderPosition());

	m_DataPathIsImageFile = m_DataPathExtension == ".png" || m_DataPathExtension == ".bmp";

	m_DataPathWithoutExtension = m_DataPath.substr(0, m_DataPath.length() - m_DataPathExtension.length());
	s_PathHashes[GetHash()] = m_DataPath;
	m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);
}

size_t ContentFile::GetHash() const {
	return Hash(m_DataPath);
}

void ContentFile::SetFormattedReaderPosition(const std::string& newPosition) {
	m_FormattedReaderPosition = newPosition;
	m_DataPathAndReaderPosition = m_DataPath + "\n" + newPosition;
}

int ContentFile::GetImageFileInfo(ImageFileInfoType infoTypeToGet) {
	bool fetchFileInfo = false;
	for (const int& fileInfoEntry: m_ImageFileInfo) {
		if (fileInfoEntry == -1) {
			fetchFileInfo = true;
			break;
		}
	}
	if (fetchFileInfo) {
		if (m_IsMemoryPNG) {
			if (const SDL_Surface* png = s_MemoryPNGs[m_DataPath]) {
				m_ImageFileInfo[ImageFileInfoType::ImageBitDepth] = static_cast<int>(SDL_BITSPERPIXEL(png->format));
				m_ImageFileInfo[ImageFileInfoType::ImageWidth] = static_cast<int>(png->w);
				m_ImageFileInfo[ImageFileInfoType::ImageHeight] = static_cast<int>(png->h);
				return m_ImageFileInfo[infoTypeToGet];
			}
		}

		FILE* imageFile = fopen(m_DataPath.c_str(), "rb");
		RTEAssert(imageFile, "Failed to open file prior to reading info of image file with following path and name:\n\n" + m_DataPath + "\n\nThe file may not exist or be corrupt.");

		if (m_DataPathExtension == ".png") {
			ReadAndStorePNGFileInfo(imageFile);
		} else if (m_DataPathExtension == ".bmp") {
			ReadAndStoreBMPFileInfo(imageFile);
		} else {
			RTEAbort("Somehow ended up attempting to read image file info for an unsupported image file type.\nThe file path and name were:\n\n" + m_DataPath);
		}
		fclose(imageFile);
	}
	return m_ImageFileInfo[infoTypeToGet];
}

void ContentFile::ReadAndStorePNGFileInfo(FILE* imageFile) {
	std::array<uint8_t, 8> fileSignature = {};

	// Read the first 8 bytes to then verify they match the PNG file signature which is { 137, 80, 78, 71, 13, 10, 26, 10 } or { '\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n' }.
	fread(fileSignature.data(), sizeof(uint8_t), fileSignature.size(), imageFile);
	if (png_sig_cmp(fileSignature.data(), 0, fileSignature.size()) == 0) {
		png_structp pngReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		png_infop pngInfo = png_create_info_struct(pngReadStruct);

		png_init_io(pngReadStruct, imageFile);
		// Set the PNG reader to skip the first 8 bytes since we already handled them.
		png_set_sig_bytes(pngReadStruct, fileSignature.size());
		png_read_info(pngReadStruct, pngInfo);

		m_ImageFileInfo[ImageFileInfoType::ImageBitDepth] = static_cast<int>(png_get_bit_depth(pngReadStruct, pngInfo));
		m_ImageFileInfo[ImageFileInfoType::ImageWidth] = static_cast<int>(png_get_image_width(pngReadStruct, pngInfo));
		m_ImageFileInfo[ImageFileInfoType::ImageHeight] = static_cast<int>(png_get_image_height(pngReadStruct, pngInfo));

		png_destroy_read_struct(&pngReadStruct, &pngInfo, nullptr);
	} else {
		RTEAbort("Encountered invalid PNG file signature while attempting to read info of image file with following path and name:\n\n" + m_DataPath + "\n\nThe file may be corrupt or is not a PNG file.");
	}
}

void ContentFile::ReadAndStoreBMPFileInfo(FILE* imageFile) {
	std::array<uint8_t, 2> bmpSignature = {0x42, 0x4D}; // { 'B', 'M' }.
	std::array<uint8_t, 2> fileSignature = {};

	// Read the first 2 bytes to then verify they match the BMP file signature.
	fread(fileSignature.data(), sizeof(uint8_t), fileSignature.size(), imageFile);
	if (fileSignature == bmpSignature) {
		std::array<uint8_t, 4> bmpData = {};

		const auto toInt32 = [](uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
			return (static_cast<int32_t>(b0)) | (static_cast<int32_t>(b1) << 8) | (static_cast<int32_t>(b2) << 16) | (static_cast<int32_t>(b3) << 24);
		};

		// Skip the next 16 bytes. Dimensions data starts at the 18th byte.
		fseek(imageFile, 16, SEEK_CUR);

		fread(bmpData.data(), sizeof(uint8_t), bmpData.size(), imageFile);
		m_ImageFileInfo[ImageFileInfoType::ImageWidth] = toInt32(bmpData[0], bmpData[1], bmpData[2], bmpData[3]);

		fread(bmpData.data(), sizeof(uint8_t), bmpData.size(), imageFile);
		m_ImageFileInfo[ImageFileInfoType::ImageHeight] = toInt32(bmpData[0], bmpData[1], bmpData[2], bmpData[3]);

		// Skip the next 2 bytes. Bit depth data starts at the 28th byte.
		fseek(imageFile, 2, SEEK_CUR);

		// Bit depth is stored as 2 bytes, so ignore the last 2 in the array when converting to int32.
		fread(bmpData.data(), sizeof(uint8_t), bmpData.size(), imageFile);
		m_ImageFileInfo[ImageFileInfoType::ImageBitDepth] = toInt32(bmpData[0], bmpData[1], 0, 0);
	} else {
		RTEAbort("Encountered invalid BMP file signature while attempting to read info of image file with following path and name:\n\n" + m_DataPath + "\n\nThe file may be corrupt or is not a BMP file.");
	}
}

void ContentFile::ManuallyLoadDataPNG(const std::string& filePath, SDL_Surface* surface) {
	s_MemoryPNGs[filePath] = surface;

	int bitDepth = SDL_GetPixelFormatDetails(surface->format)->bits_per_pixel;
	BITMAP* bitmap = create_bitmap_ex(bitDepth, surface->w, surface->h);

	// Allegro doesn't align lines, SDL does 4byte alignment
	for (int y = 0; y < surface->h; y++) {
		memcpy(bitmap->line[y],
		       static_cast<unsigned char*>(surface->pixels) + surface->pitch * y,
		       surface->w * SDL_BYTESPERPIXEL(surface->format));
	}

	s_LoadedBitmaps[BitDepths::Eight].try_emplace(filePath, bitmap);
}

void ContentFile::ReloadAllBitmaps() {
	for (const std::unordered_map<std::string, BITMAP*>& bitmapCache: s_LoadedBitmaps) {
		for (const auto& [filePath, oldBitmap]: bitmapCache) {
			ReloadBitmap(filePath);
		}
	}
	g_ConsoleMan.PrintString("SYSTEM: Sprites reloaded");
}

BITMAP* ContentFile::GetAsBitmap(int conversionMode, bool storeBitmap, const std::string& dataPathToSpecificFrame) {
	if (m_DataPath.empty()) {
		return nullptr;
	}
	BITMAP* returnBitmap = nullptr;
	const int bitDepth = conversionMode == COLORCONV_8_TO_32 ? BitDepths::ThirtyTwo : BitDepths::Eight;
	std::string dataPathToLoad = dataPathToSpecificFrame.empty() ? m_DataPath : dataPathToSpecificFrame;

	if (g_PresetMan.GetReloadEntityPresetCalledThisUpdate()) {
		ReloadBitmap(dataPathToLoad, conversionMode);
	}

	// Check if the file has already been read and loaded from the disk and, if so, use that data.
	std::unordered_map<std::string, BITMAP*>::iterator foundBitmap = s_LoadedBitmaps[bitDepth].find(dataPathToLoad);
	if (foundBitmap != s_LoadedBitmaps[bitDepth].end()) {
		if (storeBitmap) {
			returnBitmap = (*foundBitmap).second;
		} else if (SDL_Surface* surface = s_MemoryPNGs[dataPathToLoad]) {
			std::unordered_map<std::string, BITMAP*>::iterator foundBitmap = s_LoadedBitmaps[BitDepths::Eight].find(dataPathToLoad);
			if (foundBitmap != s_LoadedBitmaps[BitDepths::Eight].end()) {
				returnBitmap = foundBitmap->second;
				s_LoadedBitmaps[BitDepths::Eight].erase(dataPathToLoad);
			}

			SDL_DestroySurface(surface);
			s_MemoryPNGs.erase(dataPathToLoad);
		}
	}

	if (returnBitmap == nullptr) {
		if (!System::PathExistsCaseSensitive(dataPathToLoad)) {
			const std::string dataPathWithoutExtension = dataPathToLoad.substr(0, dataPathToLoad.length() - m_DataPathExtension.length());
			const std::string altFileExtension = (m_DataPathExtension == ".png") ? ".bmp" : ".png";

			if (System::PathExistsCaseSensitive(dataPathWithoutExtension + altFileExtension)) {
				g_ConsoleMan.AddLoadWarningLogExtensionMismatchEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
				SetDataPath(m_DataPathWithoutExtension + altFileExtension);
				dataPathToLoad = dataPathWithoutExtension + altFileExtension;
			} else {
				RTEAbort("Failed to find image file with following path and name:\n\n" + dataPathToLoad + " or " + altFileExtension + "\n" + m_FormattedReaderPosition);
			}
		}
		returnBitmap = LoadAndReleaseBitmap(conversionMode, dataPathToLoad); // NOTE: This takes ownership of the bitmap file

		// Insert the bitmap into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
		if (storeBitmap) {
			s_LoadedBitmaps[bitDepth].try_emplace(dataPathToLoad, returnBitmap);
		}
	}

	return returnBitmap;
}

void ContentFile::GetAsAnimation(std::vector<BITMAP*>& vectorToFill, int frameCount, int conversionMode) {
	if (m_DataPath.empty() || frameCount < 1) {
		return;
	}
	vectorToFill.reserve(frameCount);

	if (frameCount == 1) {
		// Check for 000 in the file name in case it is part of an animation but the FrameCount was set to 1. Do not warn about this because it's normal operation, but warn about incorrect extension.
		if (!System::PathExistsCaseSensitive(m_DataPath)) {
			const std::string altFileExtension = (m_DataPathExtension == ".png") ? ".bmp" : ".png";

			if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + "000" + m_DataPathExtension)) {
				SetDataPath(m_DataPathWithoutExtension + "000" + m_DataPathExtension);
			} else if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + "000" + altFileExtension)) {
				g_ConsoleMan.AddLoadWarningLogExtensionMismatchEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
				SetDataPath(m_DataPathWithoutExtension + "000" + altFileExtension);
			}
		}
		vectorToFill.emplace_back(GetAsBitmap(conversionMode));
	} else {
		char framePath[1024];
		for (int frameNum = 0; frameNum < frameCount; ++frameNum) {
			std::snprintf(framePath, sizeof(framePath), "%s%03i%s", m_DataPathWithoutExtension.c_str(), frameNum, m_DataPathExtension.c_str());
			vectorToFill.emplace_back(GetAsBitmap(conversionMode, true, framePath));
		}
	}
}
SDL_Palette* ContentFile::DefaultPaletteToSDL(bool preMask) {
	SDL_Palette* palette = SDL_CreatePalette(256);
	std::array<SDL_Color, 256> paletteColor;
	const PALETTE& defaultPalette = g_FrameMan.GetDefaultPalette();
	if (preMask) {
		paletteColor[0] = {.r = 0, .g = 0, .b = 0, .a = 0};
	} else {
		paletteColor[0] = {.r = defaultPalette[0].r,
			               .g = defaultPalette[0].g,
			               .b = defaultPalette[0].b,
						   .a = 255
			               };
	}
	for (size_t i = 1; i < paletteColor.size(); ++i) {
		paletteColor[i].r = defaultPalette[i].r;
		paletteColor[i].g = defaultPalette[i].g;
		paletteColor[i].b = defaultPalette[i].b;
		paletteColor[i].a = 255;
	}
	SDL_SetPaletteColors(palette, paletteColor.data(), 0, 256);
	return palette;
}

SDL_Surface* ContentFile::LoadImageAsSurface(int conversionMode, const std::string& dataPathToLoad) {
	SDL_Surface* image = IMG_Load(dataPathToLoad.c_str());
	bool convert8To32 = conversionMode & COLORCONV_8_TO_32;
	bool convertTo8 = conversionMode & COLORCONV_REDUCE_TO_256;
	int bitDepth = SDL_GetPixelFormatDetails(image->format)->bits_per_pixel;
	if (convertTo8 && bitDepth != 8) {
		SDL_Palette* palette = DefaultPaletteToSDL();
		SDL_Surface* newImage = SDL_ConvertSurfaceAndColorspace(image, SDL_PIXELFORMAT_INDEX8, palette, SDL_COLORSPACE_UNKNOWN, 0);
		SDL_DestroyPalette(palette);
		SDL_DestroySurface(image);
		image = newImage;
		bitDepth = 8;
	} else if (bitDepth != 8 || convert8To32) {
		SDL_Palette* palette = DefaultPaletteToSDL(true);
		if (SDL_GetPixelFormatDetails(image->format)->bits_per_pixel == 8) {
			SDL_SetSurfacePalette(image, palette);
			SDL_SetSurfaceColorKey(image, true, 0);
		}
		SDL_DestroyPalette(palette);
		SDL_Surface* newImage = SDL_ConvertSurface(image, SDL_PIXELFORMAT_RGBA32);
		SDL_DestroySurface(image);
		image = newImage;
		bitDepth = 32;
	}

	return image;
}

BITMAP* ContentFile::LoadAndReleaseBitmap(int conversionMode, const std::string& dataPathToSpecificFrame) {
	if (m_DataPath.empty()) {
		return nullptr;
	}
	const std::string dataPathToLoad = dataPathToSpecificFrame.empty() ? m_DataPath : dataPathToSpecificFrame;

	SDL_Surface* image = LoadImageAsSurface(conversionMode, dataPathToLoad);
	int bitDepth = SDL_GetPixelFormatDetails(image->format)->bits_per_pixel;

	BITMAP* returnBitmap = create_bitmap_ex(bitDepth, image->w, image->h);

	// allegro doesn't (always) align lines to 4byte, so copy line by line. SDL_Surface.pitch is the size in bytes per line + alignment padding.
	for (int y = 0; y < image->h; ++y) {
		memcpy(returnBitmap->line[y], static_cast<unsigned char*>(image->pixels) + image->pitch * y, image->w * SDL_GetPixelFormatDetails(image->format)->bytes_per_pixel);
	}
	SDL_DestroySurface(image);

	RTEAssert(returnBitmap, "Failed to load image file with following path and name:\n\n" + m_DataPathAndReaderPosition + "\nThe file may be corrupt, incorrectly converted or saved with unsupported parameters.");

	return returnBitmap;
}

FMOD::Sound* ContentFile::GetAsSound(bool abortGameForInvalidSound, bool asyncLoading) {
	if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
		return nullptr;
	}
	FMOD::Sound* returnSample = nullptr;

	std::unordered_map<std::string, FMOD::Sound*>::iterator foundSound = s_LoadedSamples.find(m_DataPath);
	if (foundSound != s_LoadedSamples.end()) {
		returnSample = (*foundSound).second;
	} else {
		returnSample = LoadAndReleaseSound(abortGameForInvalidSound, asyncLoading); // NOTE: This takes ownership of the sample file

		// Insert the Sound object into the map, PASSING OVER OWNERSHIP OF THE LOADED FILE
		s_LoadedSamples.try_emplace(m_DataPath, returnSample);
	}
	return returnSample;
}

FMOD::Sound* ContentFile::LoadAndReleaseSound(bool abortGameForInvalidSound, bool asyncLoading) {
	if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
		return nullptr;
	}
	if (!System::PathExistsCaseSensitive(m_DataPath)) {
		bool foundAltExtension = false;
		for (const char* altFileExtension: c_SupportedAudioFormats) {
			const std::string altDataPathToLoad = m_DataPathWithoutExtension + altFileExtension;
			if (System::PathExistsCaseSensitive(altDataPathToLoad)) {
				g_ConsoleMan.AddLoadWarningLogExtensionMismatchEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
				SetDataPath(altDataPathToLoad);
				foundAltExtension = true;
				break;
			}
		}
		if (!foundAltExtension) {
			std::string errorMessage = "Failed to find audio file with following path and name:\n\n" + m_DataPath + " or any alternative supported file type";
			RTEAssert(!abortGameForInvalidSound, errorMessage + "\n" + m_FormattedReaderPosition);
			g_ConsoleMan.PrintString(errorMessage + ". The file was not loaded!");
			return nullptr;
		}
	}
	if (std::filesystem::file_size(m_DataPath) == 0) {
		const std::string errorMessage = "Failed to create sound because the file was empty. The path and name were: ";
		RTEAssert(!abortGameForInvalidSound, errorMessage + "\n\n" + m_DataPathAndReaderPosition);
		g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
		return nullptr;
	}
	FMOD::Sound* returnSample = nullptr;

	FMOD_MODE fmodFlags = FMOD_CREATESAMPLE | FMOD_3D | (asyncLoading ? FMOD_NONBLOCKING : FMOD_DEFAULT);
	FMOD_RESULT result = g_AudioMan.GetAudioSystem()->createSound(m_DataPath.c_str(), fmodFlags, nullptr, &returnSample);

	if (result != FMOD_OK) {
		const std::string errorMessage = "Failed to create sound because of FMOD error:\n" + std::string(FMOD_ErrorString(result)) + "\nThe path and name were: ";
		RTEAssert(!abortGameForInvalidSound, errorMessage + "\n\n" + m_DataPathAndReaderPosition);
		g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
		return returnSample;
	}
	return returnSample;
}

void ContentFile::ReloadBitmap(const std::string& filePath, int conversionMode) {
	const int bitDepth = (conversionMode == COLORCONV_8_TO_32) ? BitDepths::ThirtyTwo : BitDepths::Eight;

	auto bmpItr = s_LoadedBitmaps[bitDepth].find(filePath);
	if (bmpItr == s_LoadedBitmaps[bitDepth].end()) {
		return;
	}

	PALETTE currentPalette;
	get_palette(currentPalette);
	set_color_conversion((conversionMode == COLORCONV_NONE) ? COLORCONV_NONE : conversionMode);

	BITMAP* loadedBitmap = (*bmpItr).second;

	SDL_Surface* newImage = LoadImageAsSurface(conversionMode, filePath);

	BITMAP* newBitmap = create_bitmap_ex(SDL_GetPixelFormatDetails(newImage->format)->bits_per_pixel, newImage->w, newImage->h);

	// allegro doesn't (always) align lines to 4byte, so copy line by line. SDL_Surface.pitch is the size in bytes per line + alignment padding.
	for (int y = 0; y < newImage->h; y++) {
		memcpy(newBitmap->line[y], static_cast<unsigned char*>(newImage->pixels) + y * newImage->pitch, newImage->w * SDL_GetPixelFormatDetails(newImage->format)->bytes_per_pixel);
	}

	// AddAlphaChannel(newBitmap);
	BITMAP swap;

	std::memcpy(&swap, loadedBitmap, sizeof(BITMAP));
	std::memcpy(loadedBitmap, newBitmap, sizeof(BITMAP));
	std::memcpy(newBitmap, &swap, sizeof(BITMAP));

	destroy_bitmap(newBitmap);
}

void ContentFile::AddAlphaChannel(BITMAP* bitmap) {
	if (!bitmap) {
		return;
	}
	if (bitmap_color_depth(bitmap) != 32) {
		return;
	}
	for (int y = 0; y < bitmap->h; ++y) {
		for (int x = 0; x < bitmap->w; ++x) {
			unsigned long color = _getpixel32(bitmap, x, y);
			if (color != MASK_COLOR_32) {
				_putpixel32(bitmap, x, y, color | (0xFF << _rgb_a_shift_32));
			}
		}
	}
}
