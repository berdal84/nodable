#pragma once

#include <string>

#include <Nodable.h>    // forward declarations and common stuff
#include <Node.h>       // base class
#include <Member.h>

namespace Nodable
{

    /*
        The role of this class is to symbolize an instruction result as a node.
    */
    class ResultNode : public Node
    {
    public:
        ResultNode(const char* _label);
        ~ResultNode(){};

        Member* value()const
        {
            return get("value");
        }

        void setValue(Member* _value)
        {
            get("value")->set(_value);
        };

        std::string getTypeAsString()const;

    private:
        MIRROR_CLASS(ResultNode)
        (
            MIRROR_PARENT(Node)
        );
    };
}