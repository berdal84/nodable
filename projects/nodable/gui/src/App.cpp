#include <ndbl/gui/App.h>

#include <algorithm>
#include <fw/core/system.h>
#include <fw/core/assertions.h>
#include <fw/gui/EventManager.h>

#include <ndbl/gui/AppView.h>
#include <ndbl/gui/Condition.h>
#include <ndbl/gui/Event.h>
#include <ndbl/gui/File.h>
#include <ndbl/gui/FileView.h>
#include <ndbl/gui/GraphNodeView.h>
#include <ndbl/gui/NodeConnector.h>
#include <ndbl/gui/NodeView.h>
#include <ndbl/gui/PropertyConnector.h>
#include <ndbl/gui/commands/Cmd_ConnectEdge.h>
#include <ndbl/gui/commands/Cmd_DisconnectEdge.h>
#include <ndbl/gui/commands/Cmd_Group.h>
#include <ndbl/core/DataAccess.h>
#include <ndbl/core/VariableNode.h>

using namespace ndbl;

App* App::s_instance = nullptr;

App::App()
    : fw::App(new AppView(this, Settings::get_instance().fw_app_view))
    , m_current_file_index(0)
    , m_current_file(nullptr)
{
    FW_EXPECT(s_instance == nullptr, "Can't create two concurrent App. Delete first instance.");
    s_instance = this;
}

App::~App()
{
    delete m_view;
    s_instance = nullptr;
}

