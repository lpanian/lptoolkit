#include "toolkit/common.hh"

#include <cstdio>
#include <cstdlib>
#include "toolkit/filehelper.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <climits>

#if defined(LINUX)
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#endif
#if defined(WINDOWS)
#include <io.h>
#include <direct.h>
#include <codecvt>
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

namespace lptk
{
    bool FileExists(const char* filename)
    {
#ifdef USING_VS
        return (_access(filename,00) == 0);
#else
        return (access(filename,F_OK) == 0);
#endif
    }

    time_t FileModifiedTime(const char* filename)
    {
        if(!FileExists(filename))
            return 0;
        struct stat buffer;
        if(stat(filename, &buffer) < 0)
            return 0;

        return buffer.st_mtime;
    }

    Str GetProgramPath()
    {
#if defined(LINUX)
        char path[PATH_MAX+1];
        char procQuery[PATH_MAX+1];
        snprintf(procQuery, PATH_MAX, "/proc/%d/exe", getpid());
        int bytes = readlink(procQuery, path, PATH_MAX);
#elif defined(WINDOWS)
        wchar_t path[PATH_MAX+1];
        int bytes = GetModuleFileName(NULL, path, PATH_MAX);
#endif
        path[bytes] = '\0';

#if defined(WINDOWS)
        // convert to utf8 before returning to lptk::Str (which is char only)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.to_bytes(path).c_str();
        }
#else
        return path;
#endif
    }
    
    Str GetWorkingDirectory()
    {
        char path[PATH_MAX+1];
#if defined(LINUX)
        const char* cwd = getcwd(path, ARRAY_SIZE(path));
#elif defined(WINDOWS)
        const char* cwd = _getcwd(path, ARRAY_SIZE(path));
#endif
        if(!cwd) 
        {
            fprintf(stderr, "ERROR: failed to get the current working directory\n");
            cwd = "";
        }
        return Str(cwd);
    }

    bool ChangeWorkingDirectory(const char* path)
    {
#if defined(LINUX)
        return 0 == chdir(path);
#elif defined(WINDOWS)
        return 0 == _chdir(path);
#endif
    }

    bool MakeDir(const char* path)
    {
        if (FileExists(path))
        {
            if (FileIsDirectory(path))
                return true;
            return false;
        }
#if defined(LINUX)
        return 0 == mkdir(path, 0777);
#elif defined(WINDOWS)
        return 0 == _mkdir(path);
#endif
    }

    bool MakeDirs(const char* path)
    {
        lptk::Str fullPath = path;
        if (fullPath.empty())
            return false;
        
        auto pos = fullPath.find(FILE_SEP);
        while (pos >= 0)
        {
            const auto tmp = fullPath[pos];
            fullPath[pos] = '\0';
            if (!MakeDir(fullPath.c_str()))
                return false;
            fullPath[pos] = tmp;
            pos = fullPath.find(pos + 1, FILE_SEP);
        }
        if (!MakeDir(fullPath.c_str()))
            return false;

        return true;
    }

    bool FileIsDirectory(const char* filename)
    {
        struct stat s;

        if(stat(filename,&s) == 0) {
#if defined(LINUX)
            return S_ISDIR(s.st_mode);
#elif defined(WINDOWS)
            return 0 != (_S_IFDIR & s.st_mode);
#endif
        } else {
            return false;
        }

    }

    bool FileIsWriteable(const char* filename)
    {
        int mode = 02;
#if defined(LINUX)
        mode = W_OK;
        return (access(filename,mode) == 0);
#elif defined(WINDOWS)
        return (_access(filename,mode) == 0);
#endif
    }

    bool FileIsReadable(const char* filename)
    {
        int mode = 04;
#if defined(LINUX)
        mode = R_OK;
        return (access(filename,mode) == 0);
#elif defined(WINDOWS)
        return (_access(filename,mode) == 0);
#endif
    }

    size_t FileSize(const char* filename)
    {
        struct stat s;

        if(stat(filename,&s) == 0) {
            return s.st_size;
        } else {
            return 0;
        }
    }

    bool IsAbsolutePath(const char* filename)
    {
        bool result = false;
#if defined(LINUX)
        result = (filename[0] == FILE_SEP);
#elif defined(WINDOWS)
        result = isalpha(filename[0]) && filename[1] == ':' && filename[2] == FILE_SEP;
#endif
        return result;
    }

    Str FileBasename(const char* filename, char fileSep)
    {
        int curIdx = 0;
        int lastEnd = 0;
        while(filename[curIdx] != '\0') {
            if(filename[curIdx] == fileSep)
                lastEnd = curIdx;
            ++curIdx;
        }
        ++lastEnd;
        return Str(filename + lastEnd, curIdx - lastEnd);
    }

    Str FileBasename(const Str& filename, char fileSep)
    {
        int pos = filename.rfind(fileSep);
        if(pos >= 0)
        {
            return filename.substr(pos+1, -1);
        }
        else return Str();
    }

    Str FileDirname(const char* filename, char fileSep)
    {
        int curIdx = 0;
        int lastEnd = 0;
        while(filename[curIdx] != '\0') {
            if(filename[curIdx] == fileSep)
                lastEnd = curIdx;
            ++curIdx;
        }
        return Str(filename, lastEnd);
    }

    Str FileDirname(const Str& filename, char fileSep)
    {
        int pos = filename.rfind(fileSep);
        if(pos >= 0)
        {
            return filename.substr(0, pos);
        }
        else return Str();
    }

    Str PathJoin(const Str& left, const Str& right, const char* fileSep)
    {
        Str l = left;
        Str r = right;
        while (!r.empty() && StartsWith(r, fileSep))
            r = r.substr(static_cast<int>(strlen(fileSep)));
        while (!l.empty() && EndsWith(l, fileSep))
            l = l.substr(0, static_cast<int>(l.length() - strlen(fileSep)));
        if (l.empty())
            return r;
        else if (r.empty())
            return l;
        else
            return l + fileSep + r;
    }

    Str PathJoin(const char* left, const char* right, const char* fileSep)
    {
        return PathJoin(Str(left), Str(right), fileSep);
    }

    Str ToSystemFilename(const char* filename)
    {
        Str result;
        const char* start = filename;
        while(*start && *start == '/') ++start;
        if(start - filename < 2)
            result = start;
        else
            result = Str(FILE_SEP_STR) + start;
        if('/' != FILE_SEP)
            result.sub('/', FILE_SEP);
        return result;
    }

    Str ToSystemFilename(const Str& filename)
    {
        return ToSystemFilename(filename.c_str());
    }

    ////////////////////////////////////////////////////////////////////////////////
    DirList::DirList(const Str& dir)
