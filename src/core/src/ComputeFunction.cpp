#include <nodable/ComputeFunction.h>
#include <nodable/Log.h>
#include <nodable/Language.h>

using namespace Nodable;

REFLECT_DEFINE(ComputeFunction)

ComputeFunction::ComputeFunction(const Invokable* _function)
    : ComputeBase()
    , m_function(_function)
{
    NODABLE_ASSERT(_function != nullptr); // must be defined !
    m_args.resize(_function->getSignature()->getArgCount(), nullptr );
}

bool ComputeFunction::update()
{
	m_function->invoke(m_result, m_args);
	return true;
}
