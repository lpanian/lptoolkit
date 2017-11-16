#pragma once
#ifndef INCLUDED_toolkit_ostreamformat_HH
#define INCLUDED_toolkit_ostreamformat_HH

#include <ostream>

namespace lptk
{
    // silly ostream format helpers
    namespace ostreamformat
    {
        struct spaces
        {
            int count;
            explicit spaces(int c) : count(4 * c) {}
        };

        inline std::ostream& operator<<(std::ostream& out, const spaces& s)
        {
            for (int i = 0; i < s.count; ++i)
                out << " ";
            return out;
        }
    }
}

#endif

