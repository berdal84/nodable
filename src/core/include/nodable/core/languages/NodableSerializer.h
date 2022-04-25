#pragma once

#include <nodable/core/ISerializer.h>

namespace ndbl
{
    // forward declarations
    class NodableLanguage;

    class NodableSerializer : public ISerializer
    {
    public:

        explicit NodableSerializer(const NodableLanguage& _language): m_language(_language) {};
        ~NodableSerializer() = default;

        std::string& serialize(std::string& _out, const InvokableComponent*) const override;
        std::string& serialize(std::string& _out, const func_type*, const std::vector<Member*>&)const override;
        std::string& serialize(std::string& _out, const func_type*)const override;
        std::string& serialize(std::string& _out, const Token_t&)const override;
        std::string& serialize(std::string& _out, type)const override;
        std::string& serialize(std::string& _out, const Member*, bool followConnections = true)const override;
        std::string& serialize(std::string& _out, std::shared_ptr<const Token>) const override;
        std::string& serialize(std::string& _out, const InstructionNode*)const override;
        std::string& serialize(std::string& _out, const Node*)const override;
        std::string& serialize(std::string& _out, const Scope*)const override;
        std::string& serialize(std::string& _out, const ForLoopNode* _for_loop)const override;
        std::string& serialize(std::string& _out, const ConditionalStructNode*) const override;
        std::string& serialize(std::string& _out, const variant* variant) const override;
        std::string& serialize(std::string& _out, const VariableNode *_node) const override;
    protected:
        const NodableLanguage& m_language;
    };
}
