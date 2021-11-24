#include <nodable/App.h>

#include <string>
#include <algorithm>

#include <nodable/NodeView.h>
#include <nodable/AppView.h>
#include <nodable/File.h>
#include <nodable/Config.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/DataAccess.h>

using namespace Nodable;

App* App::s_instance = nullptr;

App* App::Get()
{
    return App::s_instance;
}

App::App(const char* _name)
    : m_currentFileIndex(0)
    , m_assetsFolderPath(ghc::filesystem::current_path() / NODABLE_ASSETS_DIR)
    , m_name(_name)
{
    NODABLE_ASSERT(s_instance == nullptr); // can't create more than a single app
    LOG_MESSAGE("App", "Asset folder is %s\n", m_assetsFolderPath.c_str() )
    s_instance = this;
	m_view = new AppView(_name, this);
}

App::~App()
{
	delete m_view;
	s_instance = nullptr;
}

bool App::init()
{
  Reflect::Initialize();
  m_view->init();
	return true;
}

bool App::update()
{
	File* file = getCurrentFile();

	if (file)
	{
        file->update();
    }

    return !m_quit;
}

void App::stopExecution()
{
    m_quit = true;
}

void App::shutdown()
{
    m_view->shutdown();
}

bool App::openFile(const ghc::filesystem::path& _filePath)
{		
	auto file = File::OpenFile(_filePath.string() );

	if (file != nullptr)
	{
		m_loadedFiles.push_back(file);
		setCurrentFileWithIndex(m_loadedFiles.size() - 1);
	}

	return file != nullptr;
}

void App::saveCurrentFile() const
{
	auto currentFile = m_loadedFiles.at(m_currentFileIndex);
	if (currentFile)
		currentFile->save();
}

void App::closeCurrentFile()
{
    this->closeFile(this->m_currentFileIndex);
}

File* App::getCurrentFile()const {

	if (m_loadedFiles.size() > m_currentFileIndex) {
		return m_loadedFiles.at(m_currentFileIndex);
	}
	return nullptr;
}

void App::setCurrentFileWithIndex(size_t _index)
{
	if (m_loadedFiles.size() > _index)
	{
        m_currentFileIndex = _index;
	}
}

void App::SaveNode(Node* _node)
{
    auto component = new DataAccess;
	_node->addComponent(component);
	component->update();
	_node->deleteComponent<DataAccess>();
}

std::string App::getAssetPath(const char* _fileName)const
{
    ghc::filesystem::path assetPath(m_assetsFolderPath);
	assetPath /= _fileName;
	return assetPath.string();
}

size_t App::getFileCount() const
{
	return m_loadedFiles.size();
}

File *App::getFileAtIndex(size_t _index) const
{
	return m_loadedFiles[_index];
}

size_t App::getCurrentFileIndex() const
{
	return m_currentFileIndex;
}

void App::closeFile(size_t _fileIndex)
{
    auto currentFile = m_loadedFiles.at(_fileIndex);
    if (currentFile != nullptr)
    {
        auto it = std::find(m_loadedFiles.begin(), m_loadedFiles.end(), currentFile);
        m_loadedFiles.erase(it);
        delete currentFile;
        if (m_currentFileIndex > 0)
            setCurrentFileWithIndex(m_currentFileIndex - 1);
        else
            setCurrentFileWithIndex(m_currentFileIndex);
    }
}

ProgramNode *App::getCurrentFileProgram() const
{
    if ( File* file = getCurrentFile())
        return file->getGraph()->getProgram();
    return nullptr;
}

void App::runCurrentFileProgram()
{
    ProgramNode* program = getCurrentFileProgram();
    if (program)
    {
        m_runner.load(program);
        m_runner.run();
    }
}

void App::debugCurrentFileProgram()
{
    ProgramNode* program = getCurrentFileProgram();
    if (program)
    {
        m_runner.load(program);
        m_runner.debug();
    }
}

void App::stepOverCurrentFileProgram()
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

void App::stopCurrentFileProgram()
{
    m_runner.stop();
}

void App::resetCurrentFileProgram()
{
    if ( auto currFile = getCurrentFile() )
    {
        if( m_runner.isRunning())
            m_runner.stop();

        // TODO: restore graph state without parsing again like that:
        currFile->evaluateSelectedExpression();
    }
}

std::string App::GetAssetPath(const char *_path)
{
    return s_instance->getAssetPath( _path );
}
