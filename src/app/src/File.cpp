#include <nodable/File.h>

#include <fstream>

#include <nodable/App.h>
#include <nodable/FileView.h>
#include <nodable/GraphNodeView.h>
#include <nodable/ProgramNode.h>
#include <nodable/Parser.h>
#include <nodable/LanguageFactory.h>
#include <nodable/History.h>
#include <nodable/NodeFactory.h>

using namespace Nodable;

File::File( std::filesystem::path _path, const char* _content)
    : m_path(_path)
    , m_modified(false)
    , m_open(false)
    , m_language(nullptr)
    , m_factory(nullptr)
{
    // TODO: Detect the language
    m_language = LanguageFactory::GetNodable();
    m_factory  = new NodeFactory(m_language);

	// FileView
#ifndef NODABLE_HEADLESS
	m_view = new FileView(this);
    m_view->init();
    m_view->setText(_content);
	auto textEditor = m_view->getTextEditor();

	// History
	m_history = new History();
    auto undoBuffer = m_history->getUndoBuffer(textEditor);
    m_view->setUndoBuffer(undoBuffer);
#endif
	// GraphNode
    m_graph = new GraphNode( m_language, m_factory );
    m_graph->setLabel(_path.filename().string() + "'s inner container");

#ifndef NODABLE_HEADLESS
    m_graph->addComponent( new GraphNodeView() );
#endif
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

File* File::OpenFile(std::filesystem::path _filePath)
{
    LOG_MESSAGE( "File", "Loading file \"%s\"...\n", _filePath.c_str())

	std::ifstream fileStream(_filePath);

	if (!fileStream.is_open())
	{
		LOG_ERROR("File", "Unable to load \"%s\"\n", _filePath.c_str())
		return nullptr;
	}

	// TODO: do that inside File constr ?
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	File* file = new File(_filePath.c_str(), content.c_str());
    file->m_open = true;

    LOG_MESSAGE( "File", "File \"%s\" loaded.\n", _filePath.c_str())

	return file;
}

bool File::evaluateExpression(std::string& _expression)
{
	Parser* parser = m_language->getParser();
    m_graph->clear();

    auto graphView = m_graph->getComponent<GraphNodeView>();
    if (graphView)
    {
        graphView->clearConstraints();
    }

    if (parser->expressionToGraph(_expression, m_graph) && m_graph->hasProgram() )
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

    App* app = App::Get();
    bool runner_is_stopped = app && app->getRunner().isStopped();
	if( runner_is_stopped )
    {
        auto graphUpdateResult = m_graph->update();

        if (graphUpdateResult == UpdateResult::SuccessWithoutChanges && !m_view->getSelectedText().empty() )
        {
            return false;
        }

        auto scope = m_graph->getProgram();
        if ( scope && !scope->getChildren().empty() )
        {
            std::string code;
            m_language->getSerializer()->serialize(code, scope );
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
