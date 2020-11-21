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

#include <fstream>

using namespace Nodable;

Nodable::File::File( std::filesystem::path _path, const char* _content):
	path(_path),
	language(Language::Nodable()) /* Detect the language (TODO) */
{		

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
	auto api = language->getAllFunctions();

	for (auto it = api.cbegin(); it != api.cend(); it++) {
		const auto function = new Function(*it);

		auto op = language->findOperator(function->signature);


		if (op != nullptr )
		{
			auto lambda = [container, function, op]()->Node*
			{
                return container->newOperator(op);
			};

			auto label = op->signature.getLabel();
			containerView->addContextualMenuItem( "Operators", label, lambda);
		}
		else
		{
			auto lambda = [container, function, op]()->Node*
			{
				return container->newFunction(function);
			};

			auto label = language->serialize((*it).signature);
			containerView->addContextualMenuItem( "Functions", label, lambda);
		}
		
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

File* File::OpenFile(std::filesystem::path _filePath)
{
    LOG_MESSAGE(0u, "Loading file \"%s\"...\n", _filePath.c_str());

	std::ifstream fileStream(_filePath);

	if (!fileStream.is_open())
	{
		LOG_ERROR(0u, "Unable to load \"%s\"\n", _filePath.c_str());
		return nullptr;
	}

	// TODO: do that inside File constr ?
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	File* file = new File(_filePath.c_str(), content.c_str());

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
		LOG_MESSAGE(0u, "Expression evaluated: %s\n", _expression.c_str());
		return true;
	}
	return false;
}

UpdateResult File::update() {

	if (auto history = this->getComponent<History>()) {
		if (history->dirty) {
			this->evaluateSelectedExpression();
			history->dirty = false;
		}
	}

	auto containerUpdateResult = getInnerContainer()->update();
	auto view = getComponent<FileView>();

	if (containerUpdateResult == UpdateResult::SuccessWithoutChanges && view->getSelectedText() != "")
		return UpdateResult::SuccessWithoutChanges;

	auto result		= getInnerContainer()->getResultVariable();

	if (!result) {
		return UpdateResult::SuccessWithoutChanges;
	}

	auto member		= result->getMember();
	auto expression = member->getSourceExpression();
	

	view->replaceSelectedText(expression);
	
	return UpdateResult::SuccessWithChanges;
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

bool& File::isOpen() {
    return open;
}
