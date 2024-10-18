#pragma once
#include <memory> // std::shared_ptr
#include "tools/core/reflection/reflection"
#include "Node.h" // Base class

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:
        Token               token;

        LiteralNode() {};
        ~LiteralNode() override {};

        void init(const tools::TypeDescriptor* _type, const std::string& _name);

    private:
        const tools::TypeDescriptor* m_type = nullptr;
        REFLECT_DERIVED_CLASS()
    };
}