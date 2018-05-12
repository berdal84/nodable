#include "Nodable.h"
#include "View.h"   // base class
#include <imgui.h>

namespace Nodable{
	class WireView : public View
	{
	public:
		WireView(Wire*);
		~WireView();
		void draw();
	private:
		Wire*   wire  = nullptr;
	};
}