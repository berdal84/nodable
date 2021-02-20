#include "Core/File.h"
#include "Core/Log.h"
#include "Component/History.h"
#include "Component/FileView.h"
#include "Component/ContainerView.h"
#include "Component/Container.h"
#include "Component/View.h"
#include "Component/NodeView.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Language/Common/Parser.h"
#include "Language/Common/LanguageLibrary.h"
#include "IconFontCppHeaders/IconsFontAwesome5.h"

#include <fstream>


using namespace Nodable;

Nodable::File::File( std::filesystem::path _path, const char* _content):
	path(_path),
	language(LanguageLibrary::GetNodable()) /* Detect the language (TODO) */
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
	container->setLabel(_path.filename().string() + "'s inner container");
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

			auto label = language->getSerializer()->serialize((*it).signature);
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
    LOG_MESSAGE( "File", "Loading file \"%s\"...\n", _filePath.c_str());

	std::ifstream fileStream(_filePath);

	if (!fileStream.is_open())
	{
		LOG_ERROR("File", "Unable to load \"%s\"\n", _filePath.c_str());
		return nullptr;
	}

	// TODO: do that inside File constr ?
	std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	File* file = new File(_filePath.c_str(), content.c_str());

	return file;
}

bool File::evaluateExpression(std::string& _expression)
{
	Parser* parser = language->getParser();
	Container* container = getInnerContainer();

    if ( parser->evalCodeIntoContainer(_expression, container) && container->hasInstructions() )
    {
        NodeView::ArrangeRecursively(container->getScope()->getLastCodeBlock()->as<CodeBlockNode>());
        LOG_MESSAGE("File", "Expression evaluated: %s\n", _expression.c_str());
        return true;
    }
    return false;
}

UpdateResult File::update() {

	if (auto history = this->getComponent<History>())
	{
		if (history->dirty)
		{
			this->evaluateSelectedExpression();
			history->dirty = false;
		}
	}

	auto containerUpdateResult = getInnerContainer()->update();
	auto view = getComponent<FileView>();

	if (containerUpdateResult == UpdateResult::SuccessWithoutChanges && !view->getSelectedText().empty() )
    {
        return UpdateResult::SuccessWithoutChanges;
    }

	auto scope = getInnerContainer()->getScope();

	if ( !scope->innerBlocs.empty() )
    {
        std::string code = language->getSerializer()->serialize( scope );
        view->replaceSelectedText(code);
    }
	
	return UpdateResult::Success;
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
