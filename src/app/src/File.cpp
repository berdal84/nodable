#include "File.h"

#include "Application.h"
#include "FileView.h"
#include "GraphNodeView.h"
#include "ProgramNode.h"
#include "Parser.h"
#include "LanguageFactory.h"
#include "History.h"
#include "NodeFactory.h"

#include <fstream>

using namespace Nodable::app;

File::File( std::filesystem::path _path, const char* _content)
    : m_path(_path)
    , m_modified(false)
    , m_open(false)
    , m_language(nullptr)
    , m_factory(nullptr)
{
    // TODO: Detect the language
    m_language = LanguageFactory::GetNodable();
    m_factory  = new app::NodeFactory(m_language);

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
    LOG_MESSAGE( "File", "Loading file \"%s\"...\n", _filePath.c_str());

	std::ifstream fileStream(_filePath);

	if (!fileStream.is_open())
	{
		LOG_ERROR("File", "Unable to load \"%s\"\n", _filePath.c_str());
		return nullptr;
	}

	// TODO: do that inside File constr ?
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	File* file = new File(_filePath.c_str(), content.c_str());
    file->m_open = true;

    LOG_MESSAGE( "File", "File \"%s\" loaded.\n", _filePath.c_str());

	return file;
}

bool File::evaluateExpression(std::string& _expression)
{
	Parser* parser = m_language->getParser();
    m_graph->clear();

    // Store the Result node position to restore it later
    // TODO: handle multiple results
    if ( auto program = m_graph->getProgram()  )
    {
        auto view = program->getComponent<NodeView>();
        GraphNode::s_mainScopeView_lastKnownPosition = view->getPosRounded();
    }

    auto graphView = m_graph->getComponent<GraphNodeView>();
    if (graphView)
    {
        graphView->clearConstraints();
    }

    if (parser->expressionToGraph(_expression, m_graph) && m_graph->hasProgram() )
    {
        if ( auto program = m_graph->getProgram() )
        {
            if (auto programView = program->getComponent<NodeView>())
            {
                bool hasKnownPosition = GraphNode::s_mainScopeView_lastKnownPosition.x != -1 &&
                                        GraphNode::s_mainScopeView_lastKnownPosition.y != -1;

                if ( graphView )
                {
                    if (hasKnownPosition)
                    {                                 /* if result node had a position stored, we restore it */
                        programView->setPosition(GraphNode::s_mainScopeView_lastKnownPosition);
                    }

                    auto graphViewRect = graphView->getVisibleRect();
                    graphViewRect.Expand(-20.f);
                    if (!NodeView::IsInsideRect(programView, graphViewRect))
                    {
                        programView->setPosition(programView->getSize() * 1.5f);
                    }
                }

                programView->arrangeRecursively(false);
            }
        }
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

    bool runner_is_stopped = Application::s_instance && Application::s_instance->getRunner().isStopped();
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
