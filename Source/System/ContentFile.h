#pragma once

#include "Serializable.h"

namespace FMOD {
	class Sound;
}
struct BITMAP;

namespace RTE {

	/// A representation of a content file that is stored directly on disk.
	class ContentFile : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

#pragma region Creation
		/// Constructor method used to instantiate a ContentFile object in system memory. Create() should be called before using the object.
		ContentFile() { Clear(); }

		/// Constructor method used to instantiate a ContentFile object in system memory, and also do a Create() in the same line.
		/// @param filePath A string defining the path to where the content file itself is located.
		explicit ContentFile(const char* filePath) {
			Clear();
			Create(filePath);
		}

		/// Constructor method used to instantiate a ContentFile object in system memory from a hash value of the file path, and also do a Create() in the same line.
		/// @param hash A hash value containing the path to where the content file itself is located.
		explicit ContentFile(size_t hash) {
			Clear();
			Create(GetPathFromHash(hash).c_str());
		}

		/// Makes the ContentFile object ready for use.
		/// @param filePath A string defining the path to where the content file itself is located.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const char* filePath);

		/// Creates a ContentFile to be identical to another, by deep copy.
		/// @param reference A reference to the ContentFile to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const ContentFile& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a ContentFile object before deletion from system memory.
		~ContentFile() override = default;

		/// Resets the entire ContentFile, including its inherited members, to their default settings or values.
		void Reset() override { Clear(); }

		/// Frees all loaded data used by all ContentFile instances. This should ONLY be done when quitting the app, or after everything else is completely destroyed.
		static void FreeAllLoaded();
#pragma endregion

#pragma region Getters and Setters
		/// Gets the file path of the content file represented by this ContentFile object.
		/// @return A string with the file name path.
		const std::string& GetDataPath() const { return m_DataPath; }

		/// Sets the file path of the content file represented by this ContentFile object.
		/// @param newDataPath A string with the new file name path.
		void SetDataPath(const std::string& newDataPath);

		/// Creates a hash value out of a path to a ContentFile.
		/// @return Hash value of a path to a ContentFile.
		size_t GetHash() const;

		/// Converts hash values into file paths to ContentFiles.
		/// @param hash Hash value to get file path from.
		/// @return Path to ContentFile.
		static std::string GetPathFromHash(size_t hash) { return (s_PathHashes.find(hash) == s_PathHashes.end()) ? "" : s_PathHashes[hash]; }
#pragma endregion

#pragma region Logging
		/// Gets the file and line that are currently being read. Formatted to be used for logging warnings and errors.
		/// @return A string containing the currently read file path and the line being read.
		const std::string& GetFormattedReaderPosition() const { return m_FormattedReaderPosition; }

		/// Sets the file and line that are currently being read. Formatted to be used for logging warnings and errors.
		/// @param newPosition A string containing the currently read file path and the line being read.
		void SetFormattedReaderPosition(const std::string& newPosition) override;
#pragma endregion

#pragma region Image Info Getters
		/// Gets whether the data file at this ContentFile's path is a supported image format.
		/// @return Whether the data file at this ContentFile's path is a supported image format.
		bool DataPathIsImageFile() const { return m_DataPathIsImageFile; }

		/// Gets the bit depth of the image file at this ContentFile's path.
		/// @return The bit depth of the image file at this ContentFile's path, or -1 if the file is not an image format.
		int GetImageBitDepth() { return m_DataPathIsImageFile ? GetImageFileInfo(ImageFileInfoType::ImageBitDepth) : -1; }

		/// Gets the width of the image file at this ContentFile's path.
		/// @return The width of the image file at this ContentFile's path, in pixels, or -1 if the file is not an image format.
		int GetImageWidth() { return m_DataPathIsImageFile ? GetImageFileInfo(ImageFileInfoType::ImageWidth) : -1; }

