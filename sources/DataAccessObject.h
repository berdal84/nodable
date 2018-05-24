#pragma once

#include "Nodable.h"
#include "Component.h"

namespace Nodable
{
	class DataAccessObject : public Component
	{
	public:
		DECLARE_COMPONENT(DataAccessObject);
		void update()override;
	};
}
