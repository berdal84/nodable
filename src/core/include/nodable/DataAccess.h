#pragma once

#include <nodable/Nodable.h>
#include <nodable/Component.h>
#include <nodable/Reflect.h>

namespace Nodable
{
	class DataAccess : public Component
	{
	public:
		DataAccess() {};
		bool update()override;
		REFLECT_DERIVED(DataAccess)
    REFLECT_EXTENDS(Component)
    REFLECT_END
	};
}
