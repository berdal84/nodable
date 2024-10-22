#include "File.h"

#include <fstream>

#include "ndbl/core/Utils.h"
#include "ndbl/core/FunctionNode.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/language/Nodlang.h"

#include "GraphView.h"
#include "History.h"
#include "NodeView.h"
#include "Physics.h"

using namespace ndbl;
using namespace tools;

File::File()
: path()
, dirty(true)
, view()
, history()
{
    LOG_VERBOSE( "File", "Constructor being called ...\n")

    // FileView
    view.init(*this);

    LOG_VERBOSE( "File", "View built, creating History ...\n")

    // History
    TextEditor*       text_editor     = view.get_text_editor();
    TextEditorBuffer* text_editor_buf = history.configure_text_editor_undo_buffer(text_editor);
    view.set_undo_buffer(text_editor_buf);

    LOG_VERBOSE( "File", "History built, creating graph ...\n")

    // Graph
    _graph = new Graph(get_node_factory());
    auto* graph_view = new GraphView(_graph);
    _graph->set_view(graph_view);

    // Fill the "create node" context menu
    for( IAction* action : get_action_manager()->get_actions() )
        if ( auto create_node_action = dynamic_cast<Action_CreateNode*>(action))
            _graph->get_view()->add_action_to_node_menu(create_node_action);

    LOG_VERBOSE( "File", "Constructor being called.\n")
}

File::~File()
{
    delete _graph->get_view();
    delete _graph;
}

std::string File::get_text() const
{
    return view.get_text(isolation);
}

void File::set_text(const std::string& text)
{
    view.set_text( text, isolation );
}

void File::update_text_from_graph()
{
    const Node* root_node = _graph->get_root();

    if ( root_node == nullptr )
    {
        return;
    }

    std::string code;
    get_language()->_serialize_node( code, root_node, SerializeFlag_RECURSE );

    view.set_text( code, isolation );
}

void File::update()
{
    //
    // When history is dirty we update the graph from the text.
    // (By default undo/redo are text-based only, if hybrid_history is ON, the behavior is different
    if (history.is_dirty && !get_config()->has_flags(ConfigFlag_EXPERIMENTAL_HYBRID_HISTORY) )
    {
        update_graph_from_text();
        history.is_dirty = false;
    }
}

void File::update_graph_from_text()
{
    // Destroy all physics constraints
    auto physics_components = Utils::get_components<Physics>(_graph->get_node_registry() );
    Physics::destroy_constraints( physics_components );

    // Parse source code
    // note: File owns the parsed text buffer
    parsed_text = get_text();
    get_language()->parse(parsed_text, _graph );
}

size_t File::size() const
{
    return view.size();
}

std::string File::filename() const
{
    return path.filename().string();
}


bool File::write( File& file, const tools::Path& path)
{
    if( path.empty() )
    {
        LOG_ERROR("File", "No path defined, unable to save file\n");
        return false;
    }

    if ( !file.dirty )
    {
        LOG_MESSAGE("File", "Nothing to save\n");
    }

    std::ofstream out_fstream(path.string());
    std::string content = file.get_text();
    out_fstream.write(content.c_str(), content.size()); // TODO: size can exceed fstream!
    file.dirty = false;
    file.path = path;
    LOG_MESSAGE("File", "%s saved\n", file.filename().c_str() );

    return true;
}

bool File::read( File& file, const tools::Path& path)
{
    LOG_MESSAGE("File", "\"%s\" loading... (%s).\n", path.filename().c_str(), path.c_str())
    if(path.empty() )
    {
        LOG_ERROR("File", "Path is empty \"%s\"\n", path.c_str())
        return false;
    }

    std::ifstream file_stream(path.string());
    if (!file_stream.is_open())
    {
        LOG_ERROR("File", "Unable to load \"%s\"\n", path.c_str())
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
    file.set_text(content);
    file.dirty = false;
    file.path = path;

    LOG_MESSAGE("File", "\"%s\" loaded (%s).\n", path.filename().c_str(), path.c_str())

    return true;
}

void File::reset()
{
    update_graph_from_text();
}
