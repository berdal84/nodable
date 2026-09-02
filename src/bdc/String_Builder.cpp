#include "String_Builder.hpp"

namespace bdc 
{
    void string_builder_init(String_Builder& sb)
    {
        sb.allocator = temp_allocator();
        array_init(sb.buffer, 0, sb.allocator);
    }

    void string_builder_release(String_Builder& sb)
    {
        array_release(sb.buffer);

        sb.buffer.data = nullptr;
        sb.buffer.size = 0;
    }

    String_Builder& string_builder_append(String_Builder& sb, const String& str)
    {
        array_append(sb.buffer, str);
        return sb;
    }

    String_Builder& string_builder_append(String_Builder& sb, const Resizable_Array<String>& arr)
    {
        for(size_t i = 0; i < arr.size; ++i)
        {
            array_append(sb.buffer, arr[i] );
        }
        return sb;
    }

    String string_builder_build_string(String_Builder& sb, String separator, Allocator* allocator)
    {
        assert(allocator != nullptr);

        // compute the size of the output string
        u32_t size = 0;

        for( u32_t i = 0; i < sb.buffer.size; i++)
        {
            assert( sb.buffer[i].size < 4096 ); // you sure?!!
            size += sb.buffer[i].size;
        }

        if( sb.buffer.size )
        {
            size += separator.size * (sb.buffer.size-1); // 1 separator after each, except last
        }

        if( size == 0)
        {
            return "";
        }

        size += 1; // +1 null-terminated

        // Initialize a string at the given length
        char* data = static_cast<char*>(allocator->proc_malloc(size)); 
        assert(data != nullptr);
        
        // Copy elem0 + separator + elem1 + ... + elemN-1
        char* cursor = data;
        for(u32_t i = 0; i < sb.buffer.size; ++i)
        {
            if ( separator.size != 0 && i != 0 )
            {
                std::memcpy(cursor, separator.data, separator.size);
                cursor += separator.size;
            }
            std::memcpy(cursor,  sb.buffer[i].data, sb.buffer[i].size);
            cursor += sb.buffer[i].size;
        }

        data[size-1] = '\0';

        string_builder_release(sb);

        String result(data, size-1, String_Flags_IS_NULL_TERMINATED);

        return result;
    }

    String string_builder_build_string(String_Builder& sb, Allocator* allocator)
    {
        return string_builder_build_string(sb, "", allocator);
    }

} // namespace bdc
