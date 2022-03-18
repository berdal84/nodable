#pragma once
#include <nodable/app/Command.h>
#include <nodable/core/Node.h>
#include <nodable/core/GraphNode.h>

namespace Nodable
{
    class Cmd_DisconnectNodes : public IUndoableCmd
    {
    public:
        Cmd_DisconnectNodes(DirectedEdge _relation)
        : m_relation(_relation)
        , m_graph(_relation.nodes.src->get_parent_graph())
        {
            char str[200];
            sprintf(str
                    , "DisconnectNodes\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                      " - relation: %s\n"
                    , m_relation.nodes.src->get_label()
                    , m_relation.nodes.dst->get_label()
                    , to_string(m_relation.type));
            m_description.append(str);
        }

        ~Cmd_DisconnectNodes() override = default;

        void execute() override
        {
            m_graph->disconnect( m_relation);
        }

        void undo() override
        {
            m_graph->connect( m_relation);
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