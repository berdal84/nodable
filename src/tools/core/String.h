#pragma once

#include <cassert>
#include <cstring> // for memcpy
#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>
#include <utility> // std::move

namespace tools
{
    enum class Allocation_Strategy {
        NEXT_ALLOC_USE_HEAP, // Currently allocated on stack, next alloc will be on the heap
        HEAP                 // Currently allocated on the heap, next alloc will be on the heap
    };

    template<typename Char_Type = char>
    class Basic_String
    {

    protected: Char_Type* m_ptr;          // Pointer to the buffer (static or dynamic)
    protected: size_t m_length;       // String length (excluding terminal string)
    protected: size_t m_capacity;     // Buffer size - 1
    protected: Allocation_Strategy m_alloc_strategy;

    public:
        Basic_String()
            : m_alloc_strategy(Allocation_Strategy::HEAP)
            , m_capacity(0)
            , m_length(0)
            , m_ptr(nullptr)
        {}

        explicit Basic_String(const Char_Type* str)
            : m_alloc_strategy(Allocation_Strategy::HEAP)
            , m_length(strlen(str))
            , m_capacity(0)
            , m_ptr(nullptr)
        {
            if( m_length > 0 )
            {
                m_ptr = expand_capacity_to_fit(m_length);
                memcpy(m_ptr, str, m_length);
                m_ptr[m_length] = 0;
            }
        }

        Basic_String(const Basic_String& other)
            : Basic_String(other.c_str())
        {}

        Basic_String(Basic_String&& other) noexcept
            : m_alloc_strategy(Allocation_Strategy::NEXT_ALLOC_USE_HEAP)
            , m_length(0)
            , m_capacity(0)
            , m_ptr(nullptr)
        {
            *this = std::move(other);
        }

        Basic_String& operator=(Basic_String&& other) noexcept
        {
            if ( this == &other )
                return *this;

            if( m_alloc_strategy == Allocation_Strategy::HEAP )
            {
                delete[] m_ptr;
                m_ptr = other.m_ptr;
                m_length = other.m_length;
                m_capacity = other.m_capacity;
            }
            else
            {
                append(other); // may use heap or stack depending on capacity
            }

            other.m_length = 0;
            other.m_capacity = 0;
            other.m_ptr = nullptr;

            return *this;
        }

        Basic_String& operator=(const Basic_String& other)
        {
            if( this == &other)
                return *this;

            if( m_capacity < other.m_capacity)
                m_ptr = expand_capacity_to_fit(other.m_length);

            memcpy(m_ptr, other.m_ptr, other.m_length);
            m_length = other.m_length;
            m_capacity = other.m_capacity;
            m_ptr[m_length] = 0;
            return *this;
        }

    protected:
        Basic_String(Char_Type* data, size_t capacity, size_t length, Allocation_Strategy strategy = Allocation_Strategy::NEXT_ALLOC_USE_HEAP)
            : m_alloc_strategy(strategy)
            , m_capacity(capacity )
            , m_length(length)
            , m_ptr(data)
        {}


    public:
        ~Basic_String()
        {
            if( m_alloc_strategy == Allocation_Strategy::HEAP && m_ptr != nullptr)
            {
                delete[] m_ptr;
            }
        }

        bool heap_allocated() const
        { return m_alloc_strategy == Allocation_Strategy::HEAP; }

        const char* data() const
        { return m_ptr != nullptr ? const_cast<const char*>(m_ptr) : ""; }

        const char* c_str() const
        { return data(); }

        Basic_String& append(const Char_Type* str, size_t length)
        {
            if( m_capacity < m_length + length )
            {
                m_ptr = expand_capacity_to_fit(m_length + length);
            }
            memcpy(m_ptr + m_length, str, length);
            m_length += length;
            m_ptr[m_length] = 0;
            return *this;
        }
        Basic_String& append(const Basic_String& str)
        { return append(str.m_ptr, str.m_length); }

        Basic_String& append(const Char_Type* str)
        { return append(str, strlen(str)); }

        template<typename ...Args>
        size_t append_fmt(const char* _Format, Args...args )
        { return m_length = snprintf(m_ptr+m_length, m_capacity-m_length, _Format, args... ); }

        size_t append_fmt(const char* _str )
        { return m_length = snprintf(m_ptr+m_length, m_capacity-m_length, "%s", _str ); }

        /** provided to easily switch to/from std::string */
        Basic_String& push_back(Char_Type str)
        { return append(&str, 1); }

        size_t capacity() const
        { return m_capacity; }

        size_t length() const
        { return m_length; }

        bool is_empty() const
        { return m_length == 0; }

        void clear()
        {
            m_length = 0;
            if( m_ptr != nullptr) m_ptr[0] = 0;
        }

        bool equals(const Basic_String& other) const {
            return m_length == other.m_length && strcmp(c_str(), other.c_str()) == 0;
        }

    private:
        /**
         * Expand the buffer to the closest power of two of the desired set_size.
         */
        Char_Type* expand_capacity_to_fit(size_t desired_capacity)
        {
            assert(desired_capacity > m_capacity );

            // compute the next highest power of 2 of 64-bit

            // based on https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
            size_t new_buf_size = desired_capacity;
            //new_buf_size--; // capacity is buffer_size - 1
            new_buf_size |= new_buf_size >> 1;
            new_buf_size |= new_buf_size >> 2;
            new_buf_size |= new_buf_size >> 4;
            new_buf_size |= new_buf_size >> 8;
            new_buf_size |= new_buf_size >> 16;

            new_buf_size++;

            Char_Type* new_ptr = new Char_Type[new_buf_size];

            if( m_ptr )
            {
                memcpy(new_ptr, m_ptr, m_length+1); // We only copy the string + null char
            }

            if (m_alloc_strategy == Allocation_Strategy::HEAP)
            {
                delete[] m_ptr;
            }
            else
            {
                m_alloc_strategy = Allocation_Strategy::HEAP;
            }
            m_capacity = new_buf_size - 1;

            return new_ptr;
        }
    };

    /**
     * Stack allocated string.
     * Switches to dynamic allocations when stack buffer is too small.
     *
     * Buffer set_size and string length are stored in an unsigned integer (1 byte)
     */
    template<size_t STATIC_BUF_SIZE, typename CharType = char>
    class Inline_String : public Basic_String<CharType> {
    private:
        static_assert(STATIC_BUF_SIZE >= 8);
        CharType m_static_buf[STATIC_BUF_SIZE]; // Static buffer

    public:
        Inline_String(): Basic_String<CharType>(m_static_buf, STATIC_BUF_SIZE-1, 0)
        { m_static_buf[0] = '\0'; }

        Inline_String(CharType *str, size_t length) : Basic_String<CharType>(m_static_buf, STATIC_BUF_SIZE-1, 0)
        { this->append(str, length); }

        Inline_String(const CharType *str) : Inline_String(const_cast<char*>(str), strlen(str))
        {}

        Inline_String(const Basic_String<CharType>& other): Inline_String()
        {  this->append(other.c_str(), other.length()); }
    };

    // Define some aliases
    using String     = Basic_String<char>;
    using String_8   = Inline_String<8>;
    using String_16  = Inline_String<16>;
    using String_32  = Inline_String<32>;
    using String_64  = Inline_String<64>;
    using String_128 = Inline_String<128>;
    using String_256 = Inline_String<256>;
    using String_512 = Inline_String<512>;
}