#include "File.h"

#include <fstream>

#include "ndbl/core/NodeUtils.h"
#include "ndbl/core/InvokableComponent.h"
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
    graph = new Graph(get_node_factory());
    auto* graph_view = new GraphView(graph);
    graph->set_view(graph_view);
    view.add_child(graph_view->base());

    // Fill the "create node" context menu
    for( IAction* action : get_action_manager()->get_actions() )
        if ( auto create_node_action = dynamic_cast<Action_CreateNode*>(action))
            graph->get_view()->add_action_to_context_menu( create_node_action );

    LOG_VERBOSE( "File", "Constructor being called.\n")
}

File::~File()
{
    delete graph->get_view();
    delete graph;
}

std::string File::get_text( Isolation mode) const
{
    return view.get_text( mode );
}

void File::set_text(const std::string& text, Isolation mode)
{
    view.set_text( text, mode );
}

UpdateResult File::update_text_from_graph( Isolation mode )
{
    const Node* root_node = graph->get_root();

    if ( root_node == nullptr )
    {
        return UpdateResult::SUCCES_WITHOUT_CHANGES;
    }

    std::string code;
    get_language()->serialize_node( code, root_node );

    view.set_text( code, mode );

    return UpdateResult::SUCCESS_WITH_CHANGES;
}

UpdateResult File::update( Isolation flags )
{
    // 1) Handle when view changes (graph or text)
    //--------------------------------------------

    if( view.changed() )
    {
        if( view.focused_text_changed() && !view.is_graph_dirty() )
        {
            update_graph_from_text( flags );
        }
        else if ( view.is_graph_dirty() )
        {
            update_text_from_graph( flags );
        }
        else
        {
            // TODO: The case where both focused_text and graph changed is not handled yet
            //       This is not supposed to happens, that's why there is an assert to be aware of is
            ASSERT(false);
        }
        view.set_dirty( false );
    }

    // 2) Handle when graph (not the graph view) changes
    //--------------------------------------------------

    if ( graph->is_dirty() )
    {
        // Refresh text
        update_text_from_graph( flags );

        // Refresh constraints
        auto physics_components = NodeUtils::get_components<Physics>(graph->get_node_registry() );
        Physics::destroy_constraints( physics_components );
        Physics::create_constraints(graph->get_node_registry() );

        graph->set_dirty(false);
    }

    return graph->update(); // ~ garbage collection
}

UpdateResult File::update_graph_from_text( Isolation scope)
{
    // Destroy all physics constraints
    auto physics_components = NodeUtils::get_components<Physics>(graph->get_node_registry() );
    Physics::destroy_constraints( physics_components );

    // Parse source code
    std::string text = get_text(scope);
    bool parse_ok = get_language()->parse(text, graph );
    if (parse_ok && !graph->is_empty() )
    {
        Physics::create_constraints(graph->get_node_registry() );
        graph_changed.emit(graph );
        return UpdateResult::SUCCESS_WITH_CHANGES;
    }
    return UpdateResult::SUCCES_WITHOUT_CHANGES;
}

size_t File::size() const
{
    return view.size();
}

std::string File::filename() const
{
    return path.filename().string();
}


bool File::write( File& file, const std::filesystem::path& path)
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

    std::ofstream out_fstream(path.c_str());
    std::string content = file.get_text();
    out_fstream.write(content.c_str(), content.size()); // TODO: size can exceed fstream!
    file.dirty = false;
    file.path = path;
    LOG_MESSAGE("File", "%s saved\n", file.filename().c_str() );

    return true;
}

bool File::read( File& file, const std::filesystem::path& path)
{
    LOG_MESSAGE("File", "\"%s\" loading... (%s).\n", path.filename().c_str(), path.c_str())
    if(path.empty() )
    {
        LOG_ERROR("File", "Path is empty \"%s\"\n", path.c_str())
        return false;
    }

    std::ifstream file_stream(path);
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