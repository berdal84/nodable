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
#include "HybridFileView.h"
#include "History.h"
#include "Nodable.h"
#include "types.h"

namespace ndbl
{
    // forward declarations
    class Node;
    class Graph;
    class GraphView;
    class History;

    /**
     * Class representing a file in both textual and nodal paradigm.
     * It contains:
     * - the source code
     * - the graph equivalent
     * - the history of changes
     */
	class HybridFile
	{
	public:
        observe::Event<Graph*>           graph_changed;
        ghc::filesystem::path            path;    // file path on disk
        std::string                      name;    // friendly name
        bool                             changed; // true if changes needs to be saved
        HybridFileView                   view;

		explicit HybridFile(std::string _name);
		explicit HybridFile(const ghc::filesystem::path& _path);
        ~HybridFile();

        bool                             load();
        UpdateResult                     update(); // to call each frame
        bool                             write_to_disk();
        inline Graph*                    get_graph() { return m_graph; }
        UpdateResult                     update_graph_from_text(bool isolate_selection);
        UpdateResult                     update_text_from_graph(bool isolate_selection);
        inline GraphView*                get_graph_view() { return m_graph_view; }
        inline History*                  get_history() { return &m_history; }
        std::string                      get_text(bool isolate_selection) const;
        void                             set_text(const std::string& text);
        size_t                           size() const;
    private:
		History                    m_history;
		Graph*                     m_graph;
        GraphView*                 m_graph_view;
    };
}
