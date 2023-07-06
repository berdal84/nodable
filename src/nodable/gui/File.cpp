#include "File.h"

#include <fstream>
#include <utility>

#include "core/ConditionalStructNode.h"
#include "core/InstructionNode.h"
#include "core/InvokableComponent.h"
#include "core/LiteralNode.h"
#include "core/VariableNode.h"
#include "core/language/Nodlang.h"
#include "gui/FileView.h"
#include "gui/GraphNodeView.h"
#include "gui/History.h"
#include "gui/Nodable.h"
#include "gui/NodeView.h"

using namespace ndbl;

File::File(std::string _name)
        : name(std::move(_name))
        , changed(true)
        , view(*this)
        , m_graph(nullptr)
        , m_factory(&Nodlang::get_instance(), [](Node* _node) {

          // Code executed after node instantiation

          // add a view
          auto node_view = _node->add_component<NodeView>();

          // Set common colors
          const Config& config = Nodable::get_instance().config;
          node_view->set_color(fw::View::ColorType_Highlighted      , &config.ui_node_highlightedColor);
          node_view->set_color(fw::View::ColorType_Border           , &config.ui_node_borderColor);
          node_view->set_color(fw::View::ColorType_BorderHighlights , &config.ui_node_borderHighlightedColor);
          node_view->set_color(fw::View::ColorType_Shadow           , &config.ui_node_shadowColor);
          node_view->set_color(fw::View::ColorType_Fill             , &config.ui_node_fillColor);

          // Set specific colors
          if(_node->is<VariableNode>())
          {
              node_view->set_color(fw::View::ColorType_Fill, &config.ui_node_variableColor);
          }
          else if (_node->has_component<InvokableComponent>())
          {
              node_view->set_color(fw::View::ColorType_Fill, &config.ui_node_invokableColor);
          }
          else if (_node->is<InstructionNode>())
          {
              node_view->set_color(fw::View::ColorType_Fill, &config.ui_node_instructionColor);
          }
          else if (_node->is<LiteralNode>())
          {
              node_view->set_color(fw::View::ColorType_Fill, &config.ui_node_literalColor);
          }
          else if (_node->is<ConditionalStructNode>() || _node->is<ForLoopNode>())
          {
              node_view->set_color(fw::View::ColorType_Fill, &config.ui_node_condStructColor);
          }
        })
        , m_history(&Nodable::get_instance().config.experimental_hybrid_history)
{
    LOG_VERBOSE( "File", "Constructor being called ...\n")

    auto& language = Nodlang::get_instance();

    // FileView
    view.init();

    LOG_VERBOSE( "File", "View built, creating History ...\n")

    // History
    TextEditor*       text_editor     = view.get_text_editor();
    TextEditorBuffer* text_editor_buf = m_history.configure_text_editor_undo_buffer(text_editor);
    view.set_undo_buffer(text_editor_buf);

    LOG_VERBOSE( "File", "History built, creating graph ...\n")

    // GraphNode
    m_graph = new GraphNode(
            &language,
            &m_factory,
            &Nodable::get_instance().config.experimental_graph_autocompletion );

    char label[50];
    snprintf(label, sizeof(label), "%s's graph", name.c_str());
    m_graph->set_name(label);
    m_graph->add_component<GraphNodeView>();

    LOG_VERBOSE( "File", "Constructor being called.\n")

}

File::File(const ghc::filesystem::path& _path)
    : File(_path.filename().string())
{
    path = _path;
}

File::~File()
{
    delete m_graph;
}

bool File::write_to_disk()
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

bool File::update_graph(std::string& _code_source)
{
    LOG_VERBOSE("File","updating graph ...\n")
    m_graph->clear();

    GraphNodeView* graph_view = m_graph->get_component<GraphNodeView>();
    if (graph_view)
    {
        LOG_VERBOSE("File","clear graph view child constraints ...\n")
        graph_view->destroy_child_view_constraints();
    }

    if ( Nodlang::get_instance().parse(_code_source, m_graph) && !m_graph->is_empty() )
    {
        graph_view->create_child_view_constraints();
        m_graph->set_dirty(false);
        LOG_VERBOSE("File","graph changed, emiting event ...\n")
        event_graph_changed.emit(m_graph);
        return true;
    }
    return false;
}

bool File::update()
{
    Nodable &          app           = Nodable::get_instance();
    bool          graph_changed = false;

	if ( m_history.is_dirty() )
	{
        LOG_VERBOSE("File","history is dirty\n")
        if ( !app.config.experimental_hybrid_history )
        {
            update_graph(); // when not in hybrid mode the undo/redo is text based
        }

        m_history.set_dirty(false);
	}

    if( !app.virtual_machine.is_program_running() )
    {
        LOG_VERBOSE("File","m_graph->update()\n")
        auto result = m_graph->update();

        if ( result == UpdateResult::Success_NoChanges && !view.get_text().empty() )
        {
            LOG_VERBOSE("File","graph_has_changed = false\n")
            graph_changed = false;
        }
        else
        {
            LOG_VERBOSE("File","graph_has_changed = true\n")

            Node* root_node = m_graph->get_root();
            if ( root_node )
            {
                LOG_VERBOSE("File","serialize root node\n")

                std::string code;
                Nodlang::get_instance().serialize(code, root_node );

                LOG_VERBOSE("File","replace text\n")
                app.config.isolate_selection ? view.replace_selected_text(code) : view.replace_text(code);
            }
            graph_changed = true;
        }
    }
    else
    {
        LOG_VERBOSE("File","graph_has_changed = false\n")
        graph_changed = false;
    }

	return graph_changed;
}

bool File::update_graph()
{
    LOG_VERBOSE("File","get selected text\n")
	std::string code_source = Nodable::get_instance().config.isolate_selection ? view.get_selected_text() : view.get_text();
	return update_graph(code_source);
}

bool File::load()
{
    if(path.empty() )
    {
        return false;
    }

    LOG_MESSAGE( "File", "Loading file \"%s\"...\n", path.c_str())
    std::ifstream file_stream(path);
    LOG_VERBOSE( "File", "Input file stream created.\n")

    if (!file_stream.is_open())
    {
        LOG_ERROR("File", "Unable to load \"%s\"\n", path.c_str())
        return false;
    }

    LOG_VERBOSE("File", "Input file stream is open, reading content ...\n")
    std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());

    LOG_VERBOSE("File", "Content read, creating File object ...\n")
    view.set_text(content);
    changed = false;
    LOG_MESSAGE("File", "\"%s\" loaded (%s).\n", name.c_str(), path.c_str())

    return true;
}
