#include "Application.h"
#include "ApplicationView.h"
#include "Variable.h"
#include "DataAccess.h"
#include "File.h"
#include "Config.h"

#include <string>
#include <algorithm>

using namespace Nodable;

Application::Application(const char* _name):
    currentFileIndex(0),
    assetsFolderPath(NODABLE_ASSETS_DIR)
{
	set("__class__", "Application");
	setLabel(_name);
}

Application::~Application()
{
	loadedFiles.clear();
}

bool Application::init()
{
	auto view = getComponent<ApplicationView>();
	view->init();

	return true;
}

UpdateResult Application::update()
{
	auto file = getCurrentFile();
	
	if (!file)
	{
		return UpdateResult::Failed;
	}
	else
	{
		const auto fileUpdateResult = file->update();

		if( quit )
        {
		    return UpdateResult::Stopped;
        }
		else
        {
            return fileUpdateResult;
        }
	}
}

void Application::stopExecution()
{
	quit = true;
}

void Application::shutdown()
{
}

bool Application::openFile(std::filesystem::path _filePath)
{		
	auto file = File::OpenFile(_filePath);

	if (file != nullptr)
	{
		loadedFiles.push_back( std::move(file) );
		setCurrentFileWithIndex(loadedFiles.size() - 1);
	}

	return file != nullptr;
}

void Application::saveCurrentFile() const
{
	auto currentFile = getCurrentFile();
	if (currentFile)
		currentFile->save();
}

void Application::closeCurrentFile()
{
    auto it = std::find(loadedFiles.begin(), loadedFiles.end(), loadedFiles.at(currentFileIndex));

    loadedFiles.erase(it);

    if (currentFileIndex > 0)
        setCurrentFileWithIndex(currentFileIndex - 1);
    else
        setCurrentFileWithIndex(currentFileIndex);
}

File* Application::getCurrentFile()const {

	if (loadedFiles.size() > currentFileIndex) {
		return loadedFiles.at(currentFileIndex).get();
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
    auto component = _node->newComponent<DataAccess>().lock();
	component->update();
	_node->deleteComponent<DataAccess>();
}

std::filesystem::path Application::getAssetPath(const char* _fileName)const
{
	auto assetPath = this->assetsFolderPath;
	assetPath /= _fileName;
	return assetPath;
}