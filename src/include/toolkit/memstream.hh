#pragma once
#ifndef INCLUDED_TOOLKIT_MEMSTREAM_HH
#define INCLUDED_TOOLKIT_MEMSTREAM_HH

#include <type_traits>
#include "vec.hh"
#include "common.hh"
#include "color.hh"

namespace lptk
{

    ////////////////////////////////////////////////////////////////////////////////
    namespace MemFormatFlag
    {
        enum Type : uint32_t
        {
            FLAG_BigEndianData = 0x1, // data being read is big-endian, convert to native.
            FLAG_LittleEndianData = 0x2, // data being read is little-endian, convert to native
            FLAG_AlignedAccess = 0x4, // align to alignof(T) when putting or getting.
        };
    }



    ////////////////////////////////////////////////////////////////////////////////
    class MemWriter
    {
    public:
        MemWriter();
        MemWriter(char* buffer, size_t size, int flags = 0);

        explicit operator bool() const { return !Full() && !Error(); }

        char* Put(const void* bytes, size_t size, bool swapEndian = true);
        char* Advance(size_t size);
        char* AlignedAdvance(size_t align);

        template<class T>
        char* Put(const T& t, bool swapEndian = true) {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "type is not numeric");
            if (0 != (m_flags & MemFormatFlag::FLAG_AlignedAccess))
                AlignedAdvance(alignof(T));
            return Put(&t, sizeof(t), swapEndian);
        }
        template<class T>
        char* PutVec3(const vec3<T>&, bool swapEndian = true);
        char* PutColor(const Color&, bool swapEndian = true);
        char* PutColorRGBA(ColorRGBA);

        size_t GetPos() const { return m_pos; }

        bool Full() const { return m_s == m_pos; }
        bool Error() const { return m_error; }
    private:
        char* m_b = nullptr;
        size_t m_s = 0;
        size_t m_pos = 0;
        bool m_error = true;
        int m_flags = 0;
    };



    ////////////////////////////////////////////////////////////////////////////////
    class MemReader
    {
    public:
        MemReader();
        MemReader(const char* buffer, size_t size, int flags = 0);
        explicit operator bool() const { return !Empty() && !Error(); }

        void Get(void* bytes, size_t size, bool swapEndian = true);
        void Consume(size_t size);
        void AlignedConsume(size_t align); // consume bytes so the next value to bread has the given alignment.

        template<class T>
        vec3<T> GetVec3(bool swapEndian = true);
        Color GetColor(bool swapEndian = true);
        ColorRGBA GetColorRGBA();

        template<class T>
        T Get(bool swapEndian = true) {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "type is not numeric");
            if (0 != (m_flags & MemFormatFlag::FLAG_AlignedAccess))
                AlignedConsume(alignof(T));
            T result;
            Get(&result, sizeof(T), swapEndian);
            return result;
        }

        auto GetPos() const { return m_pos; }
        auto GetSize() const { return m_s; }
        auto GetPtr() const { return m_b + m_pos; }

        auto GetTopSize() const { return m_topSize; }
        auto GetTopPtr() const { return m_top; }

        bool Empty() const { return m_s == m_pos; }
        bool Error() const { return m_error; }

        // Get a subreader starting at the same offset we're currently at in this reader, with whatever
        // the remaining size is.
        MemReader GetSubReader(int flags = 0) const;

        // Get a subreader starting at the specified offset and requestedSize, if that size
        // is less than or equal to the size in this reader after the offset.
        MemReader GetSubReader(size_t offset, size_t requestedSize, int flags = 0) const;

        // Get a reader at the offset and requested size of the topmost reader. This is good
        // for reading files that give offsets from the top of the file.
        MemReader GetTopReader(size_t offset, size_t requestedSize, int flags = 0) const;
    private:
        const char* m_top = nullptr;
        size_t m_topSize = 0;
        const char* m_b = nullptr;
        size_t m_s = 0;
        size_t m_pos = 0;
        bool m_error = true;
        int m_flags = 0;
        MemReader(const char* top, size_t topSize, const char* buffer, size_t size, int flags);
    };



    // Helpers specifically for vector.
    // TODO: This seems out of place here and should probably be moved elsewhere.
    template<class T>
    char* MemWriter::PutVec3(const vec3<T>& v, bool swapEndian)
    {
        auto start = Put<T>(v.x, swapEndian);
        Put<T>(v.y, swapEndian);
        Put<T>(v.z, swapEndian);
        return start;
    }

    template<class T>
    vec3<T> MemReader::GetVec3(bool swapEndian)
    {
        vec3<T> result;
        result.x = Get<T>(swapEndian);
        result.y = Get<T>(swapEndian);
        result.z = Get<T>(swapEndian);
        return result;
    }

}

#endif

