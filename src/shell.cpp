#include "toolkit/shell.hh"

#include <cstdio>
#include <cstdlib>
#include "toolkit/dynary.hh"

#if defined(LINUX)
#include <unistd.h>
#endif

#if defined(WINDOWS)
#include <shellapi.h>
#endif

namespace lptk
{
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
        Str cmdStr = command;
        cmdStr.sub('/', '\\');

        const char* program = &cmdStr[0];
        const char* params = nullptr;

        int loc = cmdStr.find(' ');
        if(loc >= 0) 
        {
            cmdStr[loc] = '\0';
            ++loc;
            if(loc >= 0 && loc < cmdStr.length())
                params = &cmdStr[loc];
        }

        SHELLEXECUTEINFO shexi;
        bzero(&shexi, sizeof(shexi));
        shexi.cbSize = sizeof(shexi);
        shexi.lpVerb = "open";
        shexi.lpFile = program;
        shexi.lpParameters = params;
        shexi.nShow = SW_SHOW;
        
        if (ShellExecuteEx(&shexi))
            return true;
        else
        {
            fprintf(stderr, "failed to execute %s\n", program);
            return false;
        }
    #else
    #error "Missing LaunchProgram implementation"
    #endif
    }
}
