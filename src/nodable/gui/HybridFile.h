#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <observe/event.h>
#include <string>

#include "fw/core/reflection/reflection"
#include "fw/core/log.h"

#include "nodable/core/NodeFactory.h"
#include "nodable/gui/HybridFileView.h"
#include "nodable/gui/History.h"
#include "nodable/gui/Nodable.h"
#include "nodable/gui/types.h"

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
        std::filesystem::path            path;    // file path on disk
        std::string                      name;    // friendly name
        bool                             is_content_dirty; // true if changes needs to be saved
        HybridFileView                   view;

        explicit HybridFile(const char*); // name only
		explicit HybridFile(const std::filesystem::path& _path); // name will be extracted from path, path will be stored too
        ~HybridFile();

        bool                             load();
        UpdateResult                     update(); // to call each frame
        bool                             write_to_disk();
        inline Graph*                    get_graph() { return m_graph; }
        UpdateResult                     update_graph_from_text(bool isolate_selection = false);
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
