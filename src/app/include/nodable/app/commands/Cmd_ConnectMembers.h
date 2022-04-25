#pragma once
#include <nodable/app/Command.h>
#include <nodable/core/Member.h>
#include <nodable/core/GraphNode.h>

namespace ndbl
{
    /**
     * Command to drop_on two members.
     * src output --> dst input
     */
    class Cmd_ConnectMembers : public IUndoableCmd
    {
    public:
        Cmd_ConnectMembers(Member* _src, Member* _dst)
        : m_src(_src)
        , m_dst(_dst)
        , m_graph(_src->get_owner()->get_parent_graph())
        , m_wire(nullptr)
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "ConnectMembers\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                    , _src->get_name().c_str()
                    , _dst->get_name().c_str() );
            m_description.append(str);
        }

        ~Cmd_ConnectMembers() override = default;

        void execute() override
        {
            m_wire = m_graph->connect(m_src, m_dst);
        }

        void undo() override
        {
            m_graph->disconnect(m_wire);
        }

        const char* get_description() const override
        {
            return m_description.c_str();
        }

    private:
        std::string m_description;
        Member*     m_src;
        Member*     m_dst;
        Wire*       m_wire;
        GraphNode*  m_graph;
    };
}