		/// Gets the height of the image file at this ContentFile's path.
		/// @return The height of the image file at this ContentFile's path, in pixels, or -1 if the file is not an image format.
		int GetImageHeight() { return m_DataPathIsImageFile ? GetImageFileInfo(ImageFileInfoType::ImageHeight) : -1; }
#pragma endregion

#pragma region Data Handling
		/// Reloads all BITMAPs in the cache from disk, allowing any changes to be reflected at runtime.
		static void ReloadAllBitmaps();

		/// Gets the data represented by this ContentFile object as an Allegro BITMAP, loading it into the static maps if it's not already loaded. Note that ownership of the BITMAP is NOT transferred!
		/// @param conversionMode The Allegro color conversion mode to use when loading this bitmap.
		/// @param storeBitmap Whether to store the BITMAP in the relevant static map after loading it or not. If this is false, ownership of the BITMAP IS transferred!
		/// @param dataPathToSpecificFrame Path to a specific frame when loading an animation to avoid overwriting the original preset DataPath when loading each frame.
		/// @return Pointer to the BITMAP loaded from disk.
		BITMAP* GetAsBitmap(int conversionMode = 0, bool storeBitmap = true, const std::string& dataPathToSpecificFrame = "");

		/// Fills an existing vector of Allegro BITMAPs representing each frame in the animation with the data represented by this ContentFile object.
		/// It loads the BITMAPs into the static maps if they're not already loaded. Ownership of the BITMAPs is NOT transferred!
		/// @param vectorToFill The existing vector of Allegro BITMAPs to fill.
		/// @param frameCount The number of frames to attempt to load, more than 1 frame will mean 00# is appended to DataPath to handle naming conventions.
		/// @param conversionMode The Allegro color conversion mode to use when loading this bitmap.
		void GetAsAnimation(std::vector<BITMAP*>& vectorToFill, int frameCount = 1, int conversionMode = 0);

		/// Gets the data represented by this ContentFile object as a vector of Allegro BITMAPs, each representing a frame in the animation.
		/// It loads the BITMAPs into the static maps if they're not already loaded. Ownership of the BITMAPs is NOT transferred!
		/// @param frameCount The number of frames to attempt to load, more than 1 frame will mean 00# is appended to DataPath to handle naming conventions.
		/// @param conversionMode The Allegro color conversion mode to use when loading this bitmap.
		/// @return A vector of BITMAP pointers loaded from the disk.
		std::vector<BITMAP*> GetAsAnimation(int frameCount = 1, int conversionMode = 0) {
			std::vector<BITMAP*> returnBitmaps;
			GetAsAnimation(returnBitmaps, frameCount, conversionMode);
			return returnBitmaps;
		}

		/// Gets the data represented by this ContentFile object as an FMOD FSOUND_SAMPLE, loading it into the static maps if it's not already loaded. Ownership of the FSOUND_SAMPLE is NOT transferred!
		/// @param abortGameForInvalidSound Whether to abort the game if the sound couldn't be added, or just show a console error. Default true.
		/// @param asyncLoading Whether to enable FMOD asynchronous loading or not. Should be disabled for loading audio files with Lua AddSound.
		/// @return Pointer to the FSOUND_SAMPLE loaded from disk.
		FMOD::Sound* GetAsSound(bool abortGameForInvalidSound = true, bool asyncLoading = true);
#pragma endregion

	private:
		/// Enumeration for loading BITMAPs by bit depth. NOTE: This can't be lower down because s_LoadedBitmaps relies on this definition.
		enum BitDepths {
			Eight = 0,
			ThirtyTwo,
			BitDepthCount
		};

