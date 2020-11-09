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
#include <utility>

using namespace Nodable;

Nodable::File::File( std::filesystem::path _path, const char* _content):
    Node(),
	path( std::move(_path) )
{
    this->language = Language::Nodable(); // TODO: detect language from file

	/* Creates the FileView	*/
	auto fileView = newComponent<FileView>().lock();
	fileView->init();
	fileView->setText(_content);
	auto textEditor = fileView->getTextEditor();

	/* Creates an history for UNDO/REDO	*/
	auto history = newComponent<History>().lock();
    auto undoBuffer = history->createTextEditorUndoBuffer(textEditor);
	fileView->setUndoBuffer(undoBuffer);
	
	/* Creates a node container */
	innerContainer = std::make_shared<Container>(language);
	auto containerView = innerContainer->newComponent<ContainerView>().lock();

	/* Add inputs in contextual menu */
	auto api = language->getAllFunctions();

	for (const auto& function : api)
	{
	    auto op = language->findOperator(function->signature);

		if ( op != nullptr )
		{
			auto lambda = [this, op]()->Node*
			{	
				return innerContainer->newBinOp(op);
			};

			auto label = op->signature.getLabel();
			containerView->addContextualMenuItem( "Operators", label, lambda);
		}
		else
		{
			auto lambda = [this, function]()->Node*
			{
				return innerContainer->newFunction(function);
			};

			auto label = language->serialize(function->signature);
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

std::unique_ptr<File> File::OpenFile(std::filesystem::path _filePath)
{

	std::ifstream fileStream(_filePath);

	if (!fileStream.is_open())
	{
		LOG_ERROR(0u, "Unable to load \"%s\"\n", _filePath.c_str());
		return nullptr;
	}

	// TODO: do that inside File constr ?
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	auto file = std::make_unique<File>( _filePath.c_str(), content.c_str() );

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
