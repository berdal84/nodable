#pragma once
#include <nodable/Command.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>

namespace Nodable
{
    class Cmd_DisconnectNodes : public IUndoableCmd
    {
    public:
        Cmd_DisconnectNodes(Node* _src, Node* _dst, Relation_t _relation)
        : m_src(_src)
        , m_dst(_dst)
        , m_relation(_relation)
        , m_graph(_src->get_parent_graph())
        {
            char str[200];
            sprintf(str
                    , "DisconnectNodes\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                      " - relation: %s\n"
                    , _src->get_label()
                    , _dst->get_label()
                    , to_string(m_relation));
            m_description.append(str);
        }

        ~Cmd_DisconnectNodes() override = default;

        void execute() override
        {
            m_graph->disconnect( m_src, m_dst, m_relation);
        }

        void undo() override
        {
            m_graph->connect( m_src, m_dst, m_relation);
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