#ifdef LINUX
        : m_dir(0)
#endif
#ifdef WINDOWS
        : m_findData()
        , m_handle(INVALID_HANDLE_VALUE)
        , m_done(false)
#endif
        {
#ifdef LINUX
            m_dir = opendir(dir.c_str());
#endif
#ifdef WINDOWS
            Str actualDir = dir;
            if(!EndsWith(dir, "\\*"))
                actualDir += "\\*";

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            const auto wideString = converter.from_bytes(actualDir.c_str());

            m_handle = FindFirstFile(wideString.c_str(), &m_findData);
            if(m_handle == INVALID_HANDLE_VALUE)
                m_done = true;
#endif
        }

    DirList::~DirList()
    {
#ifdef LINUX
        if(m_dir)
            closedir(m_dir);
#endif
#ifdef WINDOWS
        if(m_handle != INVALID_HANDLE_VALUE)
            FindClose(m_handle);
#endif
    }

    bool DirList::Next(Str& fname)
    {
#ifdef LINUX
        if(m_dir == 0)
            return false;

        struct dirent* de = readdir(m_dir);
        if(!de)
            return false;

        fname = de->d_name;
        return true;
#endif

#ifdef WINDOWS
        if(m_done)
            return false;

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        fname = converter.to_bytes(m_findData.cFileName).c_str();

        m_done = (FindNextFile(m_handle, &m_findData) == 0);
        return true;
#endif
    }

    ////////////////////////////////////////////////////////////////////////////////
    PushWorkingDirectory::PushWorkingDirectory(const char* filename)
        : m_old(GetWorkingDirectory())
    {
        if(filename && *filename != '\0')
            ChangeWorkingDirectory(filename);
    }
    
    PushWorkingDirectory::~PushWorkingDirectory()
    {
        if(!m_old.empty())
            ChangeWorkingDirectory(m_old.c_str());
    }
    

    bool StartsWithPath(const Str& wholePath, const Str& partPath)
    {
        if (StrNCaseCmp(wholePath.c_str(), partPath.c_str(), partPath.length()) == 0)
        {
            auto const after = partPath.length();
            if (wholePath[after] == FILE_SEP)
                return true;
        }
        return false;
    }
}