		/// Enumeration for the image file information types that can be stored.
		enum ImageFileInfoType {
			ImageBitDepth,
			ImageWidth,
			ImageHeight,
			ImageInfoTypeCount
		};

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		static std::unordered_map<size_t, std::string> s_PathHashes; //!< Static map containing the hash values of paths of all loaded data files.
		static std::array<std::unordered_map<std::string, BITMAP*>, BitDepths::BitDepthCount> s_LoadedBitmaps; //!< Static map containing all the already loaded BITMAPs and their paths for each bit depth.
		static std::unordered_map<std::string, FMOD::Sound*> s_LoadedSamples; //!< Static map containing all the already loaded FSOUND_SAMPLEs and their paths.

		std::string m_DataPath; //!< The path to this ContentFile's data file. In the case of an animation, this filename/name will be appended with 000, 001, 002 etc.
		std::string m_DataPathExtension; //!< The extension of the data file of this ContentFile's path.
		std::string m_DataPathWithoutExtension; //!< The path to this ContentFile's data file without the file's extension.

		bool m_DataPathIsImageFile; //!< Whether the data file at this ContentFile's path is a supported image format.
		std::array<int, ImageFileInfoType::ImageInfoTypeCount> m_ImageFileInfo; //!< Array that holds image file information read directly from the data file on disk.

		std::string m_FormattedReaderPosition; //!< A string containing the currently read file path and the line being read. Formatted to be used for logging.
		std::string m_DataPathAndReaderPosition; //!< The path to this ContentFile's data file combined with the ini file and line it is being read from. This is used for logging.

#pragma region Image Info Getters
		/// Gets the specified image info from this ContentFile's data file on disk.
		/// @param infoTypeToGet The image info type to get. See ImageFileInfoType enumeration.
		/// @return An integer value with the requested image info.
		int GetImageFileInfo(ImageFileInfoType infoTypeToGet);

		/// Reads a PNG file from disk and stores the relevant information without actually loading the whole file into memory.
		/// @param imageFile Pointer to the file stream to read.
		void ReadAndStorePNGFileInfo(FILE* imageFile);

		/// Reads a BMP file from disk and stores the relevant information without actually loading the whole file into memory.
		/// @param imageFile Pointer to the file stream to read.
		void ReadAndStoreBMPFileInfo(FILE* imageFile);
#pragma endregion

#pragma region Data Handling
		/// Loads and transfers the data represented by this ContentFile object as an Allegro BITMAP. Ownership of the BITMAP IS transferred!
		/// Note that this is relatively slow since it reads the data from disk each time.
		/// @param conversionMode The Allegro color conversion mode to use when loading this bitmap. Only applies the first time a bitmap is loaded from the disk.
		/// @param dataPathToSpecificFrame Path to a specific frame when loading an animation to avoid overwriting the original preset DataPath when loading each frame.
		/// @return Pointer to the BITMAP loaded from disk.
		BITMAP* LoadAndReleaseBitmap(int conversionMode = 0, const std::string& dataPathToSpecificFrame = "");

		/// Loads and transfers the data represented by this ContentFile object as an FMOD FSOUND_SAMPLE. Ownership of the FSOUND_SAMPLE is NOT transferred!
		/// @param abortGameForInvalidSound Whether to abort the game if the sound couldn't be added, or just show a console error. Default true.
		/// @param asyncLoading Whether to enable FMOD asynchronous loading or not. Should be disabled for loading audio files with Lua AddSound.
		/// @return Pointer to the FSOUND_SAMPLE loaded from disk.
		FMOD::Sound* LoadAndReleaseSound(bool abortGameForInvalidSound = true, bool asyncLoading = true);

		/// Reloads a specific BITMAP in the cache from disk, allowing any changes to be reflected at runtime.
		/// @param filePath The filepath to the bitmap we want to reload.
		/// @param conversionMode The Allegro color conversion mode to use when reloading this bitmap.
		static void ReloadBitmap(const std::string& filePath, int conversionMode = 0);

		/// Set alpha value of non mask color pixels to 255 for 32-bit bitmaps. (WARN: would override existing alpha values!)
		static void AddAlphaChannel(BITMAP* bitmap);
#pragma endregion

		/// Clears all the member variables of this ContentFile, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
