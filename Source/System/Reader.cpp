#include "Reader.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "SettingsMan.h"
#include "System.h"

#include <fstream>

using namespace RTE;

void Reader::Clear() {
	m_Stream = nullptr;
	m_FilePath.clear();
	m_CurrentLine = 1;
	m_PreviousIndent = 0;
	m_IndentDifference = 0;
	m_ObjectEndings = 0;
	m_EndOfStreams = false;
	m_ReportTabs = "\t";
	m_FileName.clear();
	m_DataModuleName.clear();
	m_DataModuleID = -1;
	m_OverwriteExisting = false;
	m_SkipIncludes = false;
	m_CanFail = false;
	m_NonModulePath = false;
}

Reader::Reader(const std::string& fileName, bool overwrites, bool failOK, bool nonModulePath) {
	Clear();
	m_NonModulePath = nonModulePath;
	Create(fileName, overwrites, failOK);
}

Reader::Reader(std::unique_ptr<std::istream>&& stream, const std::string& fileName, bool overwrites, bool failOK) {
	Clear();
	Create(std::move(stream), fileName, overwrites, failOK);
}

int Reader::Create(const std::string& fileName, bool overwrites, bool failOK) {
	if (fileName.empty()) {
		return -1;
	}
	
	if (m_NonModulePath) {
		m_FilePath = std::filesystem::path(fileName).generic_string();
	} else {
		m_FilePath = g_PresetMan.GetFullModulePath(fileName);
	}
	
	return Create(
		std::make_unique<std::ifstream>(m_FilePath),
		fileName,
		overwrites,
		failOK
	);
}

int Reader::Create(std::unique_ptr<std::istream>&& stream, const std::string& fileName, bool overwrites, bool failOK) {
	// We redundantly do this following block of code in both constructors, which feels really ugly and lazy
	if (fileName.empty()) {
		return -1;
	}

	if (m_NonModulePath) {
		m_FilePath = std::filesystem::path(fileName).generic_string();
		// Associate non-module paths with Base to prevent implosions when dealing with creating Entities.
		m_DataModuleName = "Base.rte";
		m_DataModuleID = 0;
	} else {
		m_FilePath = g_PresetMan.GetFullModulePath(fileName);

		// Extract the file name and module name from the path
		m_FileName = m_FilePath.substr(m_FilePath.find_last_of("/\\") + 1);
		m_DataModuleName = g_PresetMan.GetModuleNameFromPath(m_FilePath);
		m_DataModuleID = g_PresetMan.GetModuleID(m_DataModuleName);
	}

	m_CanFail = failOK;

	m_Stream = std::move(stream);

	if (!m_CanFail) {
		RTEAssert(m_Stream->good(), "Failed to open data file \"" + m_FilePath + "\"!");
	}

	m_OverwriteExisting = overwrites;

	return m_Stream->good() ? 0 : -1;
}

int Reader::GetReadModuleID() const {
	return (m_DataModuleID < 0) ? g_PresetMan.GetModuleID(m_DataModuleName) : m_DataModuleID;
}

std::string Reader::WholeFileAsString() const {
	std::stringstream stringStream;
	stringStream << m_Stream->rdbuf();
	return stringStream.str();
}

std::string Reader::ReadLine() {
	DiscardEmptySpace();

	std::string retString;

	char ch;
	while (m_Stream->get(ch)) {
		if (ch == '\n' || ch == '\r' || ch == '\t') {
			m_Stream->unget();
			break;
		}
		if (ch == '/') {
			if (m_Stream->peek() == '/') {
				m_Stream->unget();
				break;
			}
		}
		retString += ch;
	}

	if (!m_Stream->good() && !m_Stream->eof()) {
		ReportError("Stream failed for some reason");
	}

	return TrimString(retString);
}

