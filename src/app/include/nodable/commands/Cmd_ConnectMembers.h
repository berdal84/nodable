#pragma once
#include <nodable/Command.h>
#include <nodable/Member.h>
#include <nodable/GraphNode.h>

namespace Nodable
{
    /**
     * Command triggered when user modify text in the text editor.
     */
    class Cmd_ConnectMembers : public IUndoableCmd
    {
    public:
        Cmd_ConnectMembers(Member* _src, Member* _dst, ConnBy_ _conn_by, GraphNode* _graph)
        : m_src(_src)
        , m_dst(_dst)
        , m_conn_by(_conn_by)
        , m_graph(_graph)
        {
            char str[200];
            sprintf(str
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
            m_wire = m_graph->connect(m_src, m_dst, m_conn_by);
        }

        void redo() override
        {
            execute();
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
        ConnBy_     m_conn_by;
        Wire*       m_wire;
        GraphNode*  m_graph;
    };
}