bool App::onInit()
{
    using fw::EventType; // TODO: split framework and nodable event types.
    
    // Bind commands to shortcuts
    m_event_manager.bind(
            {"Delete",
             EventType_delete_node_action_triggered,
             {SDLK_DELETE, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {"Arrange",
             EventType_arrange_node_action_triggered,
             {SDLK_a, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {"Fold/Unfold",
             EventType_toggle_folding_selected_node_action_triggered,
             {SDLK_x, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {"Next",
             EventType_select_successor_node_action_triggered,
             {SDLK_n, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {ICON_FA_SAVE " Save",
             fw::EventType_save_file_triggered,
             {SDLK_s, KMOD_CTRL},
             Condition_ENABLE});
    m_event_manager.bind(
            {ICON_FA_SAVE " Save as",
             fw::EventType_save_file_as_triggered,
             {SDLK_s, KMOD_CTRL},
             Condition_ENABLE});
    m_event_manager.bind(
            {ICON_FA_TIMES "  Close",
             fw::EventType_close_file_triggered,
             {SDLK_w, KMOD_CTRL},
             Condition_ENABLE});
    m_event_manager.bind(
            {ICON_FA_FOLDER_OPEN " Open",
             fw::EventType_browse_file_triggered,
             {SDLK_o, KMOD_CTRL},
             Condition_ENABLE});
    m_event_manager.bind(
            {ICON_FA_FILE " New",
             fw::EventType_new_file_triggered,
             {SDLK_n, KMOD_CTRL},
             Condition_ENABLE});
    m_event_manager.bind(
            {"Splashscreen",
             fw::EventType_show_splashscreen_triggered,
             {SDLK_F1},
             Condition_ENABLE});
    m_event_manager.bind(
            {ICON_FA_SIGN_OUT_ALT " Exit",
             fw::EventType_exit_triggered,
             {SDLK_F4, KMOD_ALT},
             Condition_ENABLE});
    m_event_manager.bind(
            {"Undo",
             fw::EventType_undo_triggered,
             {SDLK_z, KMOD_CTRL},
             Condition_ENABLE});
    m_event_manager.bind(
            {"Redo",
             fw::EventType_redo_triggered,
             {SDLK_y, KMOD_CTRL},
             Condition_ENABLE});
    m_event_manager.bind(
            {"Isolate on/off",
             EventType_toggle_isolate_selection,
             {SDLK_i, KMOD_CTRL},
             Condition_ENABLE | Condition_HIGHLIGHTED_IN_TEXT_EDITOR});
    m_event_manager.bind(
            {"Select",
             fw::EventType_none,
             {0, KMOD_NONE, "Left mouse click on a node"},
             Condition_ENABLE_IF_HAS_NO_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {"Deselect",
             fw::EventType_none,
             {0, KMOD_NONE, "Double left mouse click on background"},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {"Move Graph",
             fw::EventType_none,
             {0, KMOD_NONE, "Left mouse btn drag on background"},
             Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {"Frame Selection",
             EventType_frame_selected_node_views,
             {SDLK_f, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    m_event_manager.bind(
            {"Frame All",
             EventType_frame_all_node_views,
             {SDLK_f, KMOD_LCTRL},
             Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    return true;
}

void App::onUpdate()
{
    Settings& settings = Settings::get_instance();

    // 1. Update current file
    if ( m_current_file )
    {
        m_current_file->update();
    }

    // 2. Handle events

    // shorthand to push all shortcuts to a file view overlay depending on conditions
    auto push_overlay_shortcuts = [=](ndbl::FileView* view, Condition condition) -> void {
        for (const auto& _binded_event: m_event_manager.get_binded_events())
        {
            if( (_binded_event.condition & condition) == condition)
            {
                if (_binded_event.condition & Condition_HIGHLIGHTED_IN_GRAPH_EDITOR)
                {
                    view->push_overlay(
                        {
                            _binded_event.label.substr(0, 12),
                            _binded_event.shortcut.to_string()
                        }, OverlayType_GRAPH);
                }
                if ( _binded_event.condition & Condition_HIGHLIGHTED_IN_TEXT_EDITOR)
                {
                    view->push_overlay(
                        {
                            _binded_event.label.substr(0,12),
                            _binded_event.shortcut.to_string()
                        }, OverlayType_TEXT);
                }
            }

        }
    };

    // Nodable events ( SDL_ API inspired, but with custom events)
    Event event{};
    NodeView*      selected_view = NodeView::get_selected();
    while(m_event_manager.poll_event((fw::Event&)event) )
    {
        switch ( event.type )
        {
            case EventType_toggle_isolate_selection:
            {
                settings.isolate_selection = !settings.isolate_selection;
                if( m_current_file )
                {
                    m_current_file->update_graph();
                }
                break;
            }

            case fw::EventType_exit_triggered:
            {
                flag_to_stop();
                break;
            }

            case fw::EventType_close_file_triggered:
            {
                if( m_current_file ) close_file(m_current_file);
                break;
            }

            case fw::EventType_undo_triggered:
            {
                if( m_current_file ) m_current_file->get_history()->undo();
                break;
            }

            case fw::EventType_redo_triggered:
            {
                if( m_current_file ) m_current_file->get_history()->redo();
                break;
            }

            case fw::EventType_browse_file_triggered:
            {
                std::string path;
                if(m_view->pick_file_path(path, fw::AppView::DIALOG_Browse))
                {
                    open_file(path);
                    break;
                }
                LOG_MESSAGE("App", "Browse file aborted by user.\n");
                break;

            }

            case fw::EventType_new_file_triggered:
            {
                new_file();
                break;
            }

            case fw::EventType_save_file_as_triggered:
            {
                if (m_current_file)
                {
                    std::string path;
                    if(m_view->pick_file_path(path, fw::AppView::DIALOG_SaveAs))
                    {
                        save_file_as(path);
                        break;
                    }
                }
                break;
            }

            case fw::EventType_save_file_triggered:
            {
                if (m_current_file)
                {
                    if(m_current_file->has_path())
                    {
                        save_file();
                    }
                    else
                    {
                        std::string path;
                        if(m_view->pick_file_path(path, fw::AppView::DIALOG_SaveAs))
                        {
                            save_file_as(path);
                        }
                    }
                }
                break;
            }

            case fw::EventType_show_splashscreen_triggered:
            {
                m_view->set_splashscreen_visible(true);
                break;
            }

            case EventType_frame_selected_node_views:
            {
                auto view = m_current_file->get_graph()->get<GraphNodeView>();
                view->frame_selected_node_views();
                break;
            }

            case EventType_frame_all_node_views:
            {
                auto view = m_current_file->get_graph()->get<GraphNodeView>();
                view->frame_all_node_views();
                break;
            }

            case EventType_node_view_selected:
            {
                FileView* view = m_current_file->get_view();
                view->clear_overlay();
                push_overlay_shortcuts(view, Condition_ENABLE_IF_HAS_SELECTION);
                LOG_MESSAGE( "App", "NodeView selected\n")
                break;
            }
            case fw::EventType_file_opened:
            {
                if (!m_current_file) break;
                m_current_file->update_graph();
                FileView *view = m_current_file->get_view();
                view->clear_overlay();
                push_overlay_shortcuts(view, Condition_ENABLE_IF_HAS_NO_SELECTION);
                break;
            }
            case EventType_node_view_deselected:
            {
                FileView* view = m_current_file->get_view();
                view->clear_overlay();
                push_overlay_shortcuts(view, Condition_ENABLE_IF_HAS_NO_SELECTION );
                break;
            }
            case EventType_delete_node_action_triggered:
            {
                if ( selected_view && !ImGui::IsAnyItemFocused() )
                {
                    selected_view->get_owner()->flag_to_delete();
                }
                break;
            }
            case EventType_arrange_node_action_triggered:
            {
                if ( selected_view )
                {
                    selected_view->arrange_recursively();
                }
                break;
            }
            case EventType_select_successor_node_action_triggered:
            {
                if ( selected_view )
                {
                    Node* possible_successor = selected_view->get_owner()->successors().get_front_or_nullptr();
                    if (possible_successor)
                    {
                        if (auto successor_view = possible_successor->get<NodeView>())
                        {
                            NodeView::set_selected(successor_view);
                        }
                    }
                }
                break;
            }
            case EventType_toggle_folding_selected_node_action_triggered:
            {
                if ( selected_view )
                {
                    if( event.toggle_folding.recursive )
                    {
                        selected_view->expand_toggle_rec();
                    }
                    else
                    {
                        selected_view->expand_toggle();
                    }
                }
                break;
            }
            case EventType_node_connector_dropped:
            {
                const NodeConnector *src = event.node_connectors.src;
                const NodeConnector *dst = event.node_connectors.dst;
                if ( src->share_parent_with(dst) )
                {
                    LOG_WARNING("App", "Unable to drop_on these two Connectors from the same Node.\n")
                }
                else if( src->m_way == dst->m_way )
                {
                    LOG_WARNING("App", "Unable to drop_on these two Node Connectors (must have different ways).\n")
                }
                else
                {
                    if ( src->m_way != Way_Out ) std::swap(src, dst); // ensure src is predecessor
                    DirectedEdge edge(src->get_node(), Edge_t::IS_PREDECESSOR_OF, dst->get_node());
                    auto cmd = std::make_shared<Cmd_ConnectEdge>(edge);
                    History *curr_file_history = current_file()->get_history();
                    curr_file_history->push_command(cmd);
                }
                break;
            }
            case EventType_property_connector_dropped:
            {
                const PropertyConnector *src = event.property_connectors.src;
                const PropertyConnector *dst = event.property_connectors.dst;
                fw::type src_meta_type = src->get_property_type();
                fw::type dst_meta_type = dst->get_property_type();

                if ( src->share_parent_with(dst) )
                {
                    LOG_WARNING( "App", "Unable to drop_on two connectors from the same Property.\n" )
                }
                else if (src->m_display_side == dst->m_display_side)
                {
                    LOG_WARNING( "App", "Unable to drop_on two connectors with the same nature (in and in, out and out)\n" )
                }
                else if ( !fw::type::is_implicitly_convertible(src_meta_type, dst_meta_type) )
                {
                    LOG_WARNING( "App", "Unable to drop_on %s to %s\n",
                                src_meta_type.get_fullname().c_str(),
                                dst_meta_type.get_fullname().c_str())
                }
                else
                {
                    if (src->m_way != Way_Out) std::swap(src, dst); // guarantee src to be the output
                    DirectedEdge edge(src->get_property(), dst->get_property());
                    auto cmd = std::make_shared<Cmd_ConnectEdge>(edge);
                    History *curr_file_history = current_file()->get_history();
                    curr_file_history->push_command(cmd);
                }
                break;
            }

            case EventType_node_connector_disconnected:
            {
                const NodeConnector* src_connector = event.node_connectors.src;
                Node* src = src_connector->get_node();
                Node* dst = src_connector->get_connected_node();

                if (src_connector->m_way != Way_Out ) std::swap(src, dst); // ensure src is predecessor

                DirectedEdge edge(src, Edge_t::IS_PREDECESSOR_OF, dst);
                auto cmd = std::make_shared<Cmd_DisconnectEdge>( edge );

                History *curr_file_history = current_file()->get_history();
                curr_file_history->push_command(cmd);

                break;
            }
            case EventType_property_connector_disconnected:
            {
                const PropertyConnector* src_connector = event.property_connectors.src;
                Property * src = src_connector->get_property();

                auto edges = src->get_owner()
                                     ->get_parent_graph()
                                     ->filter_edges(src, src_connector->m_way);

                auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Edges");
                for(auto each_edge: edges)
                {
                    auto each_cmd = std::make_shared<Cmd_DisconnectEdge>(*each_edge);
                    cmd_grp->push_cmd( std::static_pointer_cast<ICommand>(each_cmd) );
                }

                History *curr_file_history = current_file()->get_history();
                curr_file_history->push_command(std::static_pointer_cast<ICommand>(cmd_grp));

                break;
            }

            default:
            {
                LOG_VERBOSE("App", "Ignoring and event, this case is not handled\n")
            }
        }
    }
}

bool App::onShutdown()
{
    LOG_VERBOSE("App", "onShutdown ...\n")
    for( File* each_file : m_loaded_files )
    {
        LOG_VERBOSE("App", "Delete file %s ...\n", each_file->get_path().c_str())
        delete each_file;
    }
    LOG_VERBOSE("App", "onShutdown OK\n")
    return true;
}

bool App::open_file(const ghc::filesystem::path& _path, bool relative)
{
    std::string absolute_path = relative ? to_absolute_asset_path(_path.string().c_str()) : _path.string();
    auto file = new File( _path.filename().string(), absolute_path);

    if ( !file->read_from_disk() )
    {
        LOG_ERROR("File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
        delete file;
        return false;
    }

    m_loaded_files.push_back( file );
    current_file(file);
    m_event_manager.push_event(fw::EventType_file_opened);
	return true;
}

void App::save_file() const
{
	if (m_current_file && !m_current_file->write_to_disk() )
    {
        LOG_ERROR("App", "Unable to save %s (%s)\n", m_current_file->get_name().c_str(), m_current_file->get_path().c_str());
    }

}

void App::save_file_as(const ghc::filesystem::path &_path)
{
    File* curr_file = current_file();
    curr_file->set_path(_path.string());
    curr_file->set_name(_path.filename().string());
    if( !curr_file->write_to_disk() )
    {
        LOG_ERROR("App", "Unable to save as %s (%s)\n", _path.filename().c_str(), _path.c_str());
    }
}

File* App::current_file()const
{
	return m_current_file;
}

void App::current_file(File* _file)
{
    m_current_file = _file;
}

void App::close_file(File* _file)
{
    if ( _file )
    {
        auto it = std::find(m_loaded_files.begin(), m_loaded_files.end(), _file);
        FW_ASSERT(it != m_loaded_files.end());
        it = m_loaded_files.erase(it);

        if ( it != m_loaded_files.end() ) //---- try to load the file next
        {
            m_current_file = *it;
        }
        else
        {
            m_current_file = nullptr;
        }

        delete _file;
    }
}

bool App::compile_and_load_program()
{
    if ( File* file = current_file())
    {
        const GraphNode* graph = file->get_graph();

        if (graph)
        {
            assembly::Compiler compiler;
            auto asm_code = compiler.compile_syntax_tree(graph);

            if (asm_code)
            {
                auto& vm = VirtualMachine::get_instance();
                vm.release_program();

                if (vm.load_program(std::move(asm_code)))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void App::run_program()
{
    if (compile_and_load_program() )
    {
        VirtualMachine::get_instance().run_program();
    }
}

void App::debug_program()
{
    if (compile_and_load_program() )
    {
        VirtualMachine::get_instance().debug_program();
    }
}

void App::step_over_program()
{
    auto& vm = VirtualMachine::get_instance();
    vm.step_over();
    if (!vm.is_there_a_next_instr() )
    {
        NodeView::set_selected(nullptr);
    }
    else if ( auto view = vm.get_next_node()->get<NodeView>() )
    {
        NodeView::set_selected(view);
    }
}

void App::stop_program()
{
    VirtualMachine::get_instance().stop_program();
}

void App::reset_program()
{
    if ( auto currFile = current_file() )
    {
        auto& vm = VirtualMachine::get_instance();
        if ( vm.is_program_running() )
        {
            vm.stop_program();
        }

        // TODO: restore graph state without parsing again like that:
        currFile->update_graph();
    }
}

File *App::new_file()
{
    // get a unique friendly name
    const char* basename   = "Untitled";
    char        name[255];
    snprintf(name, sizeof(name), "%s.cpp", basename);

    int num = 0;
    for(auto each_file : m_loaded_files)
    {
        if( strcmp( each_file->get_name().c_str(), name) == 0 )
        {
            snprintf(name, sizeof(name), "%s_%i.cpp", basename, ++num);
        }
    }

    // create the file
    auto file = new File(name);
    m_loaded_files.push_back(file);
    current_file(file);

    return file;
}

App& App::get_instance()
{
    FW_EXPECT(s_instance, "No App instance available. Did you forget App app(...) or App* app = new App(...)");
    return *s_instance;
}

fw::AppView* App::get_view()
{
    return m_view;
}
