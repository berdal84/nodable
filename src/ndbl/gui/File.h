#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "tools/core/reflection/reflection"
#include "tools/core/log.h"

#include "Isolation.h"
#include "ndbl/core/NodeFactory.h"
#include "ndbl/gui/FileView.h"
#include "ndbl/gui/History.h"
#include "ndbl/gui/Nodable.h"
#include "ndbl/gui/types.h"

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

        tools::Path            path; // file path on disk
        bool                   dirty; // true if changed since last read/write from/to disk.
        FileView               view;
        History                history; // history of changes
    private:
        Isolation              _isolation = Isolation_OFF;
        Graph*                 _graph; // graphical representation
        std::string            _parsed_text; // last parsed text buffer
        bool                   _requires_graph_update;
        bool                   _requires_text_update;
    public:
        void                   update_graph_next_frame() { _requires_graph_update = true; }
        void                   update_text_next_frame() { _requires_text_update = true; }
        Graph&                 graph() { return *_graph; };
        std::string            filename() const;
        void                   set_isolation(Isolation mode);
        void                   update(); // to call each frame
        void                   update_graph_from_text();
        void                   update_text_from_graph();
        size_t                 size() const;

        static bool            read( File& file, const tools::Path& source ); // Read an File from a given path and update file's path.
        static bool            write( File& file, const tools::Path& dest );  // Write an File to a given path and update file's path.
    };
}
