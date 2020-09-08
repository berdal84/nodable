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

#include <Windows.h>
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
	addComponent("view", fileView);
	fileView->init();
	fileView->setText(_content);
	auto textEditor = fileView->getTextEditor();

	/* Creates an history for UNDO/REDO	*/
	auto history = new History();
	addComponent("history", history);
    auto undoBuffer = history->createTextEditorUndoBuffer(textEditor);
	fileView->setUndoBuffer(undoBuffer);
	
	/* Creates a node container */
	auto container = new Container(language);
	addComponent("container", container);
	auto containerView = new ContainerView();
	container->addComponent("view", containerView);
	container->setOwner(this);

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

std::string File::BrowseForFileAndReturnItsAbsolutePath(SDL_Window* currentWindow)
{
	OPENFILENAME ofn;                             // common dialog box structure
	char szFile[512];                             // buffer for file name            

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(currentWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;           // owner sdlWindow

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn))
		return szFile;

	return "";
}

bool File::evaluateExpression(std::string& _expression)
{
	boolean success;

	auto container = getContainer();

	/* Create a Parser node. The Parser will cut expression string into tokens
	(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
	Parser parser(language, container);
	
	if (success = parser.eval(_expression))
	{
		auto result = container->getResultVariable();
		auto view   = result->getComponent<NodeView>();
		NodeView::ArrangeRecursively(view);
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

	auto hasChanged = getContainer()->update();
	
	if (!hasChanged)
		return false;

	auto result		= getContainer()->getResultVariable();

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

	getContainer()->clear();

	auto view = getComponent<FileView>();

	auto expression = view->getSelectedText();
	success = evaluateExpression(expression);

	return success;
}