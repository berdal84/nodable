#pragma once

#include "Nodable.h"
#include "Component.h"

namespace Nodable
{
	class DataAccess : public Component
	{
	public:
		COMPONENT_CONSTRUCTOR(DataAccess);
		void update()override;
	};
}
