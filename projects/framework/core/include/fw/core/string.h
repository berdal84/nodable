#pragma once

#include <string>
#include <cstring>
#include <fw/core/types.h>
#include <xxhash/xxhash64.h>
#include <assert.h>

namespace fw
{
    template<typename CharT, u16_t STACK_BUF_SIZE> class Str; // forward decl

    // define some aliases

    typedef Str<char, 8>   Str8;
    typedef Str<char, 16>  Str16;
    typedef Str<char, 32>  Str32;
    typedef Str<char, 64>  Str64;
    typedef Str<char, 128> Str128;
    typedef Str<char, 256> Str256;
    typedef Str<char, 512> Str512;

    /**
     * Stack allocated string.
     * Switches to dynamic allocations when stack buffer is too small.
     *
     * Buffer size and string length are stored in an unsigned integer (1 byte)
     */
    template<typename CharType, u16_t STATIC_BUF_SIZE>
    class Str
    {
        static_assert(STATIC_BUF_SIZE != 0);
        private: CharType* m_ptr;                         // Pointer to the buffer (static or dynamic)
        private: u16_t     m_buf_size;                    // Buffer size
        private: u16_t     m_length;                      // String length
        private: CharType  m_static_buf[STATIC_BUF_SIZE]; // Static buffer

        public: Str()
            : m_buf_size(STATIC_BUF_SIZE)
            , m_ptr(m_static_buf)
            , m_length(0)
        {
            m_static_buf[0] = '\0';
        }

        public: Str(const CharType* str)
            : m_buf_size(STATIC_BUF_SIZE)
            , m_ptr(m_static_buf)
            , m_length(strlen(str))
        {
            assert(("str is too long, use larger buffer Str<N> (stack) or use Str (heap)", m_length + 1 <= m_buf_size));
            memcpy(m_ptr, str, m_length);
            m_ptr[m_length] = '\0';
        }

        public: ~Str()
        {
            if( is_on_heap() )
            {
                delete[] m_ptr;
            }
        }

        public: inline bool is_on_heap() const
        {
            return ((void*)m_ptr) != ((void*)m_static_buf);
        }

        public: inline const char* c_str() const
        {
            return const_cast<const char*>( m_ptr );
        }

        private: void enlarge_buffer_to_fit(u16_t desired_buf_size)
        {
            static_assert( std::is_same<typeof(desired_buf_size), u16_t>()); // code below needs to be adapted if integer is greater

            // compute the next highest power of 2 of 32-bit
            // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
            u16_t size = desired_buf_size;

            size--;
            size |= size >> 1;
            size |= size >> 2;
            size |= size >> 4;
            size |= size >> 8;
            size |= size >> 16;
            size++;

            CharType* new_ptr = new CharType[size]; // TODO: use the next power of two
            memcpy(new_ptr, m_ptr, m_buf_size);
            if (is_on_heap()) delete[] m_ptr;
            m_ptr = new_ptr;
            m_buf_size = desired_buf_size;
        }
        public: inline void append(CharType c)
        {
            if( m_buf_size <= m_length + 1 ) enlarge_buffer_to_fit(m_length + 1);
            m_ptr[m_length] = c;
            ++m_length;
            m_ptr[m_length] = 0;
        }

        public: inline void append(const CharType* str, u16_t n)
        {
            if( m_buf_size <= m_length + n ) enlarge_buffer_to_fit(m_length + n);
            memcpy(m_ptr + m_length, str, n);
            m_length += n;
            m_ptr[m_length] = 0;
        }

        public: inline void append(const CharType* str) { return append(str, strlen(str)); }
        public: inline u16_t capacity() const { return m_buf_size - 1; }
        public: inline u16_t length() const { return m_length; }
        public: inline bool is_empty() const { return m_length == 0; }
    };

    // Static checks about size
    constexpr size_t base_size = sizeof(char*) + sizeof(u16_t) * 2;
    static_assert(base_size == 2*8); // 2 Bytes
    static_assert(sizeof(Str8) == base_size + 8); // 2 bytes + static buffer size
    static_assert(sizeof(Str16) == base_size + 16); // 2 bytes + static buffer size
    static_assert(sizeof(Str32) == base_size + 32); // 2 bytes + static buffer size
    // etc...

    class string // Static library to deal with string formatting
    {
    public:
        static std::string fmt_double(double);           // Format a double to a string (without trailing zeros).
        static std::string fmt_hex(u64_t _addr);         // Format a quad-word as a hexadecimal string.
        static std::string fmt_ptr(const void* _addr);   // Format an address as a hexadecimal string.
        template<size_t width = 80>
        static std::string fmt_title(const char* _title) // Format a title for console output (ex: ------<=[ My Title ]=>--------)
        {
            /*
             * Takes _title and do:
             * ------------<=[ _title ]=>------------
             */

            const char* pre       = "-=[ ";
            const char* post      = " ]=-";
            const char* padding   = "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
                                    "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-";
            int pad_size = (width - strlen(_title) - strlen(pre) - strlen(post)) / 2;

            char result[width+1]; // _width + end of line
            snprintf(result, width, "%*.*s%s%s%s%*.*s\n",
                     0, pad_size, padding,
                     pre, _title, post,
                     0, pad_size-1, padding
            );
            result[width] = '\0';
            return result;
        }
        inline static size_t hash(char* buffer, size_t buf_size, size_t seed = 0)
        {
            return XXHash64::hash(buffer, buf_size, seed);
        }
        inline static size_t hash(const char* str, size_t seed = 0)
        {
            return XXHash64::hash(str, strlen(str), seed);
        }
    private:
        static void limit_trailing_zeros(std::string& str, int _trailing_max);
    };
}