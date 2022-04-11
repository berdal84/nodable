#pragma once

#include <nodable/core/types.h>
#include <nodable/core/Component.h>
#include <nodable/core/reflection/reflection>

namespace Nodable
{
	class DataAccess : public Component
	{
	public:
		DataAccess() {};
		bool update()override;

		R_CLASS_DERIVED(DataAccess)
        R_CLASS_EXTENDS(Component)
        R_CLASS_END
	};
}
