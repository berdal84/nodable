#pragma once

#include <imgui/imgui.h>
#include "Way.h"

namespace Nodable
{
    class Member;

	class Connector
	{
	public:

		Connector(Member* _member = nullptr, Way _way = Way::Default);
        ~Connector() = default ;

		bool equals(const Connector* _other)const;
        ImVec2 position() const;

		Member* member;
		Way way;
    };
}