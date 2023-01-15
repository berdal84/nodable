#pragma once

namespace ndbl
{
    // forward declarations
    class NodableLanguage;
    class InvokableComponent;
    class Scope;
    class InstructionNode;
    class ForLoopNode;
    class ConditionalStructNode;

    class NodableSerializer
    {
    public:

        explicit NodableSerializer(const NodableLanguage& _language): m_language(_language) {};
        ~NodableSerializer() = default;

        std::string& serialize(std::string& _out, const InvokableComponent*) const;
        std::string& serialize(std::string& _out, const func_type*, const std::vector<Property *>&)const;
        std::string& serialize(std::string& _out, const func_type*)const;
        std::string& serialize(std::string& _out, const Token_t&)const;
        std::string& serialize(std::string& _out, type)const;
        std::string& serialize(std::string& _out, const Property *, bool followConnections = true)const;
        std::string& serialize(std::string& _out, std::shared_ptr<const Token>) const;
        std::string& serialize(std::string& _out, const InstructionNode*)const;
        std::string& serialize(std::string& _out, const Node*)const;
        std::string& serialize(std::string& _out, const Scope*)const;
        std::string& serialize(std::string& _out, const ForLoopNode* _for_loop)const;
        std::string& serialize(std::string& _out, const ConditionalStructNode*) const;
        std::string& serialize(std::string& _out, const variant* variant) const;
        std::string& serialize(std::string& _out, const VariableNode *_node) const;
    protected:
        const NodableLanguage& m_language;
    };
}
