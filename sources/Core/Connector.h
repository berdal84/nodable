#pragma once
#include "Member.h"
#include "Way.h"

namespace Nodable
{
	class Connector
	{
	public:

		Connector(Member* _member = nullptr,
			Way _way = Way::Default) :
			member(_member),
			way(_way) {
		};


		bool equals(const Connector* _other)const {
			return this->member == _other->member &&
				this->way == _other->way;
		};

		~Connector() {};

		Member* member;
		Way way;

	};
}