std::string Reader::ReadPropName() {
	DiscardEmptySpace();

	std::string retString;
	retString.reserve(32);

	char ch;
	while (m_Stream->get(ch)) {
		if (ch == '=') {
			break;
		}
		if (ch == '\n' || ch == '\r' || ch == '\t') {
			ReportError("Property name wasn't followed by a value");
		}
		retString += ch;
	}

	// Handle EOF / include file end
	if (m_Stream->eof()) {
		EndIncludeFile();
	} else if (!m_Stream->good()) {
		ReportError("Stream failed for some reason");
		EndIncludeFile();
	}

	// Trim the string of whitespace
	retString = TrimString(retString);

	// If the property name turns out to be the special IncludeFile,
	// and we're not skipping include files then open that file and 
	// read the first property from it instead.
	if (retString == "IncludeFile") {
		if (m_SkipIncludes) {
			// Discard IncludeFile value
			std::string val = ReadPropValue();
			DiscardEmptySpace();
			retString = ReadPropName();
		} else {
			StartIncludeFile();
			// Return the first property name in the new file, this 
			// is to make the file inclusion seamless.
			// Alternatively, if StartIncludeFile failed, 
			// this will just grab the next prop name and ignore 
			// the failed IncludeFile property.
			retString = ReadPropName();
		}
	}
	return retString;
}

std::string Reader::ReadPropValue() {
	std::string valueStr = ReadLine();

	return TrimString(valueStr);
}

bool Reader::NextProperty() {
	if (!DiscardEmptySpace() || m_EndOfStreams) {
		return false;
	}
	// If there are fewer tabs on the last line eaten this time, that means there are no more properties to read on this object.
	if (m_ObjectEndings < -m_IndentDifference) {
		m_ObjectEndings++;
		return false;
	}
	m_ObjectEndings = 0;
	return true;
}

std::string Reader::TrimString(const std::string& stringToTrim) const {
	if (stringToTrim.empty()) {
		return "";
	}

	const char* s = stringToTrim.data();
	const char* e = s + stringToTrim.size();

	// trim left
	while (s < e && (*s == ' ')) {
		++s;
	}

	// trim right
	while (e > s && (*(e - 1) == ' ')) {
		--e;
	}

	return std::string(s, e);
}

bool Reader::DiscardEmptySpace() {
	int indent = 0;
	int leadingSpaceCount = 0;
	bool discardedLine = false;

	char ch;
	while (m_Stream->get(ch)) {
		// Discard spaces
		if (ch == ' ') {
			leadingSpaceCount++;
		}
		// Discard tabs, and count them
		else if (ch == '\t') {
			indent++;
		} 
		// Discard newlines and reset the tab count for
		// the new line, also count the lines
		else if (ch == '\n' || ch == '\r') {
			// So we don't count lines twice when there are
			// both newline and carriage return at the end of lines
			if (ch == '\n') {
				m_CurrentLine++;
			}
			indent = 0;
			leadingSpaceCount = 0;
			discardedLine = true;
		}
		// Comment line?
		else if (ch == '/') {
			// "//" comment
			if (m_Stream->peek() == '/') {
				while (m_Stream->peek() != '\n' && m_Stream->peek() != '\r' && !m_Stream->eof()) {
					m_Stream->ignore(1);
				}
			} 
			// "/**/" comment
			else if (m_Stream->peek() == '*') {
				int openBlockComments = 1;
				m_BlockCommentOpenTagLines.emplace(m_CurrentLine);

				char temp2 = 0;
				while (openBlockComments > 0 && !m_Stream->eof()) {
					temp2 = static_cast<char>(m_Stream->get());
					if (temp2 == '\n') {
						++m_CurrentLine;
					}

					// Find the matching close tag.
					if (!(temp2 == '*' && m_Stream->peek() == '/')) {
						// Check if a nested block comment open tag.
						if (temp2 == '/' && m_Stream->peek() == '*') {
							openBlockComments++;
							m_BlockCommentOpenTagLines.emplace(m_CurrentLine);
						}
					} else {
						openBlockComments--;
						m_BlockCommentOpenTagLines.pop();
					}
				}
				// Discard that final '/'.
				if (!m_Stream->eof()) {
					m_Stream->ignore(1);
				} else if (openBlockComments > 0) {
					ReportError("File stream ended with an open block comment!\nCouldn't find closing tag for block comment opened on line " + std::to_string(m_BlockCommentOpenTagLines.top()) + ".\n");
				}

				
			} 
			// Not a comment, quit.
			else {
				m_Stream->unget();
				break;
			}
		} 
		// Non-space and non-comment character
		else {
			m_Stream->unget();
			break;
		}
	}
	
	if (m_Stream->eof()) {
		return EndIncludeFile();
	} else if (!m_Stream->good()) {
		ReportError("Something went wrong reading the line; make sure it is providing the expected type");
	}

	// This precaution enables us to use DiscardEmptySpace repeatedly without messing up the indentation tracking logic
	if (discardedLine) {
		if (leadingSpaceCount > 0) {
			ReportError("Encountered space characters used for indentation where a tab character was expected!\nPlease make sure the preset definition structure is correct.\n");
		}
		// Get indentation difference from the last line of the last call to DiscardEmptySpace(), and the last line of this call to DiscardEmptySpace().
		m_IndentDifference = indent - m_PreviousIndent;
		if (m_IndentDifference > 1) {
			ReportError("Over indentation detected!\nPlease make sure the preset definition structure is correct.\n");
		}
		// Save the last tab count
		m_PreviousIndent = indent;
	}
	return true;
}

