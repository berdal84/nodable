#pragma once
#include <memory> // std::shared_ptr
#include "tools/core/reflection/reflection"
#include "ASTNode.h" // Base class

namespace ndbl
{
    class ASTLiteralNode: public ASTNode
    {
    public:
        ASTToken               token;

        ASTLiteralNode() {};
        ~ASTLiteralNode() override {};

        void init(const tools::TypeDescriptor* _type, const std::string& _name);

    private:
        const tools::TypeDescriptor* m_type = nullptr;
        REFLECT_DERIVED_CLASS()
    };
}