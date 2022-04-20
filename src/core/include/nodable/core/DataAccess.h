#pragma once

#include <nodable/core/types.h>
#include <nodable/core/Component.h>
#include <nodable/core/reflection/type.>

namespace Nodable
{
	class DataAccess : public Component
	{
	public:
		DataAccess() {};
		bool update()override;

        REFLECT_DERIVED_CLASS(Component)
	};
}
