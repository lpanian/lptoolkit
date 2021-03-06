#pragma once
#ifndef INCLUDED_toolkit_str_hh
#define INCLUDED_toolkit_str_hh

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <ostream>

#include "mathcommon.hh"
#include "compat.hh"

namespace lptk
{

template<MemPoolId POOL>
class StringImpl;

template<size_t SIZE>
class ArrayString
{
    // Data members
    size_t m_length;
    char m_data[SIZE];
public:
    ArrayString()
        : m_length(0)
    {
        m_data[0] = '\0';
    }
    template<unsigned int OTHER_SIZE>
    ArrayString(const ArrayString<OTHER_SIZE>& other)
    {
        strncpy(m_data, other.m_data, Min(SIZE - 1, OTHER_SIZE - 1));
        m_data[SIZE-1] = '\0';
        m_length = Min(other.m_length, SIZE - 1);
    }

    ArrayString(const char* sz)
    {
        strncpy(m_data, sz, SIZE - 1);
        m_data[SIZE-1] = '\0';
        m_length = strlen(m_data);
    }

    template<MemPoolId POOL>
    ArrayString(const StringImpl<POOL>& other)
    {
        const auto copyLen = Min(SIZE-1, size_t(other.length() + 1));
        strncpy(m_data, other.c_str(), copyLen );
        m_data[copyLen] = '\0';
        m_length = copyLen - 1;
    }

    // TODO: operator= for String... 
    ArrayString& operator=(const char* szStr)
    {
        strncpy(m_data, szStr, SIZE-1);
        m_data[SIZE-1] = '\0';
        m_length = strlen(m_data);
        return *this;
    }

    ArrayString& operator=(const ArrayString& other)
    {
        if(this != &other)
        {
            strncpy(m_data, other.c_str(), SIZE - 1);
            m_data[SIZE - 1] = '\0';
            m_length = other.m_length;
        }
        return *this;
    }

    ArrayString& operator+=(const char* szStr)
    {
        const auto numRemaining = SIZE - 1 - m_length;
        const auto addedLen = strlen(szStr);
        const auto numToCopy = lptk::Min(addedLen, numRemaining);

        strncat(&m_data[m_length], szStr, numToCopy);
        m_length += numToCopy;
        m_data[m_length] = '\0';

        return *this;
    }

    char& operator[](int idx) { return write_str()[idx]; }
    const char& operator[](int idx) const { return c_str()[idx]; }

    const char* c_str() const { return m_data; }
    size_t capacity() const { return SIZE; }
    size_t length() const { return m_length; }
    bool empty() const { return m_length == 0; }
    bool remaining_capacity() { return SIZE - m_length; }

    void clear() {
        m_data[0] = '\0';
        m_length = 0;
    }

    char* write_str() { return m_data; }
    void resize(size_t length) 
    {
        m_length = lptk::Min(SIZE - 1, length);
        m_data[m_length] = '\0';
    }
    // used to fix the string after passing it to a function that writes directly to write_str()
    void resize_after_write() { resize(strnlen(m_data, SIZE)); }

    int cmp(const char* other) const { return strcmp(c_str(), other); }
    int cmp(const ArrayString& other) const { if(m_data == other.m_data) return 0; else return cmp(other.c_str()); }
    int ncmp(const char* other, int len) const { return strncmp(c_str(), other, len); }
    int ncmp(const ArrayString& other, int len) const { return strncmp(c_str(), other.c_str(), len); }
    int icmp(const char* other) const { return StrCaseCmp(c_str(), other); }
    int icmp(const ArrayString& other) const { if(m_data == other.m_data) return 0; else return icmp(other.c_str()); }
    int incmp(const char* other, int len) const { return StrNCaseCmp(c_str(), other, len); }
    int incmp(const ArrayString& other, int len) const { return StrNCaseCmp(c_str(), other.c_str(), len); }

    int find(char c) const { 
        return find(0, c);
    }

    int find(int loc, char c) const { 
        int pos = loc;
        const char* str = c_str();
        for(; pos < length(); ++pos) if(str[pos] == c) break;
        return pos >= length() ? -1 : pos;
    }

    int rfind(char c) const { 
        int pos = int(length()) - 1;
        const char* str = c_str();
        for(; pos >= 0; --pos) if(str[pos] == c) break;
        return pos;
    }

    void sub(char c, char replace)
    {
        char* str = m_data;
        for(int i = 0, len = length(); i < len; ++i)
            if(str[i] == c) str[i] = replace;
    }
};

template< MemPoolId POOL >
class StringImpl
{
    // supporting classes
    struct str_head
    {
        // data members
        uint16_t refCount;
        uint16_t length;

        str_head() : refCount(0), length(0) {}
    };

