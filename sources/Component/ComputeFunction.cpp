#include "ComputeFunction.h"
#include "Log.h"
#include "Language.h"

using namespace Nodable;

ComputeFunction::ComputeFunction(const Function* _function, const Language* _language) :
	ComputeBase(_language),
	function(_function)
{
	size_t i = 0;
	while (args.size() < _function->signature.getArgs().size())
		args.push_back(nullptr);
}

bool ComputeFunction::update()
{

	if (function->implementation == NULL) {
		LOG_ERROR(0u, "Unable to find %s's nativeFunction.\n", language->serialize(function->signature).c_str());
		return false;
	}

	if (function->implementation(result, args))
		LOG_MESSAGE(0u, "Evaluation of %s's native function failed !\n", language->serialize(function->signature).c_str());

	this->updateResultSourceExpression();

	return true;
}

void ComputeFunction::updateResultSourceExpression() const
{
	std::string expr = language->serialize(function->signature, args);
	this->result->setSourceExpression(expr.c_str());
}

