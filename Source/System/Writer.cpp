#include "Writer.h"
#include "System.h"

#include <iomanip>

using namespace RTE;

void Writer::Clear() {
	m_Stream = nullptr;
	m_FilePath.clear();
	m_FolderPath.clear();
	m_FileName.clear();
	m_IndentCount = 0;
}

Writer::Writer(const std::string& fileName, bool append, bool createDir) {
	Clear();
	Create(fileName, append, createDir);
}

Writer::Writer(std::unique_ptr<std::ostream>&& stream) {
	Clear();
	Create(std::move(stream));
}

int Writer::Create(const std::string& fileName, bool append, bool createDir) {
	m_FilePath = fileName;

	// Extract filename and folder path
	size_t slashPos = m_FilePath.find_last_of("/\\");
	m_FileName = m_FilePath.substr(slashPos + 1);
	m_FolderPath = m_FilePath.substr(0, slashPos + 1);

	if (createDir && !std::filesystem::exists(System::GetWorkingDirectory() + m_FolderPath)) {
		System::MakeDirectory(System::GetWorkingDirectory() + m_FolderPath);
	}

	auto ofStream = std::make_unique<std::ofstream>(fileName, append ? (std::ios::out | std::ios::app | std::ios::ate) : (std::ios::out | std::ios::trunc));
	*ofStream << std::fixed << std::setprecision(6);
	return Create(std::move(ofStream));
}

int Writer::Create(std::unique_ptr<std::ostream>&& stream) {
	m_Stream = std::move(stream);
	if (!m_Stream->good()) {
		return -1;
	}

	return 0;
}

void Writer::NewLine(bool toIndent, int lineCount) const {
	for (int lines = 0; lines < lineCount; ++lines) {
		*m_Stream << "\n";
		if (toIndent) {
			*m_Stream << std::string(m_IndentCount, '\t');
		}
	}
}
