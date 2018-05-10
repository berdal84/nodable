#include <imgui.h>

namespace Nodable{
	class View
	{
	public:
		static void DrawRectShadow(ImVec2 _from, ImVec2 _to, float _rectInitialRadius = 0.0f, int _radius = 10, ImVec2 _offset = ImVec2());
	};
}
