#include "File.h"

#include <fstream>

#include "bdc/String_Builder.hpp"
#include "tools/core/Event.h"
#include "tools/core/Flags.h"
#include "tools/core/Asserts.h"
#include "tools/gui/Action_Manager.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/language/Nodlang.h"
#include "ndbl/gui/Event.h"
#include "ndbl/gui/Graph_View.h"
#include "ndbl/gui/File_View.h"
#include "ndbl/gui/Command_Manager.h"
#include "ndbl/gui/Node_View.h"

using namespace ndbl;
using namespace tools;

namespace ndbl
{
    void _file_set_text_dirty(File* file)
    {
        file->flags |= File_Flag_TEXT_IS_DIRTY;
    }
}

void ndbl::file_init(File* file)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "File", "Constructor being called ...\n");

    file->flags = File_Flag_NEEDS_TO_BE_SAVED | File_Flag_GRAPH_IS_DIRTY; // A File is text-based by default, so we set the graph dirty to force it to be refreshed from the text.

    // Graph
    auto* graph = bdc::memory_new<Graph>();
    file->graph = graph;
    graph_init(graph);

    auto* graph_view = bdc::memory_new<Graph_View>();
    graphview_init(graph_view, file->graph);
    graph->view = graph_view;

    graph->signal_change.connect<&_file_set_text_dirty>(file);
    graph_view->signal_change.connect<&_file_set_text_dirty>(file);

    // Fill the "create node" context menu
    for( const Action& action : action_manager()->actions )
        if ( action.event.type == Event_Type_USER && action.event.user.code == Event_Type_NEW_NODE )
            graph_view->node_search_input.items.push_back(action );

    // File_View
    fileview_init(&file->view, file);

    file->view.signal_change.connect<&file_handle_file_view_change>(file);

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "File", "View built, creating History ...\n");

    // History
    Text_Editor_Undo_Buffer* text_editor_buf = command_manager_configure_text_editor_undo_buffer(&file->view.text_editor);
    fileview_set_undo_buffer(&file->view, text_editor_buf);
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "File", "Constructor being called.\n");
}

void ndbl::file_deinit(File* file)
{
    assert(file->graph->signal_change.disconnect<&_file_set_text_dirty>(file));
    
    file->graph->view->signal_change.disconnect();
    file->view.signal_change.disconnect();

    graph_deinit(file->graph);
    delete file->graph;
    file->graph = nullptr;
}

void ndbl::file_update_text_from_graph(File* file, bool isolation_on)
{
    if ( auto* root_node = graph_root( file->graph ) )
    {
        bdc::String_Builder out;
        lang_serialize_node(language(), out, root_node, Serialization_Flag_RECURSE);
        bdc::String temp_str = bdc::string_builder_build_string(out); // the TextEditor in FileView will do a copy via an std::string
        fileview_set_text( &file->view, temp_str, isolation_on );
    }
    else
    {
        TOOLS_LOG(tools::Verbosity_Warning, "File", "Unable to update text from graph: no root found in the Graph.\n");
    }
}

void ndbl::file_update(File* file, bool isolation_on)
{
    //
    // When history is dirty we update the graph from the text.
    // (By default undo/redo are text-based only, if EXPERIMENTAL_HYBRID_COMMAND_MANAGER is ON, the behavior is different
    if ( command_manager()->is_dirty )
    {
        if ( HAS_FLAGS(config()->flags, Config_Flag_EXPERIMENTAL_HYBRID_COMMAND_MANAGER) )
        {
            ASSERT(false); // Not implemented yet
        }
        else
        {
            UNSET_FLAGS(file->flags, File_Flag_IS_DIRTY_MASK);
            SET_FLAGS(file->flags, File_Flag_GRAPH_IS_DIRTY); // set graph dirty (we are text-based!)
        }
        command_manager()->is_dirty = false;
    }

    if ( HAS_FLAGS(file->flags, File_Flag_GRAPH_IS_DIRTY) )
    {
        file_update_graph_from_text(file, isolation_on);
        graph_update(file->graph);
        UNSET_FLAGS(file->flags, File_Flag_IS_DIRTY_MASK);
    }
    else if ( HAS_FLAGS(file->flags, File_Flag_TEXT_IS_DIRTY) )
    {
        graph_update(file->graph);
        file_update_text_from_graph(file, isolation_on);
        UNSET_FLAGS(file->flags, File_Flag_IS_DIRTY_MASK);
    }
    else
    {
        graph_update(file->graph);
    }
}

