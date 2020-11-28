#include "ComputeFunction.h"
#include "Log.h"
#include "Language/Common/Language.h"

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
		LOG_ERROR(Log::Verbosity::Normal, "Unable to find %s's nativeFunction.\n", language->getSerializer()->serialize(function->signature).c_str());
		return false;
	}

	if (function->implementation(result, args))
		LOG_MESSAGE(Log::Verbosity::Normal, "Evaluation of %s's native function failed !\n", language->getSerializer()->serialize(function->signature).c_str());

	return true;
}