    // data members
    mutable char* m_data;
public:
    StringImpl() : m_data(nullptr) {}
    ~StringImpl()
    {
        DecRef();
    }

    StringImpl(const char* sz) : m_data(0) 
    {
        const size_t len = strlen(sz);
        CopyString(sz, len);
    }

    StringImpl(const char* sz, size_t len) : m_data(0)
    {
        CopyString(sz, len);
    }

    StringImpl(const char* start, const char* end) : m_data(0)
    {
        CopyString(start, end - start);
    }

    StringImpl(const StringImpl& other)
        : m_data(nullptr)
    {
        if(other.m_data) 
            CopyData(other.m_data);
    }

    StringImpl(StringImpl&& other) noexcept
        : m_data(nullptr)
    {
        m_data = other.m_data;
        other.m_data = nullptr;
    }

    template< int SIZE >
    StringImpl(const ArrayString<SIZE>& other)
    : m_data(nullptr)
    {
        CopyString(other.m_data, other.m_length);
    }

    StringImpl& operator=(const StringImpl& other)
    {
        if(this != &other &&
                m_data != other.m_data)
        {
            if(m_data) DecRef();
            if(other.m_data)
                CopyData(other.m_data);
        }
        return *this;
    }

    StringImpl& operator=(StringImpl&& other)
    {
        if (this != &other)
        {
            if (m_data) DecRef();
            m_data = other.m_data;
            other.m_data = nullptr;
        }
        return *this;
    }

    StringImpl operator+(const StringImpl& other) const
    {
        const uint16_t len = length() + other.length() ;
        // TODO handle overflow
        char* data = new (POOL, 4) char[sizeof(str_head) + len + 1];
        str_head* head = reinterpret_cast<str_head*>(data);
        head->length = (uint16_t)len;
        head->refCount = 1;

        char* szTarget = data + sizeof(str_head);
        strncpy(szTarget, m_data + sizeof(str_head), length());
        strncpy(szTarget + length(), other.m_data + sizeof(str_head), other.length());
        szTarget[len] = '\0';

        StringImpl str;
        str.m_data = data;
        return str;
    }

    StringImpl operator+(const char* szOther)
    {
        auto const lenOther = strlen(szOther);
        auto const len = length() + ClampCast<uint16_t>(lenOther);
        char* data = new (POOL, 4) char[sizeof(str_head) + len + 1];
        str_head* head = reinterpret_cast<str_head*>(data);
        head->length = static_cast<uint16_t>(len);
        head->refCount = 1;

        char* szTarget = data + sizeof(str_head);
        strncpy(szTarget, m_data + sizeof(str_head), length());
        strncpy(szTarget + length(), szOther, lenOther);
        szTarget[len] = '\0';

        StringImpl str;
        str.m_data = data;
        return str;
    }

    StringImpl& operator+=(const StringImpl& other)
    {
        *this = *this + other;
        return *this;
    }

    bool operator==(const StringImpl& other) const { return this->length() == other.length() && this->cmp(other) == 0; }
    bool operator!=(const StringImpl& other) const { return this->length() != other.length() || this->cmp(other) != 0; }

    const char* c_str() const { if(m_data) return m_data + sizeof(str_head); else return ""; }
    char* write_str() {
        if(m_data) {
            const str_head* head = reinterpret_cast<str_head*>(m_data);
            if(head->refCount > 1)
                *this = StringImpl<POOL>(this->c_str(), this->length());
            return m_data + sizeof(str_head);
        }
        else return nullptr; // don't want to write into a constant string
    }

    char* data() { return m_data ? m_data + sizeof(str_head) : nullptr; }
    const char* data() const { return m_data ? m_data + sizeof(str_head) : nullptr; }
    uint16_t size() const { return length(); }
    uint16_t length() const { return m_data ? reinterpret_cast<str_head*>(m_data)->length : 0; }
    uint16_t capacity() const { return length(); }
    bool empty() const { return length() == 0; }
    const char* begin() const { return c_str(); }
    const char* end() const { return c_str() + length(); }

    int cmp(const char* other) const { return strcmp(c_str(), other); }
    int cmp(const StringImpl& other) const { if(m_data == other.m_data) return 0; else return cmp(other.c_str()); }
    int ncmp(const char* other, int len) const { return strncmp(c_str(), other, len); }
    int ncmp(const StringImpl& other, int len) const { return strncmp(c_str(), other.c_str(), len); }
    int icmp(const char* other) const { return StrCaseCmp(c_str(), other); }
    int icmp(const StringImpl& other) const { if(m_data == other.m_data) return 0; else return icmp(other.c_str()); }
    int incmp(const char* other, int len) const { return StrNCaseCmp(c_str(), other, len); }
    int incmp(const StringImpl& other, int len) const { return StrNCaseCmp(c_str(), other.c_str(), len); }

