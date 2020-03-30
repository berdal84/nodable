#pragma once

#include "Nodable.h"
#include "Component.h"

namespace Nodable
{
	class DataAccess : public Component
	{
	public:
		bool update()override;
	};
}