void ndbl::file_update_graph_from_text(File* file, bool isolation_on)
{
    // Parse source code
    // note: File owns the parsed text buffer
    file->parsed_text = fileview_get_text(&file->view, isolation_on );
    lang_parse(language(), file->graph, file->parsed_text);

    SET_FLAGS(file->graph->view->flags, Graph_View_Flag_NEEDS_TO_BE_RESET | Graph_View_Flag_NEEDS_TO_FRAME_CONTENT);
}

size_t ndbl::file_size(const File* file)
{
    return fileview_size(&file->view);
}

const char* ndbl::file_name(const File* file)
{
    return file->path.filename().c_str();
}

bool ndbl::file_write(File* file, const tools::Path& path)
{
    if( path.empty() )
    {
        TOOLS_LOG(tools::Verbosity_Error, "File", "No path defined, unable to save file\n");
        return false;
    }

    if ( !HAS_FLAGS(file->flags, File_Flag_NEEDS_TO_BE_SAVED) && path == file->path )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "File", "Nothing to save\n");
        return true;
    }

    // get content (We do a copy here, because the data from the fileview is not necessarily contiguous, and texteditor gives us a new string instance)
    bdc::String content = fileview_get_text(&file->view, false );

    // write bytes
    std::ofstream out_fstream(path.c_str());
    out_fstream.write(content.data, content.size); // TODO: size can exceed fstream!

    // update file
    UNSET_FLAGS(file->flags, File_Flag_NEEDS_TO_BE_SAVED);
    file->path = path;

    TOOLS_LOG(tools::Verbosity_Message, "File", "%s saved\n", file_name(file) );

    return true;
}

bool ndbl::file_read( File* file, const tools::Path& path)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "File", "\"%s\" loading... (%s).\n", path.filename().c_str(), path.c_str());
    if(path.empty() )
    {
        TOOLS_LOG(tools::Verbosity_Error, "File", "Path is empty \"%s\"\n", path.c_str());
        return false;
    }

    std::ifstream file_stream( path.c_str() );

    if (!file_stream.is_open())
    {
        TOOLS_LOG(tools::Verbosity_Error, "File", "Unable to load \"%s\"\n", path.c_str());
        return false;
    }

    bdc::String_Builder sb{};
    string_builder_init(sb);

    const int MAX_LINE_LENGTH = 256;
    char buffer[MAX_LINE_LENGTH];
    while (file_stream.getline(buffer, MAX_LINE_LENGTH))
    {
        std::streamsize bytes_read = file_stream.gcount();
        String str = string_copy( String{buffer, (u32_t)bytes_read}, temp_allocator() );
        string_builder_append(sb, str);
    }

    if (file_stream.eof() == false && file_stream.fail())
    {
        TOOLS_LOG(tools::Verbosity_Warning, "File", "Line exceeded buffer size!\n");
        file_stream.clear();
    }

    fileview_set_text(&file->view, string_builder_build_string(sb, bdc::heap_allocator() ), false);
    UNSET_FLAGS(file->flags, File_Flag_NEEDS_TO_BE_SAVED);
    file->path = path;

    TOOLS_LOG(tools::Verbosity_Message, "File", "%s loaded\n", path.filename().c_str(), path.c_str());

    return true;
}

void ndbl::file_handle_file_view_change(File* file, File_View_Event_Type type)
{
    switch ( type )
    {
        case File_View_Overlay_Type_TEXT:
            SET_FLAGS(file->flags, File_Flag_TEXT_IS_DIRTY);
            break;
        
        case File_View_Overlay_Type_GRAPH:
            SET_FLAGS(file->flags, File_Flag_GRAPH_IS_DIRTY);
            break;
        
        default:
            TOOLS_UNREACHABLE("Unhandled File_View_Event_Type (value: %i)\n", type);
    }
}