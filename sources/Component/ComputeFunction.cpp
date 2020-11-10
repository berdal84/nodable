#include "ComputeFunction.h"
#include "Log.h"
#include "Function.h"

#include <utility>
#include "Language.h"
#include "Member.h"

using namespace Nodable;

ComputeFunction::ComputeFunction(
        std::shared_ptr<const Function> _function,
        std::shared_ptr<const Language> _language) :
	ComputeBase(std::move(_language)),
	function(std::move(_function))
{
    args.resize( _function->signature->getArgs().size() );
}

bool ComputeFunction::update()
{

	if (function->implementation == NULL) {
		LOG_ERROR(0u, "Unable to find %s's nativeFunction.\n", language->serialize(function->signature).c_str());
		return false;
	}

	if (function->implementation(result, args) )
		LOG_MESSAGE(0u, "Evaluation of %s's native function failed !\n", language->serialize(function->signature).c_str());

	this->updateResultSourceExpression();

	return true;
}

void ComputeFunction::updateResultSourceExpression() const
{
	std::string expr = language->serialize(function->signature, args);
	this->result->setSourceExpression(expr.c_str());
}

void ComputeFunction::setFunction(const std::shared_ptr<const Function>& _function)
{
    this->function = _function;
    this->args.resize(function->signature->getArgs().size());
}

void ComputeFunction::setArg(size_t _index, std::shared_ptr<Member> _value)
{
    args[_index] = std::move(_value);
}

