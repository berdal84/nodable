#include "File.h"

#include <fstream>

#include "tools/core/Event.h"
#include "tools/core/Flags.h"
#include "tools/core/Asserts.h"
#include "tools/core/Component.h"
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
    file->graph = new Graph();
    graph_init(file->graph);

    auto* graph_view = new Graph_View();
    component_init(graph_view, file->graph);
    componentbag_add(&file->graph->component_bag, graph_view);

    file->graph->signal_change.connect<&_file_set_text_dirty>(file);
    graph_view->signal_change.connect<&_file_set_text_dirty>(file);

    // Fill the "create node" context menu
    for( const Action& action : action_manager()->actions )
        if ( action.event.type == Event_Type_USER && action.event.user.code == Event_Code_REQUEST_CREATE_NODE )
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
    
    Graph_View* graph_view = componentbag_get<Graph_View>(&file->graph->component_bag); // TODO: we could store the ptr in ctor.
    graph_view->signal_change.disconnect();

    file->view.signal_change.disconnect();

    graph_deinit(file->graph);
    delete file->graph;
    file->graph = nullptr;
}

void ndbl::file_update_text_from_graph(File* file, bool isolation_on)
{
    if ( auto* root_node = graph_root( file->graph ) )
    {
        std::string code;
        get_language()->serialize_node(code, root_node, Serialization_Flag_RECURSE);
        fileview_set_text( &file->view, code, isolation_on );
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
        if ( HAS_FLAGS(get_config()->flags, Config_Flag_EXPERIMENTAL_HYBRID_COMMAND_MANAGER) )
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
    get_language()->parse(file->graph, file->parsed_text);

    auto* graphview = graph_component<Graph_View>(file->graph);
    SET_FLAGS(graphview->flags, Graph_View_Flag_NEEDS_TO_BE_RESET | Graph_View_Flag_NEEDS_TO_FRAME_CONTENT);
}

size_t ndbl::file_size(const File* file)
{
    return fileview_size(&file->view);
}

std::string ndbl::file_filename(const File* file)
{
    return file->path.filename().string();
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
    std::string content = fileview_get_text(&file->view, false );

    // write bytes
    std::ofstream out_fstream(path.string());
    out_fstream.write(content.c_str(), content.size()); // TODO: size can exceed fstream!

    // update file
    UNSET_FLAGS(file->flags, File_Flag_NEEDS_TO_BE_SAVED);
    file->path = path;

    TOOLS_LOG(tools::Verbosity_Message, "File", "%s saved\n", file_filename(file).c_str() );

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

    std::ifstream file_stream(path.string());
    if (!file_stream.is_open())
    {
        TOOLS_LOG(tools::Verbosity_Error, "File", "Unable to load \"%s\"\n", path.c_str());
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
    fileview_set_text(&file->view, content, false);
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