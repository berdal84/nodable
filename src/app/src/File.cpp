#include <nodable/File.h>

#include <fstream>

#include <nodable/App.h>
#include <nodable/FileView.h>
#include <nodable/GraphNodeView.h>
#include <nodable/Parser.h>
#include <nodable/History.h>
#include <nodable/AppNodeFactory.h>
#include <nodable/AppContext.h>

using namespace Nodable;

File::File( AppContext* _context, std::string _path, const char* _content)
    : m_path(_path)
    , m_context( _context )
    , m_modified(false)
    , m_open(false)
    , m_factory(nullptr)
{
    LOG_VERBOSE( "File", "Constructor being called ...\n");
    m_factory  = new AppNodeFactory(_context);
    LOG_VERBOSE( "File", "Factory created, creating View ...\n");

    // FileView
#ifndef NODABLE_HEADLESS
	m_view = new FileView(m_context, this);
    m_view->init();
    m_view->setText(_content);
	auto textEditor = m_view->getTextEditor();

    LOG_VERBOSE( "File", "View built, creating History ...\n");

	// History
	m_history = new History();
    auto undoBuffer = m_history->getUndoBuffer(textEditor);
    m_view->setUndoBuffer(undoBuffer);

    LOG_VERBOSE( "File", "History built, creating graph ...\n");

#endif
	// GraphNode
    m_graph = new GraphNode(m_context->language, m_factory );
    m_graph->set_label(getName() + "'s inner container");

#ifndef NODABLE_HEADLESS
    m_graph->add_component(new GraphNodeView(m_context));
#endif

    LOG_VERBOSE( "File", "Constructor being called.\n");

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

File* File::OpenFile(AppContext* _ctx, std::string _filePath)
{
    LOG_MESSAGE( "File", "Loading file \"%s\"...\n", _filePath.c_str())
	std::ifstream fileStream(_filePath);
    LOG_VERBOSE( "File", "Input file stream created.\n");

	if (!fileStream.is_open())
	{
		LOG_ERROR("File", "Unable to load \"%s\"\n", _filePath.c_str())
		return nullptr;
	}
    LOG_VERBOSE( "File", "Input file stream is open, reading content ...\n");

	// TODO: do that inside File constr ?
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

    LOG_VERBOSE( "File", "Content read, creating File object ...\n");

	File* file = new File(_ctx, _filePath.c_str(), content.c_str());
    file->m_open = true;

    LOG_MESSAGE( "File", "File \"%s\" loaded.\n", _filePath.c_str())

	return file;
}

bool File::evaluateExpression(std::string& _expression)
{
	Parser* parser = m_context->language->getParser();
    m_graph->clear();

    auto graphView = m_graph->get<GraphNodeView>();
    if (graphView)
    {
        graphView->clear_child_view_constraints();
    }

    if (parser->source_code_to_graph(_expression, m_graph) && m_graph->hasProgram() )
    {
        m_onExpressionParsedIntoGraph.emit(m_graph->getProgram() );
        return true;
    }
    return false;
}

bool File::update() {

	if ( m_history && m_history->dirty )
	{
        evaluateSelectedExpression();
        m_history->dirty = false;
	}

	if(m_context->vm && m_context->vm->is_program_stopped() )
    {
        auto graphUpdateResult = m_graph->update();

        if (graphUpdateResult == UpdateResult::SuccessWithoutChanges && !m_view->getSelectedText().empty() )
        {
            return false;
        }

        auto scope = m_graph->getProgram();
        if ( scope && !scope->children_slots().empty() )
        {
            std::string code;
            m_context->language->getSerializer()->serialize(code, scope );
            m_view->replaceSelectedText(code);
        }
    }
	return true;
}

bool File::evaluateSelectedExpression()
{
	bool success;
	auto expression = m_view->getSelectedText();
	success = evaluateExpression(expression);
	return success;
}

File::~File()
{
    delete m_graph;
    delete m_factory;
    delete m_view;
}
