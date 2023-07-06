#pragma once

#include <ImGuiColorTextEdit/TextEditor.h>// for coordinates
#include <SDL.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <observe/event.h>
#include <string>

#include "fw/core/reflection/reflection"
#include "fw/core/log.h"

#include "core/NodeFactory.h"
#include "FileView.h"
#include "History.h"
#include "Nodable.h"
#include "types.h"

namespace ndbl
{
    // forward declarations
    class Node;
    class GraphNode;
    class History;

	class File
	{
	public:
		explicit File(std::string _name);
		explicit File(const ghc::filesystem::path& _path);
        ~File();

        observe::Event<GraphNode*> event_graph_changed;

        bool load();
        bool                             write_to_disk();
        bool                             update();
        inline GraphNode*                get_graph() { return m_graph; }
        inline History*                  get_history() { return &m_history; }
        bool                             update_graph();
        bool                             update_graph(std::string &_expression);
        ghc::filesystem::path            path;    // file path on disk
        std::string                      name;    // friendly name
        bool                             changed; // true if changes needs to be saved
        FileView                         view;
    private:
		const NodeFactory          m_factory;
		History                    m_history;
		GraphNode*                 m_graph;
    };
}
