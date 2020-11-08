#pragma once

#include "Nodable.h"
#include "View.h" // base class
#include <imgui/imgui.h>
#include <mirror.h>

namespace Nodable
{
    class Wire;

	class WireView: public View
	{
	public:
	    explicit WireView(Wire* _wire): wire(_wire) {}
	    ~WireView() = default;
		bool draw();

    private:
	    Wire* wire;
	};
}