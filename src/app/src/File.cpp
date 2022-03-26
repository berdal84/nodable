#include <nodable/app/File.h>

#include <fstream>

#include <nodable/app/App.h>
#include <nodable/app/FileView.h>
#include <nodable/app/GraphNodeView.h>
#include <nodable/core/Parser.h>
#include <nodable/app/History.h>
#include <nodable/app/AppNodeFactory.h>
#include <nodable/app/AppContext.h>

using namespace Nodable;

File::File( AppContext* _context, const std::string& _path, const std::string& _name,  const char* _content)
    : m_name(_name)
    , m_path(_path)
    , m_context(_context)
    , m_modified(false)
    , m_open(false)
    , m_graph(nullptr)
    , m_factory(_context)
    , m_history(&_context->settings->experimental_hybrid_history)
{
    LOG_VERBOSE( "File", "Constructor being called ...\n")

    // FileView
	m_view = new FileView(m_context, this);
    m_view->init();
    m_view->setText(_content);
	auto textEditor = m_view->getTextEditor();

    LOG_VERBOSE( "File", "View built, creating History ...\n")

	// History
    TextEditorBuffer* text_editor_buf = m_history.configure_text_editor_undo_buffer(textEditor);
    m_view->setUndoBuffer(text_editor_buf);

    LOG_VERBOSE( "File", "History built, creating graph ...\n")

	// GraphNode
    m_graph = new GraphNode(
            m_context->language,
            &m_factory,
            &m_context->settings->experimental_graph_autocompletion );
    m_graph->set_label(getName() + "'s inner container");

    m_graph->add_component(new GraphNodeView(m_context));

    LOG_VERBOSE( "File", "Constructor being called.\n")

}

void File::save()
{
	if ( m_modified )
	{
		std::ofstream fileStream(this->m_path.c_str());
		auto content = m_view->getText();
        fileStream.write(content.c_str(), content.size());
        m_modified = false;
	}

}

File* File::OpenFile(AppContext* _ctx, const std::string& _path, const std::string& _name )
{
    LOG_MESSAGE( "File", "Loading file \"%s\"...\n", _path.c_str())
	std::ifstream fileStream(_path);
    LOG_VERBOSE( "File", "Input file stream created.\n")

	if (!fileStream.is_open())
	{
		LOG_ERROR("File", "Unable to load \"%s\"\n", _path.c_str())
		return nullptr;
	}
    LOG_VERBOSE( "File", "Input file stream is open, reading content ...\n")

	// TODO: do that inside File constr ?
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

    LOG_VERBOSE( "File", "Content read, creating File object ...\n")

	File* file = new File(_ctx, _path, _name, content.c_str());
    file->m_open = true;

    LOG_MESSAGE( "File", "\"%s\" loaded (%s).\n", _name.c_str(), _path.c_str())

	return file;
}

bool File::update_graph(std::string& _code_source)
{
    LOG_VERBOSE("File","updating graph ...\n")
	Parser* parser = m_context->language->getParser();
    m_graph->clear();

    auto graphView = m_graph->get<GraphNodeView>();
    if (graphView)
    {
        LOG_VERBOSE("File","clear graph view child constraints ...\n")
        graphView->clear_child_view_constraints();
    }

    if ( parser->parse_graph(_code_source, m_graph) && !m_graph->is_empty() )
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
        if ( !m_context->settings->experimental_hybrid_history )
        {
            update_graph(); // when not in hybrid mode the undo/redo is text based
        }

        m_history.set_dirty(false);
	}

	if(m_context->vm && !m_context->vm->is_program_running() )
    {
        LOG_VERBOSE("File","m_graph->update()\n")
        auto graphUpdateResult = m_graph->update();

        if (   graphUpdateResult == UpdateResult::SuccessWithoutChanges
            && !m_view->getSelectedText().empty() )
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
                Serializer* serializer = m_context->language->getSerializer();
                serializer->serialize(code, root_node );

                LOG_VERBOSE("File","replace selected text\n")
                m_view->replaceSelectedText(code);
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
	std::string code_source = m_view->getSelectedText();
	return update_graph(code_source);
}

File::~File()
{
    delete m_graph;
    delete m_view;
}
