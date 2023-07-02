#pragma once

#include <fw/core/types.h>
#include <assert.h>
#include <cstring> // for memcpy

namespace fw
{
    enum class alloc_strategy {
        NEXT_ALLOC_USE_HEAP, // Currently allocated on stack, next alloc will be on the heap
        HEAP                 // Currently allocated on the heap, next alloc will be on the heap
    };

    template<typename CharT = char>
    class basic_string {

    protected: CharT* m_ptr;          // Pointer to the buffer (static or dynamic)
    protected: u16_t  m_capacity;     // Buffer size - 1
    protected: u16_t  m_length;       // String length (excluding terminal string)
    protected: alloc_strategy m_alloc_strategy;

    public:
        basic_string()
            : m_alloc_strategy(alloc_strategy::HEAP)
            , m_capacity(0)
            , m_length(0)
            , m_ptr(nullptr)
        {}

        basic_string(const CharT* str)
            : m_alloc_strategy(alloc_strategy::HEAP)
            , m_length(strlen(str))
            , m_capacity(0)
            , m_ptr(nullptr)
        {
            m_ptr = expand_capacity_to_fit(m_length);
            memcpy(m_ptr, str, m_length);
            m_ptr[m_length] = 0;
        }

        basic_string(const basic_string& other)
            : basic_string(other.c_str())
        {}

        basic_string& operator=(const basic_string& other)
        {
            if( m_capacity < other.m_capacity) m_ptr = expand_capacity_to_fit(other.m_length + 1);
            memcpy(m_ptr, other.m_ptr, other.m_length);
            m_length = other.m_length;
            m_capacity = other.m_capacity;
            m_ptr[m_length] = 0;
            return *this;
        }

    protected:
        basic_string(CharT* data, u16_t size, u16_t length, alloc_strategy strategy)
            : m_alloc_strategy(strategy)
            , m_capacity(size-1)
            , m_length(length)
            , m_ptr(data)
        {}


    public:
        ~basic_string()
        {
            if( m_alloc_strategy == alloc_strategy::HEAP )
            {
                delete[] m_ptr;
            }
        }

        inline bool heap_allocated() const
        { return m_alloc_strategy == alloc_strategy::HEAP; }

        inline const char* c_str() const
        { return m_ptr != nullptr ? const_cast<const char*>( m_ptr ) : ""; }

        inline basic_string& append(const CharT* str, u16_t length)
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
        inline basic_string& append(const basic_string& str)
        { return append(str.m_ptr, str.m_length); }

        inline basic_string& append(const CharT* str)
        { return append(str, strlen(str)); }

        template<typename ...Args>
        inline u16_t append_fmt(const char* _format, Args... args )
        {
            return m_length = vsnprintf(m_ptr+m_length, m_capacity-m_length, _format, args...);
        }

        /** provided to easily switch to/from std::string */
        inline basic_string& push_back(CharT str)
        { return append(&str, 1); }

        inline u16_t capacity() const
        { return m_capacity; }

        inline u16_t length() const
        { return m_length; }

        inline bool is_empty() const
        { return m_length == 0; }

        inline void clear()
        {
            m_length = 0;
            if( m_ptr != nullptr) m_ptr[0] = 0;
        }

    private:
        /**
         * Expand the buffer to the closest power of two of the desired size.
         */
        CharT* expand_capacity_to_fit(u16_t desired_capacity)
        {
            assert(desired_capacity > m_capacity );

            static_assert( std::is_same<decltype(desired_capacity), u16_t>()); // code below needs to be adapted if integer is not u16

            // compute the next highest power of 2 of 32-bit
            // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
            u16_t new_buf_size = desired_capacity;
            //new_buf_size--; // capacity is buffer_size - 1
            new_buf_size |= new_buf_size >> 1;
            new_buf_size |= new_buf_size >> 2;
            new_buf_size |= new_buf_size >> 4;
            new_buf_size |= new_buf_size >> 8;
            new_buf_size |= new_buf_size >> 16;
            new_buf_size++;

            CharT* new_ptr = new CharT[new_buf_size];

            if( m_ptr )
            {
                memcpy(new_ptr, m_ptr, m_length+1); // We only copy the string + null char
            }

            if (m_alloc_strategy == alloc_strategy::HEAP)
            {
                delete[] m_ptr;
            }
            else
            {
                m_alloc_strategy = alloc_strategy::HEAP;
            }
            m_capacity = new_buf_size - 1;

            return new_ptr;
        }
    };

    /**
     * Stack allocated string.
     * Switches to dynamic allocations when stack buffer is too small.
     *
     * Buffer size and string length are stored in an unsigned integer (1 byte)
     */
    template<typename CharType, u16_t STATIC_BUF_SIZE>
    class inline_string : public basic_string<CharType> {
    private:
        static_assert(STATIC_BUF_SIZE != 0);
        CharType m_static_buf[STATIC_BUF_SIZE]; // Static buffer

    public:
        inline_string(): basic_string<CharType>(m_static_buf, STATIC_BUF_SIZE, 0, alloc_strategy::NEXT_ALLOC_USE_HEAP)
        {
            m_static_buf[0] = '\0';
        }

        inline_string(const CharType *str) : basic_string<CharType>(m_static_buf, STATIC_BUF_SIZE, strlen(str), alloc_strategy::NEXT_ALLOC_USE_HEAP)
        {
            strncpy(m_static_buf, str, this->m_length);
            m_static_buf[this->m_length] = 0;
        }

        inline_string(CharType *str, u16_t length) : basic_string<CharType>(m_static_buf, STATIC_BUF_SIZE, length, alloc_strategy::NEXT_ALLOC_USE_HEAP)
        {
            strncpy(m_static_buf, str, this->m_length);
            m_static_buf[this->m_length] = 0;
        }

        inline_string(const basic_string<CharType>& other): inline_string()
        {
            this->clear();
            this->append(other.c_str(), other.length());
        }
    };

    // Define some aliases

    using string    = basic_string<char>;
    using string8   = inline_string<char, 8>;
    using string16  = inline_string<char, 16>;
    using string32  = inline_string<char, 32>;
    using string64  = inline_string<char, 64>;
    using string128 = inline_string<char, 128>;
    using string256 = inline_string<char, 256>;
}