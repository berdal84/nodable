#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#include "tools/core/File_System.h"
#include "ndbl/gui/File_View.h"
#include "ndbl/gui/Nodable.h"

namespace ndbl
{
    // forward declarations
    class Node;
    class Graph;
    class Graph_View;
    class History;

    typedef int File_Flags;
    enum File_Flag_
    {
        File_Flag_NONE                    = 0,
        File_Flag_NEEDS_TO_BE_SAVED       = 1 << 0,
        File_Flag_TEXT_IS_DIRTY           = 1 << 1,
        File_Flag_GRAPH_IS_DIRTY          = 1 << 2,
        File_Flag_IS_DIRTY_MASK           = File_Flag_GRAPH_IS_DIRTY | File_Flag_TEXT_IS_DIRTY,
    };

    /**
     * Struct to store a nodable file (in both textual and nodal paradigm).
     * It contains:
     * - the source code
     * - the graph equivalent
     * - the history of changes (TODO: this is probably not a good idea to have the history in the file, we should have one history for the currrent session only)
     */
	struct File
    {
        tools::Path            path        = {};                // file path on disk
        File_View              view        = {};
        std::string            parsed_text = {};                // last parsed text buffer (when isolation mode is ON, this may be a portion of the file)
        Graph*                 graph       = nullptr;           // graphical representation
        File_Flags             flags       = File_Flag_NONE;        

        inline void            set_flags(File_Flags _flags) { flags |= _flags; }
        inline bool            has_flags(File_Flags _flags) { return (flags & _flags) == _flags; }
    };

    void                    file_init(File*);
    void                    file_deinit(File*);
    void                    file_update(File*, bool isolation_on); // to call each frame
    void                    file_handle_file_view_change(File*, File_View_Event_Type type);
    std::string             file_filename(const File*);
    size_t                  file_size(const File*);
    void                    file_update_graph_from_text(File*, bool isolation_on);
    void                    file_update_text_from_graph(File*, bool isolation_on);
    bool                    file_read(File* file, const tools::Path& source ); // Read an File from a given path and update file's path.
    bool                    file_write(File* file, const tools::Path& dest );  // Write an File to a given path and update file's path.

}
