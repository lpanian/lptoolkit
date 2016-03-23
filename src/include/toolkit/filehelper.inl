#pragma once

#include <cstdio>

namespace lptk
{
template<class Container>
inline Container ReadFile(const char* filename)
{
    auto const sysFilename = ToSystemFilename(filename);
    FILE* fp = nullptr;
#ifdef USING_VS
    if (0 != fopen_s(&fp, sysFilename.c_str(), "rb"))
        return Container();
#else
    fp = fopen(sysFilename.c_str(), "rb");
#endif
    if (!fp) return Container();
    auto closeOnExit = at_scope_exit([&] { fclose(fp); });

    fseek(fp, 0, SEEK_END);
    const size_t fileSize = ftell(fp);
    rewind(fp);

    Container buffer(fileSize);
    size_t numRead = fread(buffer.data(), 1, buffer.size(), fp);
    if (numRead != fileSize)
        return Container();
    return buffer;
}

template<class Container>
inline bool WriteFile(const char* filename, Container&& buffer)
{
    FILE* fp = nullptr;
#ifdef USING_VS
    if (0 != fopen_s(&fp, filename, "wb"))
        return false;
#else
    fp = fopen(filename, "wb");
#endif
    if (!fp) return false;
    auto closeOnExit = at_scope_exit([&] {fclose(fp); });

    size_t numWritten = fwrite(buffer.data(), 1, buffer.size(), fp);
    if (numWritten != buffer.size())
        return false;
    return true;

}

}

