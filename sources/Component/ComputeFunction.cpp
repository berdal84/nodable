#include "ComputeFunction.h"
#include "Log.h"
#include "Language/Common/Language.h"

using namespace Nodable;

ComputeFunction::ComputeFunction(const Function* _function, const Language* _language) :
	ComputeBase(_language),
	function(_function)
{
    NODABLE_ASSERT(_function != nullptr); // must be defined !
	while (args.size() < _function->signature.getArgs().size())
		args.push_back(nullptr);
}

bool ComputeFunction::update()
{

	if (function->implementation == NULL) {
		LOG_ERROR( "ComputeFunction", "Unable to find %s's nativeFunction.\n", language->getSerializer()->serialize(function->signature).c_str());
		return false;
	}

	if (function->implementation(result, args))
		LOG_MESSAGE( "ComputeFunction", "Evaluation of %s's native function failed !\n", language->getSerializer()->serialize(function->signature).c_str());

	return true;
}
