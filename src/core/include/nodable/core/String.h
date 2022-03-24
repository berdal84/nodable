#pragma once

#include <string>

namespace Nodable
{
    class String
    {
    public:
        template<typename T>
        static std::string from(T d)
        {
            std::string str {std::to_string(d)};
            remove_trailing_zeros(str);
            return str;
        }

        static std::string address_to_hexadecimal(void* _addr)
        {
            char str[19]; // "0x" + 16 hexa-char (8 bytes) + 1 end char
            sprintf(str, "%#16llx", (u64)_addr);
            return {str};
        }

    private:
        static void remove_trailing_zeros(std::string& str)
        {
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.find_last_of('.') + 1 == str.size())
                str.erase(str.find_last_of('.'), std::string::npos);
        }
    };
}