#pragma once
#ifndef INCLUDED_toolkit_filehelper_hh
#define INCLUDED_toolkit_filehelper_hh

#include <ctime>
#include <vector>
#include <sys/types.h>

#if defined(LINUX)
#include <dirent.h>
#endif

#include "str.hh"

#if defined(LINUX)
	#define FILE_SEP '/'
	#define FILE_SEP_STR "/"
#elif defined(_WINDOWS)
	#define FILE_SEP '\\'
	#define FILE_SEP_STR "\\"
#endif

namespace lptk
{
	constexpr char kFileSep = FILE_SEP;
	constexpr char kFileSepStr[] = FILE_SEP_STR;

std::vector<char> ReadFile(const char* filename);
bool WriteFile(const char* filename, const std::vector<char>& buffer);
bool FileExists(const char* filename);
bool FileIsDirectory(const char* filename);
bool FileIsWriteable(const char* filename);
bool FileIsReadable(const char* filename);
time_t FileModifiedTime(const char* filename);
size_t FileSize(const char* filename);
bool IsAbsolutePath(const char* filename);
Str GetProgramPath();
Str GetWorkingDirectory();
bool ChangeWorkingDirectory(const char* path);

Str FileBasename(const char* filename);
Str FileBasename(const Str& filename);
Str FileDirname(const char* filename);
Str FileDirname(const Str& filename);
Str PathJoin(const Str& left, const Str& right);
Str PathJoin(const char* left, const char* right);

Str ToSystemFilename(const char* filename);
Str ToSystemFilename(const Str& filename);

class DirList
{
public:
	DirList(const Str& dir);
	~DirList();

	bool Next(Str& fname);

private:
#if defined LINUX
	DIR* m_dir;
#elif defined _WINDOWS
	WIN32_FIND_DATA m_findData;
	HANDLE m_handle;
	bool m_done;
#endif

};

class PushWorkingDirectory
{
public:
	explicit PushWorkingDirectory(const char* filename);
	~PushWorkingDirectory();

	PushWorkingDirectory(const PushWorkingDirectory&) = delete;
	PushWorkingDirectory& operator=(const PushWorkingDirectory&) = delete;
	PushWorkingDirectory(PushWorkingDirectory&&) = delete;
	PushWorkingDirectory& operator=(PushWorkingDirectory&&) = delete;
private:
	lptk::Str m_old;
};

}

#endif

