#pragma once

#include "Nodable.h"
#include "View.h" // base class
#include <imgui/imgui.h>
#include <mirror.h>

#include <utility>

namespace Nodable
{
    class Wire;

	class WireView: public View
	{
	public:
	    explicit WireView(std::weak_ptr<Wire> _wire): m_wire(std::move(_wire)) {}
	    ~WireView() = default;
		bool draw();

    private:
	    std::weak_ptr<Wire> m_wire;
	};
}