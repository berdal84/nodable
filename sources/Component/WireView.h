#pragma once

#include "Nodable.h"
#include "View.h" // base class
#include <imgui/imgui.h>
#include <mirror.h>

namespace Nodable{
	class WireView : public View
	{
	public:
		bool update()override {}
		bool draw()override;
		MIRROR_CLASS(WireView)(
			MIRROR_PARENT(View));
	};
}