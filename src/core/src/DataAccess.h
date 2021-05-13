#pragma once

#include "Nodable.h"
#include "Component.h"
#include <mirror.h>

namespace Nodable
{
	class DataAccess : public Component
	{
	public:
		DataAccess() {};
		bool update()override;
		MIRROR_CLASS(DataAccess)(
			MIRROR_PARENT(Component));
	};
}
