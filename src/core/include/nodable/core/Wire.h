#pragma once 

#include <nodable/core/types.h>
#include <nodable/core/Node.h>
#include <nodable/core/Edge.h>

namespace ndbl
{
    /**
     * Directed edge between two Members
     */
	class Wire : public DirectedEdge
	{
	public:
	    Wire(Member* _src, Member* _dst)
            : DirectedEdge(_src->get_owner(), EdgeType::IS_INPUT_OF, _dst->get_owner())
            , members(_src, _dst){}
	    ~Wire() = default;
        const Pair<Member*> members;
	private:
        void  sanitize() override;
	};
}