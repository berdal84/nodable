#include "Application.h"
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Lexer.h"
#include "Container.h"
#include "ContainerView.h"
#include "ApplicationView.h"
#include "Variable.h"
#include "DataAccess.h"
#include "FileView.h"

#include "File.h"
#include <iostream>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <string>
#include <algorithm>

using namespace Nodable;

Application::Application(const char* _name):currentlyActiveLoadedFileIndex(0)
{
	LOG_MSG("A new Application ( label = \"%s\")\n", _name);
	setMember("__class__", "Application");
	setLabel(_name);
	addComponent("view",      new ApplicationView(_name,    this));

	// Add an History component for UNDO/REDO
	auto h = new History;
	addComponent("history", h);
	History::global = h;
}

Application::~Application()
{
	for (auto it = loadedFiles.begin(); it != loadedFiles.end(); it++)
		delete* it;
}

bool Application::init()
{
	LOG_MSG("init application ( label = \"%s\")\n", getLabel());

	auto view = reinterpret_cast<ApplicationView*>(getComponent("view"));
	view->init();
	openFile("data/startup.txt");

	return true;
}

bool Application::update()
{
	auto file = getCurrentFile();
	
	if (!file)
		return !quit;
	
	file->update();

	return !quit;
}

void Application::stopExecution()
{
	quit = true;
}

void Application::shutdown()
{
	LOG_MSG("shutting down application ( _name = \"%s\")\n", getLabel());
}

bool Application::openFile(const char* _filePath)
{		
	auto file     = File::CreateFileWithPath(_filePath);
	auto fileView = reinterpret_cast<View*>( file->getComponent("view") );

	if (file != nullptr)
	{
		loadedFiles.push_back(file);
		setCurrentlyActiveLoadedFileWithIndex(loadedFiles.size() - 1);
		auto view = reinterpret_cast<ApplicationView*>(getComponent("view"));

		view->setCurrentFileView(fileView);
	}

	return file != nullptr;
}

void Application::saveCurrentlyActiveFile() const
{
	auto currentFile = loadedFiles.at(currentlyActiveLoadedFileIndex);
	if (currentFile != nullptr)
	{
		currentFile->save();
	}
}

void Application::setCurrentlyActiveFileContent(std::string& _content)
{
	auto currentFile = loadedFiles.at(currentlyActiveLoadedFileIndex);
	if (currentFile != nullptr)
	{
		currentFile->setContent(_content);
	}
}

void Application::closeCurrentlyActiveFile()
{
	auto currentFile = loadedFiles.at(currentlyActiveLoadedFileIndex);
	if (currentFile != nullptr)
	{
		auto it = std::find(loadedFiles.begin(), loadedFiles.end(), currentFile);
		loadedFiles.erase(it);
		delete currentFile;
		if (currentlyActiveLoadedFileIndex > 0)
			setCurrentlyActiveLoadedFileWithIndex(currentlyActiveLoadedFileIndex - 1);
		else
			setCurrentlyActiveLoadedFileWithIndex(currentlyActiveLoadedFileIndex);
	}
}

File* Application::getCurrentFile()const {
	return loadedFiles.at(currentlyActiveLoadedFileIndex);
}

void Application::setCurrentlyActiveLoadedFileWithIndex(size_t _index)
{
	auto view = reinterpret_cast<ApplicationView*>(getComponent("view"));

	/* Then we set desired file as active */
	if (loadedFiles.size() > _index)
	{
		auto newFile = loadedFiles.at(_index);
		currentlyActiveLoadedFileIndex = _index;
	}

	// clearContextAndEvalHighlightedExpression();
}

void Application::SaveEntity(Entity* _entity)
{
    auto component = new DataAccess;
	_entity->addComponent("dataAccess", component);
	component->update();
	_entity->removeComponent("dataAccess");
}