void Reader::ReportError(const std::string& errorDesc) const {
	if (!m_CanFail) {
		RTEAbort(errorDesc + "\nError happened in " + m_FilePath + " at line " + std::to_string(m_CurrentLine) + "!");
	}
}

bool Reader::StartIncludeFile() {
	// Get the file path from the current stream before pushing it into the StreamStack, otherwise we can't open a new stream after releasing it because we can't read.
	std::string includeFilePath = g_PresetMan.GetFullModulePath(ReadPropValue());

	// Push the current stream onto the StreamStack for future retrieval when the new include file has run out of data.
	m_StreamStack.push(StreamInfo(m_Stream.release(), m_FilePath, m_CurrentLine, m_PreviousIndent));

	m_FilePath = includeFilePath;
	m_Stream = std::make_unique<std::ifstream>(m_FilePath);

	if (m_Stream->fail() || !System::PathExistsCaseSensitive(includeFilePath)) {
		// Backpedal and set up to read the next property in the old stream
		m_Stream.reset(m_StreamStack.top().Stream); // Destructs the current m_Stream and takes back ownership and management of the raw StreamInfo std::istream pointer.
		m_FilePath = m_StreamStack.top().FilePath;
		m_CurrentLine = m_StreamStack.top().CurrentLine;
		m_PreviousIndent = m_StreamStack.top().PreviousIndent;
		m_StreamStack.pop();

		ReportError((!m_CanFail ? "" : m_ReportTabs + "\t") + "Failed to open included data file \"" + includeFilePath + "\"");

		DiscardEmptySpace();
		return false;
	}

	// Line counting starts with 1, not 0
	m_CurrentLine = 1;
	// This is set to 0, because locally in the included file, all properties start at that count
	m_PreviousIndent = 0;

	m_FileName = m_FilePath.substr(m_FilePath.find_first_of("/\\") + 1);

	DiscardEmptySpace();
	return true;
}

bool Reader::EndIncludeFile() {
	if (m_StreamStack.empty()) {
		m_EndOfStreams = true;
		return false;
	}

	// Replace the current included stream with the parent one
	m_Stream.reset(m_StreamStack.top().Stream);
	m_FilePath = m_StreamStack.top().FilePath;
	m_CurrentLine = m_StreamStack.top().CurrentLine;

	// Observe it's being added, not just replaced. This is to keep proper track when exiting out of a file.
	m_PreviousIndent += m_StreamStack.top().PreviousIndent;

	m_StreamStack.pop();

	m_FileName = m_FilePath.substr(m_FilePath.find_first_of("/\\") + 1);

	DiscardEmptySpace();
	return true;
}
