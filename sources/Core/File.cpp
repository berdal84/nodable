#include "File.h"
#include "History.h"
#include "FileView.h"
#include "ContainerView.h"
#include "Container.h"
#include "Parser.h"
#include "View.h"
#include "Variable.h"
#include "Log.h"
#include "NodeView.h"

#include "IconFontCppHeaders/IconsFontAwesome5.h"

#include <SDL.h>
#include <SDL2\include\SDL_syswm.h>
#include <fstream>

using namespace Nodable;

Nodable::File::File(
	const char* _path,
	const char* _content,
	const char* _name)
{	
	path = _path;
	name = _name;

	/* Detect the language (TODO) */
	language = Language::Nodable();

	/* Creates the FileView	*/
	auto fileView = new FileView();
	addComponent(fileView);
	fileView->init();
	fileView->setText(_content);
	auto textEditor = fileView->getTextEditor();

	/* Creates an history for UNDO/REDO	*/
	auto history = new History();
	addComponent(history);
    auto undoBuffer = history->createTextEditorUndoBuffer(textEditor);
	fileView->setUndoBuffer(undoBuffer);
	
	/* Creates a node container */
	auto container = new Container(language);
	setInnerContainer(container);
	auto containerView = new ContainerView();
	container->addComponent(containerView);

	/* Add inputs in contextual menu */
	auto api = language->getAPI();
	for (auto it = api.begin(); it != api.end(); it++) {
		auto function = &*it;
		auto lambda = [container, function]()->Node* {
			return container->newFunction(function);
		};
		containerView->addContextualMenuItem( ICON_FA_CODE " " + language->serialize((*it).signature), lambda);
	}

}

void File::save()
{
	if (modified) {
		std::ofstream fileStream(this->path.c_str());
		auto view    = getComponent<FileView>();
		auto content = view->getText();
fileStream.write(content.c_str(), content.size());
modified = false;
	}

}

File* File::CreateFileWithPath(const char* _filePath)
{
	/*
		Creates the File
	*/

	// Sanitize path to get only slashes, and no antislashes
	std::string cleanedFilePath(_filePath);
	std::replace(cleanedFilePath.begin(), cleanedFilePath.end(), '/', '\\');

	// Extract file name from filePath
	std::string name = cleanedFilePath;
	auto firstSlashPosition = cleanedFilePath.find_last_of('\\');
	if (firstSlashPosition != std::string::npos)
		name = cleanedFilePath.substr(firstSlashPosition + 1, cleanedFilePath.size() - firstSlashPosition - 1);

	std::ifstream fileStream(cleanedFilePath.c_str());

	if (!fileStream.is_open())
	{
		LOG_ERROR("Unable to load \"%s\"\n", cleanedFilePath.c_str());
		return nullptr;
	}

	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	File* file = new File(cleanedFilePath.c_str(), content.c_str(), name.c_str());


	return file;
}

bool File::evaluateExpression(std::string& _expression)
{

	auto container = getInnerContainer();

	/* Create a Parser node. The Parser will cut expression string into tokens
	(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
	Parser parser(language, container);
	
	if (parser.eval(_expression))
	{
		auto result = container->getResultVariable();
		auto view   = result->getComponent<NodeView>();
		NodeView::ArrangeRecursively(view);
		return true;
	}
	return false;
}

bool File::update() {

	if (auto history = this->getComponent<History>()) {
		if (history->dirty) {
			this->evaluateSelectedExpression();
			history->dirty = false;
		}
	}

	auto hasChanged = getInnerContainer()->update();
	
	if (!hasChanged)
		return false;

	auto result		= getInnerContainer()->getResultVariable();

	if (!result) {
		return false;
	}

	auto member		= result->getMember();
	auto expression = member->getSourceExpression();
	auto view		= getComponent<FileView>();

	view->replaceSelectedText(expression);
	
	return true;
}

bool File::evaluateSelectedExpression()
{
	bool success;

	getInnerContainer()->clear();

	auto view = getComponent<FileView>();

	auto expression = view->getSelectedText();
	success = evaluateExpression(expression);

	return success;
}