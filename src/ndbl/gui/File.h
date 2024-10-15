#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <observe/event.h>
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

        tools::Path  path;         // file path on disk
        bool                   dirty;        // true if changed since last read/write from/to disk.
        FileView               view;
        History                history;      // history of changes
        observe::Event<Graph*> graph_changed;

    private:
        Graph*                 graph;        // graphical representation
        std::string            parsed_text;  // last parsed text buffer
    public:
        Graph&                 get_graph() { return *graph; };
        std::string            filename() const;
        UpdateResult           update( Isolation ); // to call each frame
        UpdateResult           update_graph_from_text( Isolation = Isolation_OFF );
        UpdateResult           update_text_from_graph( Isolation = Isolation_OFF );
        std::string            get_text( Isolation = Isolation_OFF ) const;
        void                   set_text(const std::string& text, Isolation = Isolation_OFF );
        size_t                 size() const;

        static bool            read( File& file, const tools::Path& source ); // Read an File from a given path and update file's path.
        static bool            write( File& file, const tools::Path& dest );  // Write an File to a given path and update file's path.

    };
}
