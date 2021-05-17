#pragma once

#include <nodable/Nodable.h>
#include <nodable/Component.h>
#include <mirror.h>

namespace Nodable::core
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
