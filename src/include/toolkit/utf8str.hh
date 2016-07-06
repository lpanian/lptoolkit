#pragma once
#ifndef INCLUDED_TOOLKIT_UTF8STR_HH
#define INCLUDED_TOOLKIT_UTF8STR_HH

#include <cstdint>
#include <tuple>

namespace lptk
{
    using CharPoint = std::uint_fast32_t;

    // return length in charpoints - we do no automatic combining of characters here.
    size_t utf8_strlen(const char* utf8);

    // consume enough utf8 chars in the argument to produce a code point, and return the code point and the new string pointer.
    // This also ignores any invalid code points, and returns 0 when there is a bad continuation byte.
    std::pair<CharPoint, const char*> utf8_decode(const char* utf8);

    // pseudo-container for range-based iteration
    class utf8range
    {
    public:
        utf8range(const char* utf8) : m_utf8(utf8) {}

        class iterator
        {
            std::pair<CharPoint, const char*> m_pos = std::make_pair(0, nullptr);
        public:
            typedef CharPoint value_type;

            iterator() = default;
            iterator(const std::pair<CharPoint, const char*>& start) : m_pos(start) {}
            iterator(const iterator& other) = default;
            iterator(iterator&& it) : m_pos(std::move(it.m_pos)) {
                it.m_pos = std::make_pair(0, nullptr);
            }
            ~iterator() = default;

            iterator& operator=(iterator&& other) {
                if (this != &other)
                {
                    m_pos = std::move(other.m_pos);
                    other.m_pos = std::make_pair(0, nullptr);
                }
                return *this;
            }
            iterator& operator=(const iterator& other) = default;

            bool operator==(const iterator& other) const { return m_pos == other.m_pos; }
            bool operator!=(const iterator& other) const { return m_pos != other.m_pos; }

            iterator& operator++() {
                next();
                return *this;
            }

            iterator operator++(int) {
                iterator result = *this;
                next();
                return result;
            }

            value_type operator*() const { return m_pos.first; }

        private:
            void next()
            {
                if (m_pos.second && *m_pos.second)
                {
                    m_pos = utf8_decode(m_pos.second);
                }
                else
                {
                    m_pos = std::make_pair(0, nullptr);
                }
            }
        };

        iterator begin() const {
            return iterator(utf8_decode(m_utf8));
        }
        iterator end() const {
            return iterator();
        }
    private:
        const char* const m_utf8 = nullptr;
    };
}

#endif

