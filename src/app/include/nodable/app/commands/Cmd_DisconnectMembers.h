#pragma once
#include <nodable/app/Command.h>
#include <nodable/core/Member.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/Wire.h>

namespace Nodable
{
    class Cmd_DisconnectMembers : public IUndoableCmd
    {
    public:
        Cmd_DisconnectMembers(Wire* _wire)
        : m_src(_wire->get_source())
        , m_dst(_wire->get_dest())
        , m_graph(_wire->get_source()->get_owner()->get_parent_graph())
        , m_wire(_wire)
        {
            char str[200];
            sprintf(str
                    , "DisconnectMembers\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                    , m_src->get_name().c_str()
                    , m_dst->get_name().c_str() );
            m_description.append(str);
        }

        ~Cmd_DisconnectMembers() override = default;

        void execute() override
        {
            m_graph->disconnect(m_wire);
        }

        void undo() override
        {
            m_wire = m_graph->connect(m_src, m_dst);
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