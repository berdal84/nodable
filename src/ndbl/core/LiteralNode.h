#pragma once
#include <memory> // std::shared_ptr
#include "tools/core/reflection/reflection"
#include "Node.h" // Base class

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:
        DECLARE_REFLECT_override

        Token token = { Token_t::literal_any };

        LiteralNode() {};
        ~LiteralNode() override {};

        void init(const tools::TypeDescriptor* _type, const std::string& _name);

    private:
        const tools::TypeDescriptor* m_type = nullptr;
    };
}