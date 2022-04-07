#include <nodable/app/File.h>

#include <fstream>

#include <nodable/app/App.h>
#include <nodable/app/FileView.h>
#include <nodable/app/GraphNodeView.h>
#include <nodable/core/Parser.h>
#include <nodable/app/History.h>
#include <nodable/app/IAppCtx.h>
#include <nodable/app/NodeView.h>

using namespace Nodable;

File::File(IAppCtx& _ctx, const std::string &_name)
        : m_name(_name)
        , m_ctx(_ctx)
        , m_modified(true)
        , m_graph(nullptr)
        , m_factory(&_ctx.language(), [&_ctx](Node* _node) { _node->add_component(new NodeView(_ctx)); } )
        , m_history(&_ctx.settings().experimental_hybrid_history)
{
    LOG_VERBOSE( "File", "Constructor being called ...\n")

    // FileView
    m_view = new FileView(m_ctx, *this);
    m_view->init();

    LOG_VERBOSE( "File", "View built, creating History ...\n")

    // History
    TextEditor* text_editor = m_view->get_text_editor();
    TextEditorBuffer* text_editor_buf = m_history.configure_text_editor_undo_buffer(text_editor);
    m_view->set_undo_buffer(text_editor_buf);

    LOG_VERBOSE( "File", "History built, creating graph ...\n")

    // GraphNode
    m_graph = new GraphNode(
            &m_ctx.language(),
            &m_factory,
            &m_ctx.settings().experimental_graph_autocompletion );

    char label[50];
    snprintf(label, sizeof(label), "%s's graph", get_name().c_str());
    m_graph->set_label( label );

    m_graph->add_component(new GraphNodeView(m_ctx));

    LOG_VERBOSE( "File", "Constructor being called.\n")

}

File::File(IAppCtx& _context, const std::string &_name, const std::string &_path)
    : File(_context, _name)
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
	}

    return true;
}

bool File::update_graph(std::string& _code_source)
{
    LOG_VERBOSE("File","updating graph ...\n")
    m_graph->clear();

    auto graphView = m_graph->get<GraphNodeView>();
    if (graphView)
    {
        LOG_VERBOSE("File","clear graph view child constraints ...\n")
        graphView->clear_child_view_constraints();
    }

    Parser& parser = m_ctx.language().get_parser();
    if ( parser.parse_graph(_code_source, m_graph) && !m_graph->is_empty() )
    {
        LOG_VERBOSE("File","graph changed, emiting event ...\n")
        m_on_graph_changed_evt.emit(m_graph);
        return true;
    }
    return false;
}

bool File::update()
{
    bool graph_has_changed;

	if ( m_history.is_dirty() )
	{
        LOG_VERBOSE("File","history is dirty\n")
        if ( !m_ctx.settings().experimental_hybrid_history )
        {
            update_graph(); // when not in hybrid mode the undo/redo is text based
        }

        m_history.set_dirty(false);
	}

    if( m_ctx.virtual_machine().is_program_running() )
    {
        LOG_VERBOSE("File","m_graph->update()\n")
        auto graphUpdateResult = m_graph->update();

        if (   graphUpdateResult == UpdateResult::SuccessWithoutChanges
            && !m_view->get_selected_text().empty() )
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
                m_ctx.language()
                     .get_serializer()
                     .serialize(code, root_node );

                LOG_VERBOSE("File","replace selected text\n")
                m_view->replace_selected_text(code);
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
	std::string code_source = m_view->get_selected_text();
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
