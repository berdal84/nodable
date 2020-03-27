#include "Application.h"
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Parser.h"
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

Application::Application(const char* _name):currentFileIndex(0)
{
	LOG_MSG("A new Application ( label = \"%s\")\n", _name);
	setMember("__class__", "Application");
	setLabel(_name);
	addComponent("view",      new ApplicationView(_name,    this));
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
	auto currentFile = loadedFiles.at(currentFileIndex);
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
		auto newFile = loadedFiles.at(_index);
		currentFileIndex = _index;
	}
}

void Application::SaveEntity(Entity* _entity)
{
    auto component = new DataAccess;
	_entity->addComponent("dataAccess", component);
	component->update();
	_entity->removeComponent("dataAccess");
}