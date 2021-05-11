#include "Core/File.h"
#include "Core/Log.h"
#include "Core/Application.h"
#include "Component/History.h"
#include "Component/FileView.h"
#include "Component/GraphNodeView.h"
#include "Node/GraphNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/ProgramNode.h"
#include "Language/Common/Parser.h"
#include "Language/Common/LanguageFactory.h"
#include "Node/DefaultNodeFactory.h"

#include <fstream>

using namespace Nodable;

Nodable::File::File( std::filesystem::path _path, const char* _content)
    : m_path(_path)
    , m_modified(false)
    , m_open(false)
    , m_language(nullptr)
    , m_factory(nullptr)
{
    // TODO: Detect the language
    m_language = LanguageFactory::GetNodable();
    m_factory  = new DefaultNodeFactory(m_language);

	// FileView
#ifndef NODABLE_HEADLESS
	auto fileView = new FileView();
	addComponent(fileView);
#endif
	fileView->init();
	fileView->setText(_content);
	auto textEditor = fileView->getTextEditor();

	// History
	auto history = new History();
	addComponent(history);
    auto undoBuffer = history->getUndoBuffer(textEditor);
	fileView->setUndoBuffer(undoBuffer);
	
	// GraphNode
	auto graphNode = new GraphNode( m_language, m_factory );
	graphNode->setLabel(_path.filename().string() + "'s inner container");
    setInnerGraph(graphNode);
#ifndef NODABLE_HEADLESS
	auto graphNodeView = new GraphNodeView();
	graphNode->addComponent(graphNodeView);
#endif
}

void File::save()
{
	if ( m_modified )
	{
		std::ofstream fileStream(this->m_path.c_str());
		auto view    = getComponent<FileView>();
		auto content = view->getText();
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
	GraphNode* graph = getInnerGraph();
    graph->clear();
    if (parser->expressionToGraph(_expression, graph) && graph->hasProgram() )
    {
#ifndef NODABLE_HEADLESS
        graph->arrangeNodeViews();
#endif
        return true;
    }
    return false;
}

UpdateResult File::update() {

	if (auto history = this->getComponent<History>())
	{
		if (history->dirty)
		{
			this->evaluateSelectedExpression();
			history->dirty = false;
		}
	}
    bool runner_is_stopped = Application::s_instance && Application::s_instance->getRunner().isStopped();
	if( runner_is_stopped )
    {
        auto graphUpdateResult = getInnerGraph()->update();
        auto view = getComponent<FileView>();

        if (graphUpdateResult == UpdateResult::SuccessWithoutChanges && !view->getSelectedText().empty() )
        {
            return UpdateResult::SuccessWithoutChanges;
        }

        auto scope = getInnerGraph()->getProgram();
        if ( scope && !scope->getChildren().empty() )
        {
            std::string code;
            m_language->getSerializer()->serialize(code, scope );
            view->replaceSelectedText(code);
        }
    }
	return UpdateResult::Success;
}

bool File::evaluateSelectedExpression()
{
	bool success;
	auto view = getComponent<FileView>();
	auto expression = view->getSelectedText();
	success = evaluateExpression(expression);
	return success;
}

bool& File::isOpen() {
    return m_open;
}

File::~File()
{
    delete getInnerGraph();
    delete m_factory;
}
