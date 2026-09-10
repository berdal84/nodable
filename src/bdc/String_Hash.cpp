#include "String_Hash.hpp"
#include "Hash.hpp"

namespace bdc
{
    String_Hash string_hash(const String& str)
    {
        return { .hash = djb2_hash(str.data, str.size), .string = str };
    }
} // namespace bdc