    char& operator[](int idx) { return write_str()[idx]; }
    const char& operator[](int idx) const { return c_str()[idx]; }

    int find(char c) const 
    { 
        return find(0, c);
    }

    int find(int loc, char c) const 
    { 
        if(empty()) return -1;
        int pos = loc;
        const char* str = c_str();
        for(; pos < length(); ++pos) if(str[pos] == c) break;
        return pos >= length() ? -1 : pos;
    }

    int find(const char* str, int pos) const 
    { 
        if(empty() || pos < 0 || pos >= length()) return -1;
        const char* p = strstr(c_str() + pos, str);
        if(p) return int(p - c_str());
        else return -1;
    }

    int find(const StringImpl& str, int pos = 0) const 
    { 
        if(empty()) return -1;
        return find(str.c_str(), pos); 
    }

    template<typename Fn>
    int find(int pos, Fn&& fn) const
    {
        if(empty() || pos < 0 || pos >= length()) return -1;
        const char* str = c_str();
        for (; pos < length(); ++pos)
            if (fn(str[pos]))
                break;
        return pos >= length() ? -1 : pos;
    }

    int rfind(char c) const 
    { 
        if(empty()) return -1;
        int pos = int(length()) - 1;
        const char* str = c_str();
        for(; pos >= 0; --pos) if(str[pos] == c) break;
        return pos;
    }

    StringImpl substr(int start, int end = -1) const
    {
        ASSERT(start >= 0);
        if(end < 0) end = length();
        int len = end - start;
        ASSERT(len >= 0);
        if(len == 0)
            return StringImpl();
        else
            return StringImpl(c_str() + start, len);
    }

    unsigned sub(char c, char replace)
    {
        unsigned count = 0;
        if(!m_data)
            return count;

        char* str = m_data + sizeof(str_head);
        for (int i = 0, len = length(); i < len; ++i)
        {
            if (str[i] == c)
            {
                // if this string is referenced by more than this object, make a copy of this string
                // before doing a replace
                const str_head* head = reinterpret_cast<str_head*>(m_data);
                if (head->refCount > 1)
                {
                    *this = StringImpl<POOL>(this->c_str(), this->length());
                    str = m_data + sizeof(str_head);
                    ASSERT(reinterpret_cast<str_head*>(m_data)->refCount == 1);
                }

                ++count;
                str[i] = replace;
            }
        }

        return count;
    }

    unsigned sub(const StringImpl<POOL>& pattern, const char* replace)
    {
        unsigned count = 0;
        if (!m_data)
            return count;

        const auto replaceLen = strlen(replace);
        const str_head* head = reinterpret_cast<str_head*>(m_data);
        auto newLen = size_t(head->length);

        auto pos = int(0);
        while (pos >= 0)
        {
            pos = find(pattern, pos);
            if (pos >= 0)
            {
                ++count;
                newLen += replaceLen;
                newLen -= pattern.length();
                ++pos;
            }
        }
        

        StringImpl<POOL> tmp("", newLen);

        const char* srcStr = m_data + sizeof(str_head);
        char* destStr = tmp.m_data + sizeof(str_head);
        auto srcPos = int(0);
        auto destPos = int(0);
        while (srcPos < length())
        {
            const auto endPos = find(pattern, srcPos);
            if (endPos < 0)
            {
                const auto curLen = length() - srcPos;
                strncpy(&destStr[destPos], &srcStr[srcPos], curLen);
                srcPos += curLen;
                destPos += curLen;
            }
            else
            {
                const auto curLen = endPos - srcPos;
                strncpy(&destStr[destPos], &srcStr[srcPos], curLen);
                srcPos += curLen;
                destPos += curLen;
                
                strncpy(&destStr[destPos], replace, replaceLen);
                srcPos += pattern.length();
                destPos += int(replaceLen);
            }
        }

        *this = std::move(tmp);
        return count;
    }

private:
    void CopyString(const char* sz, size_t len)
    {
        ASSERT(m_data == 0);
        ASSERT(len <= std::numeric_limits<uint16_t>::max());
        if(len > std::numeric_limits<uint16_t>::max()) {
            len = std::numeric_limits<uint16_t>::max();
        }
        m_data = new (POOL, 4) char[sizeof(str_head) + len + 1];
        str_head* head = reinterpret_cast<str_head*>(m_data);
        head->refCount = 1;
        head->length = (uint16_t)len;
        char* data = m_data + sizeof(str_head);
        strncpy(data, sz, len);
        data[len] = '\0';
    }

