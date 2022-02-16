#pragma once

#include <nodable/Nodable.h>
#include <nodable/Component.h>
#include <nodable/R.h>

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
