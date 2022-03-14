#pragma once
#include <nodable/Command.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>

namespace Nodable
{
    /**
     * Command to drop_on two members.
     * src output --> dst input
     */
    class Cmd_ConnectNodes : public IUndoableCmd
    {
    public:
        Cmd_ConnectNodes(Node* _src, Node* _dst)
        : m_src(_src)
        , m_dst(_dst)
        , m_relation(Relation_t::IS_SUCCESSOR_OF)
        , m_graph(_src->get_parent_graph())
        {
            char str[200];
            sprintf(str
                    , "ConnectNodes\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                    , _src->get_label()
                    , _dst->get_label() );
            m_description.append(str);
        }

        ~Cmd_ConnectNodes() override = default;

        void execute() override
        {
            m_graph->connect( m_src, m_dst, m_relation);
        }

        void redo() override
        {
            execute();
        }

        void undo() override
        {
            m_graph->disconnect( m_src, m_dst, m_relation);
        }

        const char* get_description() const override
        {
            return m_description.c_str();
        }

    private:
        std::string m_description;
        Node*       m_src;
        Node*       m_dst;
        Relation_t  m_relation;
        GraphNode*  m_graph;
    };
}