#pragma once

// All the Views uses ImGui
#include <imgui/imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
// I also import internal for maths operators
#include <imgui/imgui_internal.h>

#include <Component/Component.h>
#include <mirror.h>

namespace Nodable
{
    /**
     * View is an abstract class to provide a GUI for a specific Node.
     * View also implement a small static library to draw custom ImGui widgets/graphics.
     *
     * TODO: split "interface" and "ImGui implementation"
     */
	class View : public Component
	{
	public:

	    /**
	     * Enum to define some color types
	     */
		enum ColorType_
		{
			ColorType_Fill,
			ColorType_Highlighted,
			ColorType_Border,
			ColorType_BorderHighlights,
			ColorType_Shadow,
			ColorType_COUNT
		};

		View();

		~View() override = default;

		/**
		 * Method draw to implement in derived.
		 * @return true if View has been modified.
		 */
		virtual bool draw() = 0;

		/**
		 * Draw this View wrapped into an ImGui::BeginChild() / ImGui::EndChild()
		 * @param _name
		 * @param _size
		 * @param border
		 * @param flags
		 * @return true if View has been modified.
		 */
		bool drawAsChild(const char* _name, const ImVec2& _size, bool border = false, ImGuiWindowFlags flags = 0);

		/**
		 * Set a color for a given color type.
		 * @param _type
		 * @param _color an ImColor (see ImGui)
		 */
		void setColor(ColorType_ _type, ImVec4* _color );

		/**
		 * Get the color of a given color type.
		 * @return an ImColor (see ImGui)
		 */
		ImColor getColor(ColorType_);


		/**
		 * Change the visibility of this View
		 * @param _visibility
		 */
		inline void setVisible(bool _visibility)
		{
		    visible = _visibility;
		}

        /**
         * Get the visibility of this View
         * @return
         */
		[[nodiscard]] inline bool isVisible()const
		{
		    return visible;
		}

		/**
		 * Return true if this View is hovered by mouse cursor
		 * @return
		 */
		[[nodiscard]] inline bool isHovered()const
		{
		    return hovered;
		}

		/**
		 * Set the visible rectangle (in local space)
		 * @param _rect
		 */
		inline void setVisibleRect(ImRect _rect)
        {
		    visibleRect = _rect;
        }

        /**
         * Get the visible rectangle (in local space)
         */
        inline ImRect getVisibleRect() const
        {
            return visibleRect;
        }

    protected:
        /**
        * The visible rectangle (in local space).
        */
        ImRect  visibleRect;

        /**
         * Visible rectangle (in screen space)
         */
		ImRect  visibleScreenRect;

		/**
		 * Mouse hovered status
		 */
		bool    hovered;

    private:
	    /**
	     * Visibility, View is drawn only if visible == true.
	     */
		bool    visible;

		/**
		 * Color table
		 */
        std::map<ColorType_, ImVec4*> colors;

	public:

	    /**
	     * Draw a rounded-rectangle shadow
	     * TODO: use a low cost method, this one is drawing several rectangle with modulated opacity.
	     *
	     * @param _topLeftCorner
	     * @param _bottomRightCorner
	     * @param _borderRadius
	     * @param _shadowRadius
	     * @param _shadowOffset
	     * @param _shadowColor
	     */
		static void DrawRectShadow(
			ImVec2 	_topLeftCorner,
			ImVec2 	_bottomRightCorner,
			float 	_borderRadius = 0.0f,
			int 	_shadowRadius = 10,
			ImVec2 	_shadowOffset = ImVec2(),
			ImColor _shadowColor = ImColor(0.0f, 0.0f, 0.0f));

		/**
		 * Draw a shadowed text with control over the shadow color.
		 * This calls ImGui::TextColored & ImGui::Text to fake a colored shadow.
		 * @param _offset in pixels
		 * @param _shadowColor
		 * @param _format string format
		 * @param ... format arguments
		 */
		static void ShadowedText(
		        ImVec2 _offset,
		        ImColor _shadowColor,
		        const char* _format,
		        ...);

		/**
		 * Draw a shadowed text with control over the text and shadow color.
		 * @param _offset in pixels
		 * @param _textColor
		 * @param _shadowColor
		 * @param _format string format
		 * @param ... format arguments
		 */
		static void ColoredShadowedText(
		        ImVec2 _offset,
		        ImColor _textColor,
		        ImColor _shadowColor,
		        const char* _format,
		        ...);

		/**
		 * Helper to convert ImGui cursor position to ImGui screen position
		 * @param _cursorPosition
		 * @return the screen position
		 */
		static ImVec2 CursorPosToScreenPos(ImVec2 _cursorPosition);

		MIRROR_CLASS(View)(
			MIRROR_PARENT(Component));
	};
}
