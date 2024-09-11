#pragma once

#include <unordered_set>
#include <source_location>
#include <filesystem> // only one use of std::filesystem::file_time_type
#include <vector>

namespace RTE {

	/// Class for the system functionality.
	class System {

	public:
#pragma region Creation
		/// Store the current working directory and create any missing subdirectories.
		/// @param thisExePathAndName The path and name of this executable.
		static void Initialize(const char* thisExePathAndName);
#pragma endregion

#pragma region Program Termination
		/// Gets whether the program was set to be terminated by the user.
		/// @return Whether the program was set to be terminated by the user.
		static bool IsSetToQuit() { return s_Quit; }

		/// Sets the program to be terminated.
		/// @param quitOrNot Terminate or not.
		static void SetQuit(bool quitOrNot = true) { s_Quit = quitOrNot; }

		/// Sets termination when the close button (X) is pressed on the program window.
		static void WindowCloseButtonHandler() { SetQuit(); }
#pragma endregion

#pragma region Directories
		/// Gets the absolute path to this executable.
		/// @return Absolute path to this executable.
		static const std::string& GetThisExePathAndName() { return s_ThisExePathAndName; }

		/// Gets the current working directory.
		/// @return Absolute path to current working directory.
		static const std::string& GetWorkingDirectory() { return s_WorkingDirectory; }

		/// Gets the game data directory name.
		/// @return Folder name of the game data directory.
		static const std::string& GetDataDirectory() { return s_DataDirectory; }

		/// Gets the screenshot directory name.
		/// @return Folder name of the screenshots directory.
		static const std::string& GetScreenshotDirectory() { return s_ScreenshotDirectory; }

		/// Gets the mod directory name.
		/// @return Folder name of the mod directory.
		static const std::string& GetModDirectory() { return s_ModDirectory; }

		/// Gets the userdata directory name.
		/// @return Folder name of the userdata directory.
		static const std::string& GetUserdataDirectory() { return s_UserdataDirectory; }

		/// Gets the extension that determines a directory/file is an RTE module.
		/// @return String containing the RTE module extension.
		static const std::string& GetModulePackageExtension() { return s_ModulePackageExtension; }

		/// Gets the extension that determines a file is a zipped RTE module.
		/// @return String containing the zipped RTE module extension.
		static const std::string& GetZippedModulePackageExtension() { return s_ZippedModulePackageExtension; }

		/// Create a directory.
		/// @param path Path to create.
		/// @return Returns 0 if successful.
		static bool MakeDirectory(const std::string& pathToMake);
#pragma endregion

#pragma region Filesystem
		/// Gets whether case sensitivity is enforced when checking for file existence.
		/// @return Whether case sensitivity is enforced.
		static bool FilePathsCaseSensitive() { return s_CaseSensitive; }

		/// Sets whether case sensitivity should be enforced when checking for file existence.
		/// @param enable Whether case sensitivity should be enforced or not.
		static void EnableFilePathCaseSensitivity(bool enable) { s_CaseSensitive = enable; }

		/// Checks if a file exists. On case sensitive filesystems returns std::filesystem::exists, otherwise the working directory will be checked for a matching file.
		/// @param pathToCheck The path to check.
		/// @return Whether the file exists.
		static bool PathExistsCaseSensitive(const std::string& pathToCheck);
#pragma endregion

#pragma region Command - Line Interface
		/// Tells whether printing loading progress report and console to command-line is enabled or not.
		/// @return Whether printing to command-line is enabled or not.
		static bool IsLoggingToCLI() { return s_LogToCLI; }

		/// Enables printing the loading progress report and console to command-line. For Windows, also allocates a console instance and redirects cout to it.
		static void EnableLoggingToCLI();

		/// Prints the loading progress report to command-line.
		static void PrintLoadingToCLI(const std::string& reportString, bool newItem = false);

		/// Prints console output to command-line.
		/// @param inputString
		static void PrintToCLI(const std::string& stringToPrint);
#pragma endregion

#pragma region Archived DataModule Handling
		/// Extracts all files from a zipped DataModule, overwriting any corresponding files already existing.
		/// @param zippedModulePath Path to the module to extract.
		/// @return A string containing the progress report of the extraction.
		static std::string ExtractZippedDataModule(const std::string& zippedModulePath);
#pragma endregion

#pragma region Module Validation
		/// Gets Whether the program is running in module validation mode that is used by an external tool.
		/// @return Whether the program is running in module validation mode.
		static bool IsInExternalModuleValidationMode() { return s_ExternalModuleValidation; }

		/// Sets the program to run in module validation mode to be used by an external tool.
		static void EnableExternalModuleValidationMode() { s_ExternalModuleValidation = true; }
#pragma endregion

#pragma region Misc
		/// Fires up the default browser for the current OS on a specific URL.
		/// @param goToURL A string with the URL to send the browser to.
		static void OpenBrowserToURL(const std::string_view& goToURL) { std::system(std::string("start ").append(goToURL).c_str()); }

		/// Searches through an ASCII file on disk for a specific string and tells whether it was found or not.
		/// @param  The path to the ASCII file to search.
		/// @param  The exact string to look for. Case sensitive!
		/// @return 0 if the string was found in the file or 1 if not. -1 if the file was inaccessible.
		static int ASCIIFileContainsString(const std::string& filePath, const std::string_view& findString);
#pragma endregion

	private:
		static bool s_Quit; //!< Whether the user requested program termination through GUI or the window close button.
		static bool s_LogToCLI; //!< Bool to tell whether to print the loading log and anything specified with PrintToCLI to command-line or not.
		static bool s_ExternalModuleValidation; //!< Whether to run the program in a special mode where it will immediately quit without any messages after either successful loading of all modules or aborting during loading. For use by an external tool.
		static std::string s_ThisExePathAndName; //!< String containing the absolute path to this executable. Used for relaunching via abort message.
		static std::string s_WorkingDirectory; //!< String containing the absolute path to current working directory.
		static std::vector<size_t> s_WorkingTree; //!< Vector of the hashes of all file paths in the working directory.
		static std::filesystem::file_time_type s_ProgramStartTime; //!< Low precision time point of program start for checking if a file was created after starting.

		static bool s_CaseSensitive; //!< Whether case sensitivity is enforced when checking for file existence.
		static const std::string s_DataDirectory; //!< String containing the folder name of the game data directory.
		static const std::string s_ScreenshotDirectory; //!< String containing the folder name of the screenshots directory.
		static const std::string s_ModDirectory; //!< String containing the folder name of the mod directory.
		static const std::string s_UserdataDirectory; //!< String containing the folder name of the userdata directory.
		static const std::string s_ModulePackageExtension; //!< The extension that determines a directory/file is a RTE module.
		static const std::string s_ZippedModulePackageExtension; //!< The extension that determines a file is a zipped RTE module.

		static const std::unordered_set<std::string> s_SupportedExtensions; //!< Container for all the supported file extensions that are allowed to be extracted from zipped module files.
		static constexpr int s_MaxFileName = 512; //!< Maximum length of output file directory + name string.
		static constexpr int s_FileBufferSize = 8192; //!< Buffer to hold data read from the zip file.
		static constexpr int s_MaxUnzippedFileSize = 104857600; //!< Maximum size of single file being extracted from zip archive (100MiB).
	};
} // namespace RTE
