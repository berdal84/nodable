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
    : current_file(nullptr)
    , framework(config.framework)
    , view(this)
    , vm()
{
    LOG_VERBOSE("ndbl::App", "Constructor ...\n");

    FW_EXPECT(s_instance == nullptr, "Can't create two concurrent App. Delete first instance.");
    s_instance = this;

    // Bind methods to framework events
    LOG_VERBOSE("ndbl::App", "Binding framework ...\n");
    framework.event_after_init.connect([this](){ on_init();});
    framework.event_after_update.connect([this](){ on_update();});
    framework.event_after_shutdown.connect([this](){ on_shutdown();});

    LOG_VERBOSE("ndbl::App", "Constructor " OK "\n");

}

App::~App()
{
    s_instance = nullptr;
    LOG_VERBOSE("ndbl::App", "Destructor " OK "\n");
}

bool App::on_init()
{
    LOG_VERBOSE("ndbl::App", "on_init ...\n");

    fw::EventManager& event_manager = framework.event_manager;

    // Bind commands to shortcuts
    using fw::EventType;
    event_manager.bind(
            {"Delete",
             EventType_delete_node_action_triggered,
             {SDLK_DELETE, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {"Arrange",
             EventType_arrange_node_action_triggered,
             {SDLK_a, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {"Fold/Unfold",
             EventType_toggle_folding_selected_node_action_triggered,
             {SDLK_x, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {"Next",
             EventType_select_successor_node_action_triggered,
             {SDLK_n, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {ICON_FA_SAVE " Save",
             fw::EventType_save_file_triggered,
             {SDLK_s, KMOD_CTRL},
             Condition_ENABLE});
    event_manager.bind(
            {ICON_FA_SAVE " Save as",
             fw::EventType_save_file_as_triggered,
             {SDLK_s, KMOD_CTRL},
             Condition_ENABLE});
    event_manager.bind(
            {ICON_FA_TIMES "  Close",
             fw::EventType_close_file_triggered,
             {SDLK_w, KMOD_CTRL},
             Condition_ENABLE});
    event_manager.bind(
            {ICON_FA_FOLDER_OPEN " Open",
             fw::EventType_browse_file_triggered,
             {SDLK_o, KMOD_CTRL},
             Condition_ENABLE});
    event_manager.bind(
            {ICON_FA_FILE " New",
             fw::EventType_new_file_triggered,
             {SDLK_n, KMOD_CTRL},
             Condition_ENABLE});
    event_manager.bind(
            {"Splashscreen",
             fw::EventType_show_splashscreen_triggered,
             {SDLK_F1},
             Condition_ENABLE});
    event_manager.bind(
            {ICON_FA_SIGN_OUT_ALT " Exit",
             fw::EventType_exit_triggered,
             {SDLK_F4, KMOD_ALT},
             Condition_ENABLE});
    event_manager.bind(
            {"Undo",
             fw::EventType_undo_triggered,
             {SDLK_z, KMOD_CTRL},
             Condition_ENABLE});
    event_manager.bind(
            {"Redo",
             fw::EventType_redo_triggered,
             {SDLK_y, KMOD_CTRL},
             Condition_ENABLE});
    event_manager.bind(
            {"Isolate on/off",
             EventType_toggle_isolate_selection,
             {SDLK_i, KMOD_CTRL},
             Condition_ENABLE | Condition_HIGHLIGHTED_IN_TEXT_EDITOR});
    event_manager.bind(
            {"Select",
             fw::EventType_none,
             {0, KMOD_NONE, "Left mouse click on a node"},
             Condition_ENABLE_IF_HAS_NO_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {"Deselect",
             fw::EventType_none,
             {0, KMOD_NONE, "Double left mouse click on background"},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {"Move Graph",
             fw::EventType_none,
             {0, KMOD_NONE, "Left mouse btn drag on background"},
             Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {"Frame Selection",
             EventType_frame_selected_node_views,
             {SDLK_f, KMOD_NONE},
             Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});
    event_manager.bind(
            {"Frame All",
             EventType_frame_all_node_views,
             {SDLK_f, KMOD_LCTRL},
             Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR});

    LOG_VERBOSE("ndbl::App", "after_init ...\n");
    after_init.emit();
    LOG_VERBOSE("ndbl::App", "on_init " OK "\n");

    return true;
}

void App::on_update()
{
    LOG_VERBOSE("ndbl::App", "on_update ...\n");
    fw::EventManager& event_manager = framework.event_manager;

    // 1. Update current file
    if (current_file)
    {
        current_file->update();
    }

    // 2. Handle events

    // shorthand to push all shortcuts to a file view overlay depending on conditions
    auto push_overlay_shortcuts = [&](ndbl::FileView& _view, Condition _condition) -> void {
        for (const auto& _binded_event: event_manager.get_binded_events())
        {
            if( (_binded_event.condition & _condition) == _condition)
            {
                if (_binded_event.condition & Condition_HIGHLIGHTED_IN_GRAPH_EDITOR)
                {
                    _view.push_overlay(
                        {
                            _binded_event.label.substr(0, 12),
                            _binded_event.shortcut.to_string()
                        }, OverlayType_GRAPH);
                }
                if ( _binded_event.condition & Condition_HIGHLIGHTED_IN_TEXT_EDITOR)
                {
                    _view.push_overlay(
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
    GraphNodeView* graph_view    = current_file ?
                                   current_file->get_graph()->get<GraphNodeView>() : nullptr;
    History* curr_file_history   = current_file ? current_file->get_history() : nullptr;

    while(event_manager.poll_event((fw::Event&)event) )
    {
        switch ( event.type )
        {
            case EventType_toggle_isolate_selection:
            {
                config.isolate_selection = !config.isolate_selection;
                if(current_file)
                {
                    current_file->update_graph();
                }
                break;
            }

            case fw::EventType_exit_triggered:
            {
                framework.should_stop = true;
                break;
            }

            case fw::EventType_close_file_triggered:
            {
                if(current_file) close_file(current_file);
                break;
            }
            case fw::EventType_undo_triggered:
            {
                if(current_file) curr_file_history->undo();
                break;
            }

            case fw::EventType_redo_triggered:
            {
                if(current_file) curr_file_history->redo();
                break;
            }

            case fw::EventType_browse_file_triggered:
            {
                std::string path;
                if( pick_file_path(path, fw::AppView::DIALOG_Browse))
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
                if (current_file)
                {
                    std::string path;
                    if(pick_file_path(path, fw::AppView::DIALOG_SaveAs))
                    {
                        save_file_as(path);
                        break;
                    }
                }
                break;
            }

            case fw::EventType_save_file_triggered:
            {
                if (current_file)
                {
                    if( !current_file->path.empty())
                    {
                        save_file(current_file);
                    }
                    else
                    {
                        std::string path;
                        if(pick_file_path(path, fw::AppView::DIALOG_SaveAs))
                        {
                            save_file_as(path);
                        }
                    }
                }
                break;
            }

            case fw::EventType_show_splashscreen_triggered:
            {
                framework.config.splashscreen = true;
                break;
            }
             case EventType_frame_selected_node_views:
            {
                graph_view->frame_selected_node_views();
                break;
            }

            case EventType_frame_all_node_views:
            {
                graph_view->frame_all_node_views();
                break;
            }
            case EventType_node_view_selected:
            {
                current_file->view.clear_overlay();
                push_overlay_shortcuts(current_file->view, Condition_ENABLE_IF_HAS_SELECTION);
                LOG_MESSAGE( "App", "NodeView selected\n")
                break;
            }
            case fw::EventType_file_opened:
            {
                if (!current_file) break;
                current_file->update_graph();
                current_file->view.clear_overlay();
                push_overlay_shortcuts(current_file->view, Condition_ENABLE_IF_HAS_NO_SELECTION);
                break;
            }
            case EventType_node_view_deselected:
            {
                 current_file->view.clear_overlay();
                push_overlay_shortcuts(current_file->view, Condition_ENABLE_IF_HAS_NO_SELECTION );
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
                curr_file_history->push_command(std::static_pointer_cast<ICommand>(cmd_grp));

                break;
            }

            default:
            {
                LOG_VERBOSE("App", "Ignoring and event, this case is not handled\n")
            }
        }
    }
    LOG_VERBOSE("ndbl::App", "on_update " OK "\n");
}

bool App::on_shutdown()
{
    LOG_VERBOSE("ndbl::App", "on_shutdown ...\n");
    for( File* each_file : m_loaded_files )
    {
        LOG_VERBOSE("ndbl::App", "Delete file %s ...\n", each_file->path.c_str())
        delete each_file;
    }
    LOG_VERBOSE("ndbl::App", "on_shutdown " OK "\n");
    return true;
}

File *App::open_file(const ghc::filesystem::path& _path)
{
    auto file = new File( fw::App::asset_path(_path) );

    if ( !file->load() )
    {
        LOG_ERROR("File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
        delete file;
        return nullptr;
    }

    return open_file(file);
}

File *App::open_file(File* _file)
{
    if(!_file) return nullptr;

    m_loaded_files.push_back( _file );
    current_file = _file;
    framework.event_manager.push(fw::EventType_file_opened);
    return _file;
}

void App::save_file(File* _file) const
{
    FW_EXPECT(_file,"file must be defined");

	if ( !_file->write_to_disk() )
    {
        LOG_ERROR("ndbl::App", "Unable to save %s (%s)\n", _file->name.c_str(), _file->path.c_str());
        return;
    }
    LOG_MESSAGE("ndbl::App", "File saved: %s\n", _file->path.c_str());
}

void App::save_file_as(const ghc::filesystem::path& _path) const
{
    ghc::filesystem::path absolute_path = fw::App::asset_path(_path);
    current_file->path = absolute_path.string();
    current_file->name = absolute_path.filename().string();
    if( !current_file->write_to_disk() )
    {
        LOG_ERROR("App", "Unable to save as %s (%s)\n", absolute_path.filename().c_str(), absolute_path.c_str());
    }
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
            current_file = *it;
        }
        else
        {
            current_file = nullptr;
        }

        delete _file;
    }
}

bool App::compile_and_load_program()
{
    if ( current_file )
    {
        const GraphNode* graph = current_file->get_graph();

        if (graph)
        {
            assembly::Compiler compiler;
            auto asm_code = compiler.compile_syntax_tree(graph);

            if (asm_code)
            {
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
        vm.run_program();
    }
}

void App::debug_program()
{
    if (compile_and_load_program() )
    {
        vm.debug_program();
    }
}

void App::step_over_program()
{
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
    vm.stop_program();
}

void App::reset_program()
{
    if(!current_file) return;

    if ( vm.is_program_running() )
    {
        vm.stop_program();
    }
    current_file->update_graph();
}

File *App::new_file()
{
    // 1. Create the file in memory

    // 1.a Determine a unique-ish name
    const char* basename   = "Untitled";
    char        name[255];
    snprintf(name, sizeof(name), "%s.cpp", basename);

    int num = 0;
    for(auto each_file : m_loaded_files)
    {
        if( strcmp( each_file->name.c_str(), name) == 0 )
        {
            snprintf(name, sizeof(name), "%s_%i.cpp", basename, ++num);
        }
    }

    // 1.b Create instance
    File* file = new File(ghc::filesystem::path{name});

    // 2. try to open from disk
    if ( !open_file(file) )
    {
        delete file;
        return nullptr;
    }
    return file;
}

App& App::get_instance()
{
    FW_EXPECT(s_instance, "No App instance available. Did you forget App app(...) or App* app = new App(...)");
    return *s_instance;
}

void App::toggle_fullscreen()
{
    framework.set_fullscreen(!is_fullscreen() );
}

bool App::is_fullscreen() const
{
    return framework.is_fullscreen();
}

bool App::pick_file_path(std::string &out, fw::AppView::DialogType type)
{
    return framework.view.pick_file_path(out, type);
}
