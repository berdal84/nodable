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
	addComponent(new ApplicationView(_name, this));
}

Application::~Application()
{
	for (auto it = loadedFiles.begin(); it != loadedFiles.end(); it++)
		delete* it;
}

bool Application::init()
{
	auto view = getComponent<ApplicationView>();
	view->init();

	return true;
}

UpdateResult Application::update()
{
	File* file = getCurrentFile();
    UpdateResult fileUpdateResult = UpdateResult::Failed;

	if (file)
	{
        fileUpdateResult = file->update();
    }

    if( quit )
    {
        return UpdateResult::Stopped;
    }
    else
    {
        return fileUpdateResult;
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
