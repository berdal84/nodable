#include <nodable/app/App.h>

#include <algorithm>
#include <nodable/core/VariableNode.h>
#include <nodable/core/DataAccess.h>
#include <nodable/app/build_info.h> // file generated (aka configured) from build_info.h.in
#include <nodable/app/NodeView.h>
#include <nodable/app/AppView.h>
#include <nodable/app/File.h>
#include <nodable/app/IAppCtx.h>
#include <nodable/app/Event.h>
#include <nodable/app/commands/Cmd_ConnectMembers.h>
#include <nodable/app/MemberConnector.h>
#include <nodable/app/NodeConnector.h>
#include <nodable/app/commands/Cmd_ConnectNodes.h>
#include <nodable/app/commands/Cmd_DisconnectNodes.h>
#include <nodable/app/commands/Cmd_DisconnectMembers.h>
#include <nodable/app/commands/Cmd_Group.h>
#include <nodable/core/System.h>
#include <nodable/core/languages/Nodable.h>

using namespace Nodable;

App::App()
    : m_reflect()  // ---------------- order is important here
    , m_settings() // --
    , m_current_file_index(0)
    , m_current_file(nullptr)
    , m_should_stop(false)
    , m_language( std::make_unique<LanguageNodable>() )
    , m_assets_folder_path( ghc::filesystem::path( System::get_executable_directory() ) / BuildInfo::assets_dir )
    , m_view( *this, BuildInfo::version_extended )
{
    LOG_MESSAGE("App", "Asset folder path:      %s\n", m_assets_folder_path.c_str() )
}

bool App::init()
{
    return m_view.init();
}

void App::update()
{
    handle_events();

    if (File* file = current_file())
    {
        file->update();
    }
}

void App::flag_to_stop()
{
    m_should_stop = true;
}

void App::shutdown()
{
    for( File* each_file : m_loaded_files )
    {
        delete each_file;
    }
    m_view.shutdown();
}

