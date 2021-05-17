#include <nodable/Settings.h>
#include <imgui/imgui.h>

using namespace Nodable::app;

Settings* Settings::GetCurrent()
{
    static Settings* g_conf = nullptr;

    if ( g_conf == nullptr)
    {
        g_conf = new Settings();

        // TODO: create themes

        // main layout
        g_conf->ui.layout.propertiesRatio       = 0.25f;

        // text
        g_conf->ui.text.p.size                  = 18.0f;
        g_conf->ui.text.p.font                  = "CenturyGothic.ttf";
        g_conf->ui.text.h1.size                 = 25.0f;
        g_conf->ui.text.h1.font                 = "CenturyGothic.ttf";
        g_conf->ui.text.textEditorPalette       = {
            0xffffffff, // None
            0xffd69c56, // Keyword
            0xff00ff00, // Number
            0xff7070e0, // String
            0xff70a0e0, // Char literal
            0xffffffff, // Punctuation
            0xff409090, // Preprocessor
            0xffdddddd, // Identifier
            0xff9bc64d, // Known identifier
            0xffc040a0, // Preproc identifier
            0xff909090, // Comment (single line)
            0xff909090, // Comment (multi line)
            0x30000000, // Background
            0xffe0e0e0, // Cursor
            0x20ffffff, // Selection
            0x800020ff, // ErrorMarker
            0x40f08000, // Breakpoint
            0x88909090, // Line number
            0x40000000, // Current line fill
            0x40808080, // Current line fill (inactive)
            0x40a0a0a0, // Current line edge
        };

        // nodes
        g_conf->ui.node.padding                = 6.0f;
        g_conf->ui.node.memberConnectorRadius        = 5.0f;
        g_conf->ui.node.functionColor          = ImVec4(0.7f, 0.7f, 0.9f, 1.0f); // blue
        g_conf->ui.node.variableColor          = ImVec4(0.9f, 0.9f, 0.7f, 1.0f); // purple
        g_conf->ui.node.instructionColor       = ImVec4(0.7f, 0.9f, 0.7f, 1.0f); // green
        g_conf->ui.node.literalColor           = ImVec4(0.75f, 0.75f, 0.75f, 1.0f); // light grey
        g_conf->ui.node.fillColor              = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        g_conf->ui.node.highlightedColor       = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        g_conf->ui.node.borderColor            = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
        g_conf->ui.node.borderHighlightedColor = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);
        g_conf->ui.node.shadowColor            = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
        g_conf->ui.node.nodeConnectorHoveredColor = ImColor(127,200, 127);
        g_conf->ui.node.nodeConnectorColor     = ImColor(127,127, 127);
        g_conf->ui.node.spacing                = 15.0f;
        g_conf->ui.node.speed                  = 30.0f;
        g_conf->ui.node.nodeConnectorHeight    = 10.0f;
        g_conf->ui.node.nodeConnectorPadding   = 4.0f;

        // wires
        g_conf->ui.wire.bezier.roundness        = 0.5f;
        g_conf->ui.wire.bezier.thickness        = 2.0f;
        g_conf->ui.wire.displayArrows           = false;
        g_conf->ui.wire.fillColor               = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        g_conf->ui.wire.shadowColor             = g_conf->ui.node.shadowColor;

        // code flow
        g_conf->ui.codeFlow.lineWidthMax        = 40.0f;
        g_conf->ui.codeFlow.lineColor           = ImColor(200,255,200,50);
        g_conf->ui.codeFlow.lineShadowColor     = ImColor(0,0,0,64);

        // buttons
        g_conf->ui.button.color                 = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
        g_conf->ui.button.hoveredColor          = ImVec4(0.70f, 0.70f, 0.70f, 0.95f);
        g_conf->ui.button.activeColor           = ImVec4(0.98f, 0.73f, 0.29f, 0.95f);

    }
    return g_conf;
}

void Settings::setImGuiStyle(ImGuiStyle& _style)
{
    ImVec4* colors = _style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.64f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.90f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.90f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.49f, 0.63f, 0.69f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.60f, 0.60f, 0.60f, 0.98f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.61f, 0.61f, 0.62f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.31f, 0.23f, 0.14f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.71f, 0.46f, 0.22f, 0.63f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.71f, 0.46f, 0.22f, 1.00f);
    colors[ImGuiCol_Button]                 = ui.button.color;
    colors[ImGuiCol_ButtonHovered]          = ui.button.hoveredColor;
    colors[ImGuiCol_ButtonActive]           = ui.button.activeColor;
    colors[ImGuiCol_Header]                 = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.89f, 0.65f, 0.11f, 0.96f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.71f, 0.71f, 0.71f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(1.00f, 0.62f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.58f, 0.54f, 0.50f, 0.86f);
    colors[ImGuiCol_TabHovered]             = ImVec4(1.00f, 0.79f, 0.45f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(1.00f, 0.73f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.53f, 0.53f, 0.53f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(1.00f, 0.70f, 0.09f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.55f);

    _style.WindowBorderSize   = 1.0f;
    _style.FrameBorderSize    = 1.0f;
    _style.FrameRounding      = 3.0f;
    _style.ChildRounding      = 3.0f;
    _style.WindowRounding     = 0.0f;
    _style.AntiAliasedFill    = true;
    _style.AntiAliasedLines   = true;
    _style.WindowPadding      = ImVec2(10.0f,10.0f);
};