#pragma once

#include "tools/core/types.h"
#include "tools/core/reflection/reflection"
#include "NodeComponent.h"

namespace ndbl
{
    /**
     * @deprecated Component to handle node read/write from/to disk
     */
	class DataAccess : public NodeComponent
	{
	public:
		DataAccess() {};
		bool update();  // Write the node owner of this component to disk (as JSON)

        REFLECT_DERIVED_CLASS()
	};
}
