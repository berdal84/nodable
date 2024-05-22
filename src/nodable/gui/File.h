#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <observe/event.h>
#include <string>

#include "fw/core/reflection/reflection"
#include "fw/core/log.h"

#include "Isolation.h"
#include "nodable/core/NodeFactory.h"
#include "nodable/gui/FileView.h"
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
	class File
    {
	public:
        File();
        ~File();

        std::filesystem::path  path;         // file path on disk
        bool                   dirty;        // true if changed since last read/write from/to disk.
        FileView               view;
        History                history;      // history of changes
        Graph*                 graph;        // graphical representation
        observe::Event<Graph*> graph_changed;
        GraphView*             graph_view;

        std::string            filename() const;
        UpdateResult           update( Isolation ); // to call each frame
        UpdateResult           update_graph_from_text( Isolation = Isolation_OFF );
        UpdateResult           update_text_from_graph( Isolation = Isolation_OFF );
        std::string            get_text( Isolation = Isolation_OFF ) const;
        void                   set_text(const std::string& text, Isolation = Isolation_OFF );
        size_t                 size() const;

        static bool            read( File& file, const std::filesystem::path& source ); // Read an File from a given path and update file's path.
        static bool            write( File& file, const std::filesystem::path& dest );  // Write an File to a given path and update file's path.

    };
}
