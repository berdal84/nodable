#include "AppView.h"

#include <nfd.h>

#include "core/log.h"
#include "core/system.h"
#include "EventManager.h"
#include "App.h"
#include "TextureManager.h"

using namespace fw;

constexpr const char* k_status_window_name = "Messages";

AppView::AppView(App * _app)
    : m_app(_app)
    , m_is_layout_initialized(false)
{
    LOG_VERBOSE("fw::AppView", "Constructor " OK "\n");
}

AppView::~AppView()
{
    LOG_VERBOSE("fw::AppView", "Destructor " OK "\n");
}

bool AppView::draw()
{
    bool is_main_window_open = true;
    bool redock_all          = false;

    // 2) Draw
    //--------

    ImGui::SetCurrentFont(m_app->font_manager.get_font(FontSlot_Paragraph));

    // Show/Hide ImGui Demo Window
    {
        if (m_app->config.imgui_demo){
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&m_app->config.imgui_demo);
        }
    }

    // Splashscreen
    draw_splashscreen_window();

    // Setup main window

    ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoDocking // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        | ImGuiWindowFlags_NoMove    // because it would be confusing to have two docking targets within each others.
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;


    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // Remove padding

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Draw main window

    ImGui::Begin("App", &is_main_window_open, window_flags);
    {
        ImGui::PopStyleVar(3);

        // Build layout
        if (!m_is_layout_initialized)
        {
            // Dockspace IDs
            m_dockspaces[Dockspace_ROOT]   = ImGui::GetID("Dockspace_ROOT");
            m_dockspaces[Dockspace_CENTER] = ImGui::GetID("Dockspace_CENTER");
            m_dockspaces[Dockspace_RIGHT]  = ImGui::GetID("Dockspace_RIGHT");
            m_dockspaces[Dockspace_BOTTOM] = ImGui::GetID("Dockspace_BOTTOM");
            m_dockspaces[Dockspace_TOP]    = ImGui::GetID("Dockspace_TOP");

            // Split root to have N dockspaces
            ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

            ImGui::DockBuilderRemoveNode(m_dockspaces[Dockspace_ROOT]); // Clear out existing layout
            ImGui::DockBuilderAddNode(m_dockspaces[Dockspace_ROOT]     , ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_dockspaces[Dockspace_ROOT] , viewport_size);

            ImGui::DockBuilderSplitNode(m_dockspaces[Dockspace_ROOT]   , ImGuiDir_Down , 0.5f, &m_dockspaces[Dockspace_BOTTOM], &m_dockspaces[Dockspace_CENTER]);
            ImGui::DockBuilderSetNodeSize(m_dockspaces[Dockspace_BOTTOM] , ImVec2(viewport_size.x, m_app->config.dockspace_bottom_size));

            ImGui::DockBuilderSplitNode(m_dockspaces[Dockspace_CENTER]   , ImGuiDir_Up , 0.5f, &m_dockspaces[Dockspace_TOP], &m_dockspaces[Dockspace_CENTER]);
            ImGui::DockBuilderSetNodeSize(m_dockspaces[Dockspace_TOP] , ImVec2(viewport_size.x, m_app->config.dockspace_top_size));

            ImGui::DockBuilderSplitNode(m_dockspaces[Dockspace_CENTER] , ImGuiDir_Right, m_app->config.dockspace_right_ratio, &m_dockspaces[Dockspace_RIGHT], &m_dockspaces[Dockspace_CENTER]);

            // Configure dockspaces
            ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_CENTER])->HasCloseButton         = false;
            ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_RIGHT])->HasCloseButton          = false;
            ImGuiDockNode *ds_bottom_builder = ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_BOTTOM]);
            ds_bottom_builder->HasCloseButton         = false;

            ds_bottom_builder->SharedFlags            = ImGuiDockNodeFlags_NoDocking;
            ImGuiDockNode *ds_top_builder = ImGui::DockBuilderGetNode(m_dockspaces[Dockspace_TOP]);
            ds_top_builder->HasCloseButton            = false;
            ds_top_builder->WantHiddenTabBarToggle    = true;
            ds_top_builder->WantLockSizeOnce          = true;

            // Dock windows
            dock_window(k_status_window_name, Dockspace_BOTTOM);

            // Run user defined code
            on_reset_layout();

            // Finish the build
            ImGui::DockBuilderFinish(m_dockspaces[Dockspace_ROOT]);

            m_is_layout_initialized = true;
            redock_all              = true;
        }

        // Define root as current dockspace
        ImGui::DockSpace(get_dockspace(Dockspace_ROOT));

        // Draw Windows
        draw_status_window();

        // User defined draw
        on_draw();
    }
    ImGui::End(); // Main window

    return false;
}

bool AppView::pick_file_path(std::string& _out_path, DialogType _dialog_type) const
{
    nfdchar_t *out_path;
    nfdresult_t result;

    switch( _dialog_type )
    {
        case DIALOG_SaveAs:
            result = NFD_SaveDialog(&out_path, nullptr, 0, nullptr, nullptr);
            break;
        case DIALOG_Browse:
            result = NFD_OpenDialog(&out_path, nullptr, 0, nullptr);
            break;
    }

    switch (result)
    {
        case NFD_OKAY:
            _out_path = out_path;
            NFD_FreePath(out_path);
            return true;
        case NFD_CANCEL:
            LOG_MESSAGE("fw::AppView", "User pressed cancel.");
            return false;
        default:
            LOG_ERROR("fw::AppView", "%s\n", NFD_GetError());
            return false;
    }
}

void AppView::set_layout_initialized(bool b)
{
    m_is_layout_initialized = b;
}

ImGuiID AppView::get_dockspace(Dockspace dockspace)const
{
    return m_dockspaces[dockspace];
}

void AppView::dock_window(const char* window_name, Dockspace dockspace)const
{
    ImGui::DockBuilderDockWindow(window_name, m_dockspaces[dockspace]);
}

void AppView::draw_splashscreen_window()
{
    if (m_app->config.splashscreen && !ImGui::IsPopupOpen(m_app->config.splashscreen_window_label))
    {
        ImGui::OpenPopup(m_app->config.splashscreen_window_label);
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(550, 300), ImVec2(550, 50000));
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));

    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::BeginPopupModal(m_app->config.splashscreen_window_label, &m_app->config.splashscreen, flags))
    {
        on_draw_splashscreen();
        ImGui::EndPopup();
    }
}

void AppView::draw_status_window() const
{
    if (ImGui::Begin(k_status_window_name))
    {
        if (!fw::log::get_messages().empty())
        {
            const std::deque<fw::log::Message> &messages = fw::log::get_messages();
            auto it = messages.rend() - std::min(m_app->config.log_tooltip_max_count, messages.size());
            while (it != messages.rend())
            {
                auto &each_message = *it;
                ImGui::TextColored(m_app->config.log_color[each_message.verbosity], "%s", each_message.text.c_str());
                ++it;
            }

            if (!ImGui::IsWindowHovered())
            {
                ImGui::SetScrollHereY();
            }

        }
    }
    ImGui::End();
}
