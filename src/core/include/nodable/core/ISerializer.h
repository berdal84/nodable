#pragma once

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include <nodable/core/Token.h>
#include <nodable/core/reflection/reflection>

namespace Nodable
{
    // forward declarations
    class InvokableComponent;
    class ConditionalStructNode;
    class ForLoopNode;
    class Member;
    class Language;
    class IScope;
    class InstructionNode;
    class ForLoopNode;
    class Scope;
    class Operator;
    class Language;
    class Variant;
    class VariableNode;
    class Signature;

    class ISerializer
    {
    public:
        virtual ~ISerializer() = default;

        virtual std::string& serialize(std::string& _out, const InvokableComponent*) const = 0;
        virtual std::string& serialize(std::string& _out, const Signature*, const std::vector<Member*>&)const = 0;
        virtual std::string& serialize(std::string& _out, const Signature*)const = 0;
        virtual std::string& serialize(std::string& _out, const Token_t&)const = 0;
        virtual std::string& serialize(std::string& _out, type)const = 0;
        virtual std::string& serialize(std::string& _out, const Member*, bool followConnections = true)const = 0;
        virtual std::string& serialize(std::string& _out, std::shared_ptr<const Token>) const = 0;
        virtual std::string& serialize(std::string& _out, const InstructionNode*)const = 0;
        virtual std::string& serialize(std::string& _out, const Node*)const = 0;
        virtual std::string& serialize(std::string& _out, const Scope*)const = 0;
        virtual std::string& serialize(std::string& _out, const ForLoopNode* _for_loop)const = 0;
        virtual std::string& serialize(std::string& _out, const ConditionalStructNode*) const = 0;
        virtual std::string& serialize(std::string& _out, const Variant* variant) const = 0;
        virtual std::string& serialize(std::string& _out, const VariableNode *_node) const = 0;
    };
}