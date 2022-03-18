#pragma once 

#include <nodable/core/reflection/R.h>

#include <nodable/core/types.h>
#include <nodable/core/Node.h>

namespace Nodable
{
	class Wire
	{
	public:

	    Wire(){}
	    ~Wire(){}

		void        set_source(Member*);
		void        set_dest(Member*);

		Member*     get_source()const{return source;}
		Member*     get_dest()const{return target;}

	private:
		Member*     source       = nullptr;
		Member*     target       = nullptr;
	};
}