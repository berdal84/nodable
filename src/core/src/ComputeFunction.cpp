#include <nodable/ComputeFunction.h>
#include <nodable/Log.h>
#include <nodable/Language.h>

using namespace Nodable;

ComputeFunction::ComputeFunction(const Function* _function)
    : ComputeBase()
    , m_function(_function)
{
    NODABLE_ASSERT(_function != nullptr); // must be defined !
    m_args.resize(_function->signature.getArgs().size(), nullptr );
}

bool ComputeFunction::update()
{

	if (!m_function->implementation)
	{
		LOG_ERROR("ComputeFunction", "Unable to find %s's implementation.\n", m_function->signature.getIdentifier().c_str());
		return false;
	}

	if (m_function->implementation(m_result, m_args))
    {
        LOG_ERROR("ComputeFunction", "Unable to evaluate %s's implementation.\n", m_function->signature.getIdentifier().c_str());
        return false;
    }

	return true;
}
