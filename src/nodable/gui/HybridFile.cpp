#include "HybridFile.h"

#include <fstream>
#include <utility>

#include "core/InstructionNode.h"
#include "core/InvokableComponent.h"
#include "core/LiteralNode.h"
#include "GraphView.h"
#include "History.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "core/NodeUtils.h"

using namespace ndbl;
using namespace fw;

HybridFile::HybridFile(std::string _name)
        : name(std::move(_name))
        , changed(true)
        , view(*this)
        , m_history(&Nodable::get_instance().config.experimental_hybrid_history)
{
    LOG_VERBOSE( "File", "Constructor being called ...\n")

    // FileView
    view.init();

    LOG_VERBOSE( "File", "View built, creating History ...\n")

    // History
    TextEditor*       text_editor     = view.get_text_editor();
    TextEditorBuffer* text_editor_buf = m_history.configure_text_editor_undo_buffer(text_editor);
    view.set_undo_buffer(text_editor_buf);

    LOG_VERBOSE( "File", "History built, creating graph ...\n")

    // Graph
    m_graph      = new Graph(&Nodable::get_instance().node_factory);
    m_graph_view = new GraphView(m_graph);

    LOG_VERBOSE( "File", "Constructor being called.\n")
}

HybridFile::HybridFile(const ghc::filesystem::path& _path)
    : HybridFile(_path.filename().string())
{
    path = _path;
}

HybridFile::~HybridFile()
{
    delete m_graph;
    delete m_graph_view;
}

bool HybridFile::write_to_disk()
{
    if(path.empty() )
    {
        return false;
    }

	if (changed)
	{
		std::ofstream out_fstream(path.c_str());
        std::string content = view.get_text();
        out_fstream.write(content.c_str(), content.size());
        changed = false;
        LOG_MESSAGE("File", "%s saved\n", name.c_str());
	} else {
        LOG_MESSAGE("File", "Nothing to save\n");
    }

    return true;
}

bool HybridFile::load()
{
    LOG_MESSAGE("HybridFile", "\"%s\" loading... (%s).\n", name.c_str(), path.c_str())
    if(path.empty() )
    {
        LOG_ERROR("HybridFile", "Path is empty \"%s\"\n", path.c_str())
        return false;
    }

    std::ifstream file_stream(path);
    if (!file_stream.is_open())
    {
        LOG_ERROR("HybridFile", "Unable to load \"%s\"\n", path.c_str())
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
    set_text(content);

    changed = false;

    LOG_MESSAGE("HybridFile", "\"%s\" loaded (%s).\n", name.c_str(), path.c_str())

    return true;
}

std::string HybridFile::get_text(bool selection_only) const
{
    return selection_only ? view.get_selected_text() : view.get_text();;
}

void HybridFile::set_text(const std::string& text)
{
    view.set_text(text);
}

UpdateResult HybridFile::update_text_from_graph(bool isolate_selection)
{
    PoolID<Node> root_node = m_graph->get_root();
    if ( root_node == PoolID<Node>::null )
    {
        return UpdateResult::SUCCES_WITHOUT_CHANGES;
    }
    std::string code;
    Nodlang::get_instance().serialize_node( code, root_node );
    isolate_selection ? view.replace_selected_text(code)
                      : view.replace_text(code);
    return UpdateResult::SUCCESS_WITH_CHANGES;
}

UpdateResult HybridFile::update()
{
    //
    // TODO: Maybe the config should be global?
    //       like bool isolate_selection = Config::isolate_selection;
    //
    bool isolate_selection = Nodable::get_instance().config.isolate_selection;

    if( view.changed() )
    {
        if( view.focused_text_changed() && !view.graph_changed() )
        {
            update_graph_from_text(isolate_selection);
        }
        else if ( view.graph_changed() )
        {
            update_text_from_graph(isolate_selection);
        }
        else
        {
            // TODO: The case where both focused_text and graph changed is not handled yet
            //       This is not supposed to happens, that's why there is an assert to be aware of is
            FW_ASSERT(false);
        }
        view.changed(false);
    }

    return  m_graph->update();
}

UpdateResult HybridFile::update_graph_from_text(bool isolate_selection)
{
    // Destroy all physics' constraints
    auto physics_components = NodeUtils::get_components<Physics>( m_graph->get_node_registry() );
    Physics::destroy_constraints( physics_components );

    // Parse source code
    bool parse_ok = Nodlang::get_instance().parse( get_text(isolate_selection), m_graph);
    if (parse_ok && !m_graph->is_empty() )
    {
        Physics::create_constraints( m_graph->get_node_registry() );
        graph_changed.emit(m_graph);
        return UpdateResult::SUCCESS_WITH_CHANGES;
    }
    return UpdateResult::SUCCES_WITHOUT_CHANGES;

}