bool App::open_file(const fs_path& _path)
{
    auto file = new File( *this, _path.filename().string(), _path.string());

    if ( !file->read_from_disk() )
    {
        LOG_ERROR("File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
        delete file;
        return false;
    }

    m_loaded_files.push_back( file );
    current_file(file);

	return true;
}

void App::save_file() const
{
	if (m_current_file && !m_current_file->write_to_disk() )
    {
        LOG_ERROR("App", "Unable to save %s (%s)\n", m_current_file->get_name().c_str(), m_current_file->get_path().c_str());
    }
}

void App::save_file_as(const fs_path &_path)
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

std::string App::compute_asset_path(const char* _relative_path) const
{
    fs_path result = m_assets_folder_path / _relative_path;
	return result.string();
}

void App::close_file(File* _file)
{
    if ( _file )
    {
        auto it = std::find(m_loaded_files.begin(), m_loaded_files.end(), _file);
        NODABLE_ASSERT(it != m_loaded_files.end());
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
                m_vm.release_program();

                if (m_vm.load_program(std::move(asm_code)))
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
        m_vm.run_program();
    }
}

void App::debug_program()
{
    if (compile_and_load_program() )
    {
        m_vm.debug_program();
    }
}

void App::step_over_program()
{
    m_vm.step_over();
    if (!m_vm.is_there_a_next_instr() )
    {
        NodeView::set_selected(nullptr);
    }
    else if ( auto view = m_vm.get_next_node()->get<NodeView>() )
    {
        NodeView::set_selected(view);
    }
}

void App::stop_program()
{
    m_vm.stop_program();
}

void App::reset_program()
{
    if ( auto currFile = current_file() )
    {
        if ( m_vm.is_program_running() )
        {
            m_vm.stop_program();
        }

        // TODO: restore graph state without parsing again like that:
        currFile->update_graph();
    }
}

void App::handle_events()
{
    /*
     * SDL events
     *
     * Some of them might trigger a Nodable event, we will handle them just after.
     */
    m_view.handle_events();

    /*
     * Nodable events
     *
     * SDL_ API inspired, but with custom events.
     */
    Nodable::Event event{};
    NodeView*      selected_view = NodeView::get_selected();
    while( EventManager::poll_event(event) )
    {
        switch ( event.type )
        {
            case EventType::delete_node_action_triggered:
            {
                if ( selected_view && !ImGui::IsAnyItemFocused() )
                {
                    selected_view->get_owner()->flag_for_deletion();
                }
                break;
            }
            case EventType::arrange_node_action_triggered:
            {
                if ( selected_view )
                {
                    selected_view->arrange_recursively();
                }
                break;
            }
            case EventType::select_successor_node_action_triggered:
            {
                if ( selected_view )
                {
                    Node* possible_successor = selected_view->get_owner()->successor_slots().get_front_or_nullptr();
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
            case EventType::toggle_folding_selected_node_action_triggered:
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
            case EventType::node_connector_dropped:
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
                    auto cmd = std::make_shared<Cmd_ConnectNodes>(src->get_node(), dst->get_node(), EdgeType::IS_PREDECESSOR_OF);
                    History *curr_file_history = current_file()->get_history();
                    curr_file_history->push_command(cmd);
                }
                break;
            }
            case EventType::member_connector_dropped:
            {
                const MemberConnector *src = event.member_connectors.src;
                const MemberConnector *dst = event.member_connectors.dst;
                std::shared_ptr<const R::Meta_t> src_meta_type = src->get_member_type();
                std::shared_ptr<const R::Meta_t> dst_meta_type = dst->get_member_type();

                if ( src->share_parent_with(dst) )
                {
                    LOG_WARNING( "App", "Unable to drop_on two connectors from the same Member.\n" )
                }
                else if (src->m_display_side == dst->m_display_side)
                {
                    LOG_WARNING( "App", "Unable to drop_on two connectors with the same nature (in and in, out and out)\n" )
                }
                else if ( !R::Meta_t::is_implicitly_convertible(src_meta_type, dst_meta_type) )
                {
                    LOG_WARNING( "App", "Unable to drop_on %s to %s\n",
                                 src_meta_type->get_fullname().c_str(),
                                 dst_meta_type->get_fullname().c_str())
                }
                else
                {
                    if (src->m_way != Way_Out) std::swap(src, dst); // guarantee src to be the output
                    auto cmd = std::make_shared<Cmd_ConnectMembers>(src->get_member(), dst->get_member());
                    History *curr_file_history = current_file()->get_history();
                    curr_file_history->push_command(cmd);
                }
                break;
            }
            case EventType::node_connector_disconnected:
            {
                const NodeConnector* src_connector = event.node_connectors.src;
                Node* src = src_connector->get_node();
                Node* dst = src_connector->get_connected_node();

                if (src_connector->m_way != Way_Out ) std::swap(src, dst); // ensure src is predecessor

                DirectedEdge relation(src, EdgeType::IS_PREDECESSOR_OF, dst);
                auto cmd = std::make_shared<Cmd_DisconnectNodes>( relation );

                History *curr_file_history = current_file()->get_history();
                curr_file_history->push_command(cmd);

                break;
            }
            case EventType::member_connector_disconnected:
            {
                const MemberConnector* src_connector = event.member_connectors.src;
                Member* src = src_connector->get_member();
                auto wires = src->get_owner()->get_parent_graph()->filter_wires(src, src_connector->m_way);

                auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Wires");
                for(Wire* each_wire : wires )
                {
                    auto each_cmd = std::make_shared<Cmd_DisconnectMembers>(each_wire);
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

void App::draw()
{
    m_view.draw();
}

File *App::new_file()
{
    // get a unique friendly name
    const char* basename   = "Untitled";
    char        name[18]; // "Untitled_XXX.cpp\0";
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
    auto file = new File( *this, name);
    m_loaded_files.push_back(file);
    current_file(file);

    return file;
}
