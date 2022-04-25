#pragma once
#include <nodable/app/Command.h>
#include <nodable/core/Node.h>
#include <nodable/core/GraphNode.h>

namespace ndbl
{
    /**
     * Command to drop_on two members.
     * src output --> dst input
     */
    class Cmd_ConnectNodes : public IUndoableCmd
    {
    public:
        Cmd_ConnectNodes(Node* _src, Node* _dst, EdgeType _relation)
        : m_relation(_relation, _src, _dst)
        , m_graph(_src->get_parent_graph())
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "ConnectNodes\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                      " - relation: \"%s\"\n"
                    , _src->get_label()
                    , _dst->get_label()
                    , to_string(_relation) );
            m_description.append(str);
        }

        ~Cmd_ConnectNodes() override = default;

        void execute() override
        {
            m_graph->connect( m_relation);
        }

        void undo() override
        {
            m_graph->disconnect( m_relation);
        }

        const char* get_description() const override
        {
            return m_description.c_str();
        }

    private:
        std::string m_description;
        DirectedEdge    m_relation;
        GraphNode*  m_graph;
    };
}