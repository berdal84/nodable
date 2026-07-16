#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#include "Isolation.h"
#include "ndbl/gui/File_View.h"
#include "ndbl/gui/History.h"
#include "ndbl/gui/Nodable.h"

namespace ndbl
{
    // forward declarations
    class Node;
    class Graph;
    class Graph_View;
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
        typedef int Flags;
        enum Flags_
        {
            Flags_NONE                    = 0,
            Flags_NEEDS_TO_BE_SAVED       = 1 << 0,
            Flags_TEXT_IS_DIRTY           = 1 << 1,
            Flags_GRAPH_IS_DIRTY          = 1 << 2,
            Flags_IS_DIRTY_MASK           = Flags_GRAPH_IS_DIRTY | Flags_TEXT_IS_DIRTY,
        };

        File();
        ~File();

        tools::Path            path; // file path on disk
        File_View              view;
        History                history; // history of changes
    private:
        Isolation              _isolation = Isolation_OFF;
        Graph*                 _graph; // graphical representation
        std::string            _parsed_text; // last parsed text buffer
        Flags                  _flags = Flags_NONE;
        void                   _update_graph_from_text();
        void                   _update_text_from_graph();
    public:
        bool                   needs_to_be_saved() const { return _flags & Flags_NEEDS_TO_BE_SAVED; }
        void                   update(); // to call each frame
        void                   handle_file_view_change(File_View_Event_Type type);
        void                   set_graph_dirty() {_flags |= Flags_GRAPH_IS_DIRTY; }
        void                   set_text_dirty() {_flags |= Flags_TEXT_IS_DIRTY; }
        Graph*                 graph() { return _graph; };
        std::string            filename() const;
        void                   set_isolation(Isolation mode);
        size_t                 size() const;

        static bool            read( File& file, const tools::Path& source ); // Read an File from a given path and update file's path.
        static bool            write( File& file, const tools::Path& dest );  // Write an File to a given path and update file's path.
    };
}
