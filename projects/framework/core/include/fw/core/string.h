#pragma once

#include <string>
#include <cstring>
#include <fw/core/types.h>
#include <xxhash/xxhash64.h>

namespace fw
{
    /**
     * Stack allocated string.
     * Switches to dynamic allocations when stack buffer is too small.
     * @tparam STACK_BUF_SIZE
     */
    template<u8_t STACK_BUF_SIZE = 1>
    class Str
    {
    private:
        union { ;
            char stack[STACK_BUF_SIZE];
            char* ptr;
        } m_data;
        u8_t m_buf_size;
        u8_t m_length;
        bool m_on_stack;

    public:
        Str()
            : m_buf_size(STACK_BUF_SIZE)
            , m_on_stack(STACK_BUF_SIZE != 1)
            , m_length(0)
        {
            if( m_on_stack )
            {
                m_data.stack[0] = '\0';
            }
            else
            {
                m_data.ptr = nullptr;
            }
        }

        Str(const char* str)
            : m_buf_size(STACK_BUF_SIZE)
            , m_on_stack(STACK_BUF_SIZE != 1)
        {
            const size_t str_length = strlen(str);

            // When buffer is too small, we switch to heap.
            if(str_length > m_buf_size )
            {
                if( !m_on_stack )
                {
                    delete[] m_data.ptr;
                    m_buf_size = str_length + 1; // TODO: compute the next power of 2 for the buffer size
                    m_data.ptr = new char[m_buf_size];
                    m_data.ptr[str_length] = '\0';
                }
                m_on_stack = false;
            }

            // Copy str to the stack
            if ( m_on_stack )
            {
                memcpy(m_data.stack, str, str_length);
                m_data.stack[str_length] = '\0';
            }
            // Copy str to the heap
            else
            {
                memcpy(m_data.ptr, str, str_length);
                m_data.ptr[str_length] = '\0';
            };
            m_length = str_length;
        }

        ~Str()
        {
            if( !m_on_stack )
            {
                delete[] m_data.ptr;
            }
        }

        const char* c_str() const
        {
            if (m_on_stack)            return static_cast<const char*>(m_data.stack);
            if (m_data.ptr != nullptr) return static_cast<const char*>(m_data.ptr);
            return "";
        }

        inline u8_t capacity() const { return m_buf_size; }
        inline u8_t length() const { return m_length; }
        inline bool is_empty() const { return m_length == 0; }
    };
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