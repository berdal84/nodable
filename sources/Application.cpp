#include "Application.h"
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Lexer.h"
#include "Container.h"
#include "ContainerView.h"
#include "ApplicationView.h"
#include "Variable.h"
#include "DataAccess.h"
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

	// Add a container to the application to contain all nodes :
	auto container = new Container;
	addComponent("container", container);
	container->addComponent("view", new ContainerView);
	container->setOwner(this);

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

void Application::clearContext()
{
	if( hasComponent("container"))
	{
		auto container = getComponent("container")->getAs<Container*>();	
		container->clear();
	}
	History::global->clear();
}

Container* Application::getContext()const
{
	auto container = getComponent("container")->getAs<Container*>();
	NODABLE_VERIFY(container != nullptr);
	return container;
}

bool Application::update()
{
	auto ctx = getContext();
	if(ctx != nullptr)
		ctx->update();
	return !quit;
}

void Application::replaceHighlightedPortionInTextEditor(std::string _val)
{
	if (hasComponent("view"))
	{
		auto view =	getComponent("view")->getAs<ApplicationView*>();
		view->replaceHighlightedPortionInTextEditor(_val);
	}
}

void Application::stopExecution()
{
	quit = true;
}

bool Application::eval(std::string _expression)
{
	LOG_MSG("Application::eval() - create a variable.\n");

	auto container = getComponent("container")->getAs<Container*>();
	NODABLE_VERIFY(container != nullptr);

	currentExpressionStringVariable = container->createNodeVariable(ICON_FA_CODE);
	reinterpret_cast<View*>(currentExpressionStringVariable->getComponent("view"))->setVisible(false);

	LOG_DBG("Lexer::eval() - assign the expression string to that variable\n");
	currentExpressionStringVariable->setValue(_expression);

	LOG_DBG("Lexer::eval() - check if users type the exit keyword.\n");
	if ( currentExpressionStringVariable->getValueAsString() == "exit" ){
		LOG_DBG("Lexer::eval() - stopExecution...\n");
		stopExecution();		
	}else{
		LOG_DBG("Lexer::eval() - check if expression is not empty\n");
		if ( currentExpressionStringVariable->isSet())
		{
			/* Create a Lexer node. The lexer will cut expression string into tokens
			(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
			LOG_DBG("Lexer::eval() - create a lexer with the expression string\n");
			auto lexer = container->createNodeLexer(currentExpressionStringVariable);
			return lexer->eval();
			//container->destroyNode(lexer);
		}
	}	

	return false;
}

void Application::shutdown()
{
	LOG_MSG("shutting down application ( _name = \"%s\")\n", getLabel());
}

bool Application::openFile(const char* _filePath)
{		
	auto file = File::CreateFileWithPath(_filePath);

	if (file != nullptr)
	{
		loadedFiles.push_back(file);
		setCurrentlyActiveLoadedFileWithIndex(loadedFiles.size() - 1);
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

void Application::memorizeCurrentlyActiveLoadedFileCursorPosition()
{
	/* First we need to save current cursor position for the current active file in order to restore it if user switch back to his file */
	if (loadedFiles.size() > currentlyActiveLoadedFileIndex)
	{
		auto view = reinterpret_cast<ApplicationView*>(getComponent("view"));
		auto currentFile = loadedFiles.at(currentlyActiveLoadedFileIndex);
		if (currentFile != nullptr) {
			currentFile->setCursorPosition(view->getTextEditorCursorPosition());
		}
	}
}
void Application::setCurrentlyActiveLoadedFileWithIndex(size_t _index)
{
	auto view = reinterpret_cast<ApplicationView*>(getComponent("view"));

	/* Then we set desired file as active */
	if (loadedFiles.size() > _index)
	{
		auto newFile = loadedFiles.at(_index);
		view->setTextEditorContent(newFile->getContent());
		view->setTextEditorCursorPosition(newFile->getCursorPosition());
		currentlyActiveLoadedFileIndex = _index;

	}else{
		view->setTextEditorContent("");
		view->setTextEditorCursorPosition(TextEditor::Coordinates(0, 0));
	}

	/* Clear context (all existing nodes) */
	this->clearContext();

}

void Application::SaveEntity(Entity* _entity)
{
    auto component = new DataAccess;
	_entity->addComponent("dataAccess", component);
	component->update();
	_entity->removeComponent("dataAccess");
}