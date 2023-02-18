#include <ndbl/gui/File.h>

#include <fstream>
#include <utility>

#include <ndbl/gui/App.h>
#include <ndbl/gui/FileView.h>
#include <ndbl/gui/GraphNodeView.h>
#include <ndbl/gui/History.h>
#include <ndbl/gui/NodeView.h>
#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/InvokableComponent.h>
#include <ndbl/core/LiteralNode.h>
#include <ndbl/core/VariableNode.h>
#include <ndbl/core/language/Nodlang.h>

using namespace ndbl;

File::File(std::string _name)
        : m_name(std::move(_name))
        , m_modified(true)
        , m_graph(nullptr)
        , m_factory(&Nodlang::get_instance(), [](Node* _node) {

          // Code executed after node instantiation

          // add a view
          auto view = _node->add_component<NodeView>();

          // Set common colors
          const Settings& settings = App::get_instance().settings;
          view->set_color(fw::View::ColorType_Highlighted      , &settings.ui_node_highlightedColor);
          view->set_color(fw::View::ColorType_Border           , &settings.ui_node_borderColor);
          view->set_color(fw::View::ColorType_BorderHighlights , &settings.ui_node_borderHighlightedColor);
          view->set_color(fw::View::ColorType_Shadow           , &settings.ui_node_shadowColor);
          view->set_color(fw::View::ColorType_Fill             , &settings.ui_node_fillColor);

          // Set specific colors
          if(_node->is<VariableNode>())
          {
              view->set_color(fw::View::ColorType_Fill, &settings.ui_node_variableColor);
          }
          else if (_node->has<InvokableComponent>())
          {
              view->set_color(fw::View::ColorType_Fill, &settings.ui_node_invokableColor);
          }
          else if (_node->is<InstructionNode>())
          {
              view->set_color(fw::View::ColorType_Fill, &settings.ui_node_instructionColor);
          }
          else if (_node->is<LiteralNode>())
          {
              view->set_color(fw::View::ColorType_Fill, &settings.ui_node_literalColor);
          }
        })
        , m_history(&App::get_instance().settings.experimental_hybrid_history)
{
    LOG_VERBOSE( "File", "Constructor being called ...\n")

    auto& language = Nodlang::get_instance();

    // FileView
    m_view = new FileView(*this);
    m_view->init();

    LOG_VERBOSE( "File", "View built, creating History ...\n")

    // History
    TextEditor* text_editor = m_view->get_text_editor();
    TextEditorBuffer* text_editor_buf = m_history.configure_text_editor_undo_buffer(text_editor);
    m_view->set_undo_buffer(text_editor_buf);

    LOG_VERBOSE( "File", "History built, creating graph ...\n")

    // GraphNode
    m_graph = new GraphNode(
            &language,
            &m_factory,
            &App::get_instance().settings.experimental_graph_autocompletion );

    char label[50];
    snprintf(label, sizeof(label), "%s's graph", get_name().c_str());
    m_graph->set_name(label);
    m_graph->add_component<GraphNodeView>();

    LOG_VERBOSE( "File", "Constructor being called.\n")

}

File::File(const std::string &_name, const std::string &_path)
    : File(_name)
{
    m_path = _path;
}

File::~File()
{
    delete m_graph;
    delete m_view;
}

bool File::write_to_disk()
{
    if( m_path.empty() )
    {
        return false;
    }

	if ( m_modified )
	{
		std::ofstream out_fstream(m_path.c_str());
        std::string content = m_view->get_text();
        out_fstream.write(content.c_str(), content.size());
        m_modified = false;
        LOG_MESSAGE("File", "%s saved\n", m_name.c_str());
	} else {
        LOG_MESSAGE("File", "Nothing to save\n");
    }

    return true;
}

bool File::update_graph(std::string& _code_source)
{
    LOG_VERBOSE("File","updating graph ...\n")
    m_graph->clear();

    auto graph_view = m_graph->get<GraphNodeView>();
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
        m_on_graph_changed_evt.emit(m_graph);
        return true;
    }
    return false;
}

bool File::update()
{
    const Settings& settings    = App::get_instance().settings;
    const auto& virtual_machine = VirtualMachine::get_instance();
    bool graph_has_changed;

	if ( m_history.is_dirty() )
	{
        LOG_VERBOSE("File","history is dirty\n")
        if ( !settings.experimental_hybrid_history )
        {
            update_graph(); // when not in hybrid mode the undo/redo is text based
        }

        m_history.set_dirty(false);
	}

    if( !virtual_machine.is_program_running() )
    {
        LOG_VERBOSE("File","m_graph->update()\n")
        auto graphUpdateResult = m_graph->update();

        if (   graphUpdateResult == UpdateResult::Success_NoChanges && !m_view->get_text().empty() )
        {
            LOG_VERBOSE("File","graph_has_changed = false\n")
            graph_has_changed = false;
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
                settings.isolate_selection ? m_view->replace_selected_text(code) : m_view->replace_text(code);
            }
            graph_has_changed = true;
        }
    }
    else
    {
        LOG_VERBOSE("File","graph_has_changed = false\n")
        graph_has_changed = false;
    }

	return graph_has_changed;
}

bool File::update_graph()
{
    LOG_VERBOSE("File","get selected text\n")
	std::string code_source = App::get_instance().settings.isolate_selection ? m_view->get_selected_text() : m_view->get_text();
	return update_graph(code_source);
}

bool File::read_from_disk()
{
    if( m_path.empty() )
    {
        return false;
    }

    LOG_MESSAGE( "File", "Loading file \"%s\"...\n", m_path.c_str())
    std::ifstream file_stream(m_path);
    LOG_VERBOSE( "File", "Input file stream created.\n")

    if (!file_stream.is_open())
    {
        LOG_ERROR("File", "Unable to load \"%s\"\n", m_path.c_str())
        return false;
    }

    LOG_VERBOSE("File", "Input file stream is open, reading content ...\n")
    std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());

    LOG_VERBOSE("File", "Content read, creating File object ...\n")
    m_view->set_text(content);

    m_modified = false;
    LOG_MESSAGE("File", "\"%s\" loaded (%s).\n", m_name.c_str(), m_path.c_str())

    return true;
}

void File::set_path(const std::string& _path)
{
    m_path = _path;
}
