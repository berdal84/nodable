#pragma once

#include <imgui/imgui.h>
#include <memory>
#include "Way.h"

namespace Nodable
{
    class Member;

	class Connector
	{
	public:

		Connector(std::weak_ptr<Member> _member, Way _way);
        ~Connector() = default ;

		bool equals(const Connector* _other)const;
        ImVec2 position() const;

		std::weak_ptr<Member> member;
		Way way;
    };
}