#include "Application.h"

#include <string>
#include <algorithm>
#include <NodeView.h>

#include "ApplicationView.h"
#include "File.h"
#include "Config.h"
#include "GraphNode.h"
#include "VariableNode.h"
#include "DataAccess.h"

using namespace Nodable;

Application* Application::s_instance = nullptr;

Application::Application(const char* _name)
    : currentFileIndex(0)
    , assetsFolderPath(NODABLE_ASSETS_DIR)
    , m_name(_name)
{
    NODABLE_ASSERT(s_instance == nullptr); // can't create more than a single app
    s_instance = this;
	m_view = new ApplicationView(_name, this);
}

Application::~Application()
{
	delete m_view;
	s_instance = nullptr;
}

bool Application::init()
{
    m_view->init();
	return true;
}

bool Application::update()
{
	File* file = getCurrentFile();

	if (file)
	{
        file->update();
    }

    if( quit )
    {
        return false;
    }
    return true;
}

void Application::stopExecution()
{
	quit = true;
}

void Application::shutdown()
{
    m_view->shutdown();
}

bool Application::openFile(std::filesystem::path _filePath)
{		
	auto file = File::OpenFile(_filePath);

	if (file != nullptr)
	{
		loadedFiles.push_back(file);
		setCurrentFileWithIndex(loadedFiles.size() - 1);
	}

	return file != nullptr;
}

void Application::saveCurrentFile() const
{
	auto currentFile = loadedFiles.at(currentFileIndex);
	if (currentFile)
		currentFile->save();
}

void Application::closeCurrentFile()
{
    this->closeFile(this->currentFileIndex);
}

File* Application::getCurrentFile()const {

	if (loadedFiles.size() > currentFileIndex) {
		return loadedFiles.at(currentFileIndex);
	}
	return nullptr;
}

void Application::setCurrentFileWithIndex(size_t _index)
{
	if (loadedFiles.size() > _index)
	{
		currentFileIndex = _index;
	}
}

void Application::SaveNode(Node* _node)
{
    auto component = new DataAccess;
	_node->addComponent(component);
	component->update();
	_node->deleteComponent<DataAccess>();
}

std::filesystem::path Application::getAssetPath(const char* _fileName)const
{
	auto assetPath = this->assetsFolderPath;
	assetPath /= _fileName;
	return assetPath;
}

size_t Application::getFileCount() const
{
	return loadedFiles.size();
}

File *Application::getFileAtIndex(size_t _index) const
{
	return loadedFiles[_index];
}

size_t Application::getCurrentFileIndex() const
{
	return currentFileIndex;
}

void Application::closeFile(size_t _fileIndex)
{
    auto currentFile = loadedFiles.at(_fileIndex);
    if (currentFile != nullptr)
    {
        auto it = std::find(loadedFiles.begin(), loadedFiles.end(), currentFile);
        loadedFiles.erase(it);
        delete currentFile;
        if (currentFileIndex > 0)
            setCurrentFileWithIndex(currentFileIndex - 1);
        else
            setCurrentFileWithIndex(currentFileIndex);
    }
}

ProgramNode *Application::getCurrentFileProgram() const
{
    if ( File* file = getCurrentFile())
        return file->getGraph()->getProgram();
    return nullptr;
}

void Application::runCurrentFileProgram()
{
    ProgramNode* program = getCurrentFileProgram();
    if (program)
    {
        m_runner.load(program);
        m_runner.run();
    }
}

void Application::debugCurrentFileProgram()
{
    ProgramNode* program = getCurrentFileProgram();
    if (program)
    {
        m_runner.load(program);
        m_runner.debug();
    }
}

void Application::stepOverCurrentFileProgram()
{
    m_runner.stepOver();
    if ( m_runner.isProgramOver() )
    {
        NodeView::SetSelected(nullptr);
    }
    else if ( auto view = m_runner.getCurrentNode()->getComponent<NodeView>() )
    {
        NodeView::SetSelected(view);
    }
}

void Application::stopCurrentFileProgram()
{
    m_runner.stop();
}

void Application::resetCurrentFileProgram()
{
    if ( auto currFile = getCurrentFile() )
    {
        if( m_runner.isRunning())
            m_runner.stop();

        // TODO: restore graph state without parsing again like that:
        currFile->evaluateSelectedExpression();
    }
}
