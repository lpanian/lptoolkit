#pragma once
#ifndef INCLUDED_toolkit_shell_HH
#define INCLUDED_toolkit_shell_HH

#include "str.hh"

namespace lptk
{
    // TODO var arguments?
   
    // Launch but only launch. Fire and forget, only return false if the launch failed.
    bool LaunchProgram(const char* command);

    // Launch process, but don't return until the process has finished.
    bool RunProgram(const char* command, lptk::Str* programOutput = nullptr);
}

#endif

