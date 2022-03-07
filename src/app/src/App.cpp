#include <nodable/App.h>

#include <string>
#include <algorithm>

#include <nodable/NodeView.h>
#include <nodable/AppView.h>
#include <nodable/File.h>
#include <nodable/BuildInfo.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/DataAccess.h>
#include <nodable/AppContext.h>

using namespace Nodable;

App::App(const char* _name)
    : m_currentFileIndex(0)
    , m_assetsFolderPath( ghc::filesystem::current_path() / BuildInfo::assets_dir )
    , m_name(_name)
    , m_should_stop(false)
{
    LOG_MESSAGE("App", "Asset folder is %s\n", m_assetsFolderPath.c_str() )

    m_context = AppContext::create_default(this);
	m_view = new AppView(m_context, _name);
}

App::~App()
{
	delete m_view;
	delete m_context;
}

bool App::init()
{
  R::Initialize();
  m_view->init();
	return true;
}

void App::update()
{
	File* file = get_curr_file();

	if (file)
	{
        file->update();
    }
}

void App::flag_to_stop()
{
    m_should_stop = true;
}

void App::shutdown()
{
    m_view->shutdown();
}

bool App::open_file(const ghc::filesystem::path& _filePath)
{		
	auto file = File::OpenFile(m_context, _filePath.string() );

	if (file != nullptr)
	{
		m_loadedFiles.push_back(file);
        set_curr_file(m_loadedFiles.size() - 1);
	}

	return file != nullptr;
}

void App::save_file() const
{
	auto currentFile = m_loadedFiles.at(m_currentFileIndex);
	if (currentFile)
		currentFile->save();
}

void App::close_file()
{
    this->close_file_at(this->m_currentFileIndex);
}

File* App::get_curr_file()const {

	if (m_loadedFiles.size() > m_currentFileIndex) {
		return m_loadedFiles.at(m_currentFileIndex);
	}
	return nullptr;
}

void App::set_curr_file(size_t _index)
{
	if (m_loadedFiles.size() > _index)
	{
        m_currentFileIndex = _index;
	}
}

std::string App::get_asset_path(const char* _fileName)const
{
    ghc::filesystem::path assetPath(m_assetsFolderPath);
	assetPath /= _fileName;
	return assetPath.string();
}

size_t App::get_file_count() const
{
	return m_loadedFiles.size();
}

File *App::get_file_at(size_t _index) const
{
	return m_loadedFiles[_index];
}

size_t App::get_curr_file_index() const
{
	return m_currentFileIndex;
}

void App::close_file_at(size_t _fileIndex)
{
    auto currentFile = m_loadedFiles.at(_fileIndex);
    if (currentFile != nullptr)
    {
        auto it = std::find(m_loadedFiles.begin(), m_loadedFiles.end(), currentFile);
        m_loadedFiles.erase(it);
        delete currentFile;
        if (m_currentFileIndex > 0)
            set_curr_file(m_currentFileIndex - 1);
        else
            set_curr_file(m_currentFileIndex);
    }
}

Node* App::get_curr_file_program_root() const
{
    if ( File* file = get_curr_file())
        return file->getGraph()->get_root();
    return nullptr;
}

void App::vm_run()
{
    Node* program = get_curr_file_program_root();
    if (program && m_context->vm->load_program(program) )
    {
        m_context->vm->run_program();
    }
}

void App::vm_debug()
{
    Node* program = get_curr_file_program_root();
    if (program && m_context->vm->load_program(program) )
    {
        m_context->vm->debug_program();
    }
}

void App::vm_step_over()
{
    m_context->vm->step_over();
    if (m_context->vm->is_program_over() )
    {
        NodeView::SetSelected(nullptr);
    }
    else if ( auto view = m_context->vm->get_next_node()->get<NodeView>() )
    {
        NodeView::SetSelected(view);
    }
}

void App::vm_stop()
{
    m_context->vm->stop_program();
}

void App::vm_reset()
{
    if ( auto currFile = get_curr_file() )
    {
        if(m_context->vm->is_program_running())
            m_context->vm->stop_program();

        // TODO: restore graph state without parsing again like that:
        currFile->evaluateSelectedExpression();
    }
}