    void CopyData(char* data)
    {
        ASSERT(m_data == 0);
        str_head* head = reinterpret_cast<str_head*>(data);
        if(head->refCount == std::numeric_limits<uint16_t>::max())
        {
            const char* sz = data + sizeof(str_head);
            CopyString(sz, head->length);
        }
        else
        {
            m_data = data;
            ++head->refCount;
        }
    }

    void DecRef()
    {
        str_head* head = reinterpret_cast<str_head*>(m_data);
        if(head)
        {
            ASSERT(head->refCount > 0);
            head->refCount--;
            if(head->refCount == 0)
            {
#ifdef DEBUG
                if (head->length > 0)
                {
                    const unsigned length = head->length;
                    memset(m_data, 0xAF, length + sizeof(str_head));
                }
#endif
                delete[] m_data;
            }
            m_data = 0;
        }
    }
};

typedef StringImpl<MEMPOOL_String> Str;

namespace literals {
#if !defined(_MSC_VER) || _MSC_VER > 1800
    inline Str operator"" _s(const char* p, size_t n) 
    {
        return Str(p, n);
    }
#endif
}

template<typename StrType>
inline size_t ComputeHash(const StrType& x) {
    size_t result = 0UL;
    const char* p = x.c_str();
    while(*p) result += 261 * *p++;
    return result;
}

template< int SIZE >
bool Printf(ArrayString<SIZE>& strDest, const char* szFormat, ... )
{
    char* szTarget = strDest.write_str();
    size_t nTarget = strDest.capacity();

    va_list arglist;
    va_start(arglist, szFormat);
    int nWritten = vsnprintf(szTarget, nTarget, szFormat, arglist);
    va_end(arglist);

    strDest.resize(nWritten);
    return nWritten == SIZE;
}

template< class StringType >
bool Printf(StringType& strDest, const char* szFormat, ... )
{
    char buf[256];

    va_list arglist;
    va_start(arglist, szFormat);
    int nWritten = vsnprintf(buf, sizeof(buf), szFormat, arglist);
    va_end(arglist);

    bool bOverflowed = false;
    if(nWritten == sizeof(buf))
    {
        for(int i = sizeof(buf) - 4; i < (int)sizeof(buf) - 1; ++i)
            buf[i] = '.';
        buf[sizeof(buf) - 5] = ' ';
        bOverflowed = true;
    }
    buf[ sizeof(buf) - 1] = '\0';
    strDest = buf;
    return bOverflowed;
}

inline Str StripExtension(const char* str)
{
    int loc = -1;
    int cur = 0;
    while(str[cur] != '\0')
    {
        if(str[cur] == '.') 
            loc = cur;
        ++cur;
    }

    if(loc >= 0)
        return Str(str, loc);
    else
        return Str();
}

template<class StringType >
inline StringType StripExtension( const StringType &str )
{
    int i = str.length() - 1;
    while(i >= 0 && str.c_str()[i] != '.')
    {
        --i;
    }

    if(i > 0) {
        return StringType(str.c_str(), i);
    } else {
        return str;
    }
}

template<class StringType>
inline const char* EndsWith( const StringType &str, const char *ending )
{
    auto const len = strlen(ending);
    auto const i = str.length() - len;
    if (i >= 0 && strcmp(&str.c_str()[i], ending) == 0)
        return &str.c_str()[i];
    return nullptr;
}

inline const char* EndsWith(const char* str, const char* ending)
{
    auto const len = strlen(ending);
    auto const i = strlen(str) - len;
    if (i >= 0 && strcmp(&str[i], ending) == 0)
        return &str[i];
    return nullptr;
}

template<class StringType>
inline const char* StartsWith( const StringType &str, const char *beginning)
{
    const size_t beginningLen = strlen(beginning);
    if (StrNCaseEqual(str.c_str(), beginning, static_cast<unsigned int>(beginningLen)))
        return str.c_str() + beginningLen;
    return nullptr;
}

inline const char* StartsWith(const char* str, const char* beginning)
{
    const size_t beginningLen = strlen(beginning);
    if(StrNCaseEqual(str, beginning, static_cast<unsigned int>(beginningLen)))
        return str + beginningLen;
    return nullptr;
}

inline const char* GetExtension( const char* szStr )
{
    if (!szStr || !*szStr)
        return nullptr;
    auto const len = strlen(szStr);
    for (size_t i = 0; i < len; ++i)
    {
        const size_t index = len - 1 - i;
        if (szStr[index] == '.')
        {
            return &szStr[index + 1];
        }
    }
    return nullptr;
}

template<MemPoolId POOL>
inline std::ostream& operator<<(std::ostream& out, const StringImpl<POOL>& str)
{
    out << str.c_str();
    return out;
}

}

#endif

