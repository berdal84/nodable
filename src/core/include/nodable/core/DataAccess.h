#pragma once

#include <nodable/core/types.h>
#include <nodable/core/Component.h>
#include <nodable/core/reflection/R.h>

namespace Nodable
{
	class DataAccess : public Component
	{
	public:
		DataAccess() {};
		bool update()override;
		R_DERIVED(DataAccess)
    R_EXTENDS(Component)
    R_END
	};
}
