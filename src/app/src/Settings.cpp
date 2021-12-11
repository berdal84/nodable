#include <nodable/Settings.h>
#include <imgui/imgui.h>
#include <string>
#include <fstream>
#include <nodable/Log.h>
#include <nodable/App.h>

using namespace Nodable;

Settings* Settings::CreateInstance()
{
    Settings* conf = new Settings();

    // TODO: create themes

    // main layout
    conf->ui_layout_propertiesRatio = 0.25f;

    // splashscreen
    conf->ui_splashscreen_imagePath = "images/nodable-logo-xs.png";

    // text
    conf->ui_text_fonts = {
            {
                    "Medium 18px",
                    18.0f,
                    "fonts/JetBrainsMono-Medium.ttf",
                    true
            },
            {
                    "Bold 25px",
                    25.0f,
                    "fonts/JetBrainsMono-Bold.ttf",
                    true
            },
            {
                    "Regular 18px",
                    18.0f,
                    "fonts/JetBrainsMono-Regular.ttf",
                    true
            }
    };

    conf->ui_text_defaultFontsId[FontSlot_Paragraph] = "Medium 18px";
    conf->ui_text_defaultFontsId[FontSlot_Heading]   = "Bold 25px";
    conf->ui_text_defaultFontsId[FontSlot_Code]      = "Regular 18px";

    conf->ui_icons = {
            "Icons",
            18.0f,
            "fonts/fa-solid-900.ttf"
    };


    conf->ui_text_textEditorPalette       = {
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
    conf->ui_node_padding                = 6.0f;
    conf->ui_node_memberConnectorRadius  = 5.0f;
    conf->ui_node_invokableColor          = ImVec4(0.7f, 0.7f, 0.9f, 1.0f); // blue
    conf->ui_node_variableColor          = ImVec4(0.9f, 0.9f, 0.7f, 1.0f); // purple
    conf->ui_node_instructionColor       = ImVec4(0.7f, 0.9f, 0.7f, 1.0f); // green
    conf->ui_node_literalColor           = ImVec4(0.75f, 0.75f, 0.75f, 1.0f); // light grey
    conf->ui_node_fillColor              = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    conf->ui_node_highlightedColor       = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    conf->ui_node_borderColor            = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    conf->ui_node_borderHighlightedColor = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);
    conf->ui_node_shadowColor            = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
    conf->ui_node_nodeConnectorHoveredColor = ImColor(200,200, 200);
    conf->ui_node_nodeConnectorColor     = ImColor(127,127, 127);
    conf->ui_node_spacing                = 30.0f;
    conf->ui_node_speed                  = 30.0f;
    conf->ui_node_connector_height       = 20.0f;
    conf->ui_node_connector_padding      = 2.0f;
    conf->ui_node_connector_width        = conf->ui_node_connector_height;

    // wires
    conf->ui_wire_bezier_roundness        = 0.5f;
    conf->ui_wire_bezier_thickness        = 2.0f;
    conf->ui_wire_displayArrows           = false;
    conf->ui_wire_fillColor               = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    conf->ui_wire_shadowColor             = conf->ui_node_shadowColor;

    // code flow
    conf->ui_codeFlow_lineColor           = ImColor(200,255,200,50);
    conf->ui_codeFlow_lineShadowColor     = ImColor(0,0,0,64);

    // buttons
    conf->ui_button_color                 = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
    conf->ui_button_hoveredColor          = ImVec4(0.70f, 0.70f, 0.70f, 0.95f);
    conf->ui_button_activeColor           = ImVec4(0.98f, 0.73f, 0.29f, 0.95f);

    conf->ui_toolButton_size              = ImVec2(24.0f, 30.0f);

    return conf;
}

Settings* Settings::Get()
{
    static Settings* g_conf = Settings::CreateInstance();
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
    colors[ImGuiCol_Button]                 = ui_button_color;
    colors[ImGuiCol_ButtonHovered]          = ui_button_hoveredColor;
    colors[ImGuiCol_ButtonActive]           = ui_button_activeColor;
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
}

void Settings::Save()
{
//    // Output this default settings to default.cfg file
//    std::filesystem::path path( App::GetAssetPath("settings/") );
//    std::filesystem::create_directory( path );
//    path.append("default.cfg");
//    Settings::Save(path);
}

void Settings::Save(std::string& _path)
{
//    mirror::SimpleKeyValueSerializer serializer;
//    std::string                      out;
//
//    serializer.serialize(Settings::Get(), out);
//
//    std::ofstream outfile ( _path ,std::ofstream::binary);
//    outfile.write (out.c_str(), out.size());
//    outfile.close();
//
//    printf("SimpleKeyValueSerializer out:\n%s\n", out.c_str());
}

Settings *Settings::Load(const char * _path) {
    return nullptr;
};