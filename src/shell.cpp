#include "toolkit/shell.hh"

#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <ostream>

#include "toolkit/dynary.hh"

#if defined(LINUX)
#include <unistd.h>
#endif

#if defined(WINDOWS)
#include <shellapi.h>
#include <strsafe.h>
#include <codecvt>
#endif

namespace lptk
{
#if defined(WINDOWS)
    static void ExtractProgramAndParams(
        const char* origCmd,
        std::wstring& cmdStr,
        const WCHAR*& program,
        const WCHAR*& params)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
        cmdStr = convert.from_bytes(origCmd);

        size_t pos = 0;
        while (pos != cmdStr.npos)
        {
            pos = cmdStr.find('/');
            if (pos != cmdStr.npos)
                cmdStr.replace(pos, 1, 1, '\\');
        }

        program = cmdStr.c_str();
        params = nullptr;

        size_t loc = cmdStr.find(' ');
        if(loc != cmdStr.npos) 
        {
            cmdStr[loc] = '\0';
            ++loc;
            if(loc >= 0 && loc < cmdStr.length())
                params = &cmdStr[loc];
        }
    }
#endif

    bool LaunchProgram(const char* command)
    {

#if defined(LINUX)
        Str cmdStr = command;
        const char* program = &cmdStr[0];
        DynAry<char*> args;

        int loc = cmdStr.find(' ');
        if(loc >= 0) 
        {
            cmdStr[loc] = '\0';
            ++loc;
            while(loc >= 0 && loc < cmdStr.length())
            {
                if(cmdStr[loc] == ' ')
                    ++loc;
                else
                {
                    args.push_back(&cmdStr[loc]);
                    loc = cmdStr.find(' ');
                    if(loc >= 0)
                        cmdStr[loc] = '\0';
                }
            }
        }

        args.push_back(nullptr);

        fflush(NULL);
        pid_t pid = fork();
        if(pid == -1)
            return false;
        if(pid == 0)
        {
            if(execv(program, &args[0]) == -1)
            {
                fprintf(stderr, "execv error: %s\n", strerror(errno));
                exit(1);
            }
            return false; // unnecessary, but makes compiler happy
        }
        else
        {
            fflush(NULL);
            return true;
        }
#elif defined(WINDOWS)
        std::wstring commandCopy;
        const WCHAR* program = nullptr;
        const WCHAR* params = nullptr;
        ExtractProgramAndParams(command, commandCopy, program, params);

        SHELLEXECUTEINFO shexi;
        bzero(&shexi, sizeof(shexi));
        shexi.cbSize = sizeof(shexi);
        shexi.lpVerb = L"open";
        shexi.lpFile = program;
        shexi.lpParameters = params;
        shexi.nShow = SW_SHOW;
        
        if (ShellExecuteEx(&shexi) == TRUE)
            return true;
        else
        {
            std::wcerr << "Failed to execute " << program << "with params: " << params << "\n";
            return false;
        }
#else
#error "Missing LaunchProgram implementation"
#endif
    }
    

    // TODO: LINUX version of RunProgram
#if defined(WINDOWS)

    static void PrintLastError()
    {
        LPVOID msgBuf;
        auto const dw = GetLastError();
    
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&msgBuf,
            0, NULL);

        std::wcerr << "Error (" << dw << "): " << (LPCTSTR)msgBuf << "\n";

        LocalFree(msgBuf);
    }

    int RunProgram(const char* command, lptk::Str* programOutput)
    {
        std::wstring cmd;
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
        cmd = convert.from_bytes(command);

        // Handles for piping output
        SECURITY_ATTRIBUTES sa;
        bzero(&sa, sizeof(sa));
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE hOutRead, hOutWrite;
        if (!CreatePipe(&hOutRead, &hOutWrite, &sa, 0))
        {
            std::wcerr << "Failed to create stdout pipes for child process: \"" << cmd << "\"\n";
            PrintLastError();
            return 1;
        }
        auto closePipeRead = at_scope_exit([&hOutRead]{
            CloseHandle(hOutRead);
        });
        auto closePipeWrite = at_scope_exit([&hOutWrite]{
            CloseHandle(hOutWrite);
        });
        SetHandleInformation(hOutRead, HANDLE_FLAG_INHERIT, 0);

        PROCESS_INFORMATION pi;
        bzero(&pi, sizeof(pi));

        STARTUPINFO si;
        bzero(&si, sizeof(si));
        si.cb = sizeof(si);
        si.hStdOutput = hOutWrite;
        si.hStdError = hOutWrite;
        si.dwFlags = STARTF_USESTDHANDLES;

        // create process may modify the command buffer...so just copy it.
        // see windows docs on CreateProcess for why this size is used
        TCHAR winCmd[32768]; 
        StringCbCopy(winCmd, sizeof(winCmd), cmd.c_str());
        
        if(!CreateProcess(NULL,
            winCmd,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si,
            &pi))
        {
            std::wcerr << "CreateProcess failed for \"" << command << "\"\n";
            PrintLastError();
            return 1;
        }

        auto closeProcess = at_scope_exit([&pi]
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        });

        // Close the write side of the pipe so the process can end (otherwise 
        // ReadFile will block sometimes)
        closePipeWrite.trigger();

        lptk::DynAry<CHAR> outChunks;
        DWORD exitCode = 0;
        do
        {
            // pump pipes
            CHAR chBuf[4096];
            DWORD dwRead;
            if (ReadFile(hOutRead, chBuf, ARRAY_SIZE(chBuf), &dwRead, NULL))
            {
                if (programOutput && dwRead > 0)
                {
                    outChunks.insert(outChunks.end(), &chBuf[0], &chBuf[dwRead]);
                }
            }
        } while (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode == STILL_ACTIVE);

        if (programOutput && !outChunks.empty())
        {
            outChunks.push_back('\0');
            *programOutput = lptk::Str(outChunks.begin(), outChunks.end());
        }

        return static_cast<int>(exitCode);
    }
#endif
}
