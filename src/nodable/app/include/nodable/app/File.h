#pragma once

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <observe/event.h>
#include <ImGuiColorTextEdit/TextEditor.h> // for coordinates

#include <fw/reflection/reflection>
#include <fw/Log.h>

#include <nodable/app/App.h>
#include <nodable/app/History.h>
#include <nodable/app/types.h>
#include <nodable/core/NodeFactory.h>

namespace ndbl
{
    // forward declarations
    class Node;
    class GraphNode;
    class History;
    class FileView;

	class File
	{
	public:
		File(std::string _name);
		File(const std::string &_name, const std::string &_path);
        ~File();

        observe::Event<GraphNode*> m_on_graph_changed_evt;

        bool                             read_from_disk();
        bool                             write_to_disk();
        bool                             update();
        inline GraphNode*                get_graph() { return m_graph; }
        inline History*                  get_history() { return &m_history; }
        const std::string&               get_name()const { return m_name; }
        void                             set_name(const std::string& _name) { m_name = _name; }
        bool                             has_path()const { return !m_path.empty(); }
        void                             set_path(const std::string& _path);
        const std::string&               get_path()const { return m_path; }
        inline FileView*                 get_view()const { return m_view; };
        inline void                      set_changed_flag(bool _value = true) { m_modified = _value; }
        inline bool                      has_changed() const { return m_modified; }
        bool                             update_graph();
        bool                             update_graph(std::string &_expression);

    private:
		bool                       m_modified;
		std::string                m_name;
		std::string                m_path;
		const NodeFactory          m_factory;
		FileView*                  m_view;
		History                    m_history;
		GraphNode*                 m_graph;
    };
}
