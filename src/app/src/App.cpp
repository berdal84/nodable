#include <nodable/app/App.h>

#include <algorithm>
#include <nodable/core/VariableNode.h>
#include <nodable/core/DataAccess.h>
#include <nodable/app/build_info.h> // file generated (aka configured) from build_info.h.in
#include <nodable/app/NodeView.h>
#include <nodable/app/AppView.h>
#include <nodable/app/File.h>
#include <nodable/app/AppContext.h>
#include <nodable/app/Event.h>
#include <nodable/app/commands/Cmd_ConnectMembers.h>
#include <nodable/app/MemberConnector.h>
#include <nodable/app/NodeConnector.h>
#include <nodable/app/commands/Cmd_ConnectNodes.h>
#include <nodable/app/commands/Cmd_DisconnectNodes.h>
#include <nodable/app/commands/Cmd_DisconnectMembers.h>
#include <nodable/app/commands/Cmd_Group.h>
#include <nodable/core/System.h>

using namespace Nodable;

App::App()
    : m_current_file_index(0)
    , m_should_stop(false)
{
    m_executable_folder_path = ghc::filesystem::path( System::get_executable_directory() );
    m_assets_folder_path     =  m_executable_folder_path / BuildInfo::assets_dir;
    LOG_MESSAGE("App", "Executable folder path: %s\n", m_executable_folder_path.c_str() )
    LOG_MESSAGE("App", "Asset folder path:      %s\n", m_assets_folder_path.c_str() )
    Nodable::R::init(); // Reflection system.
    m_context = AppContext::create_default(this);
	m_view = new AppView(m_context, BuildInfo::version_extended);
}

App::~App()
{
	delete m_view;
	delete m_context;
}

bool App::init()
{
    return m_view->init();
}

void App::update()
{
    handle_events();

    if (File* file = get_curr_file())
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
    if (m_view) m_view->shutdown();
}

bool App::open_file(const ghc::filesystem::path& _filePath)
{		
	auto file = File::open(m_context, _filePath.string(), _filePath.filename().string());

	if (file)
	{
		m_loaded_files.push_back(file);
        set_curr_file(m_loaded_files.size() - 1);
	}

	return file != nullptr;
}

void App::save_file() const
{
	File* current_file = get_curr_file();
	if (current_file) current_file->write_to_disk();
}

void App::close_file()
{
    close_file_at(m_current_file_index);
}

File* App::get_curr_file()const {

	if (m_loaded_files.size() > m_current_file_index) {
		return m_loaded_files.at(m_current_file_index);
	}
	return nullptr;
}

void App::set_curr_file(size_t _index)
{
	if (m_loaded_files.size() > _index)
	{
        m_current_file_index = _index;
	}
}

std::string App::get_absolute_asset_path(const char* _relative_path)const
{
    fs_path result = m_assets_folder_path / _relative_path;
	return result.string();
}

size_t App::get_file_count() const
{
	return m_loaded_files.size();
}

File *App::get_file_at(size_t _index) const
{
	return m_loaded_files[_index];
}

size_t App::get_curr_file_index() const
{
	return m_current_file_index;
}

void App::close_file_at(size_t _fileIndex)
{
    auto currentFile = m_loaded_files.at(_fileIndex);
    if (currentFile != nullptr)
    {
        auto it = std::find(m_loaded_files.begin(), m_loaded_files.end(), currentFile);
        m_loaded_files.erase(it);
        delete currentFile;
        if (m_current_file_index > 0)
            set_curr_file(m_current_file_index - 1);
        else
            set_curr_file(m_current_file_index);
    }
}

bool App::vm_compile_and_load_program()
{
    const GraphNode* graph = nullptr;

    if ( File* file = get_curr_file())
    {
        graph = file->get_graph();
    }

    if (graph)
    {
        Asm::Compiler compiler;
        std::unique_ptr<const Asm::Code> asm_code = compiler.compile_syntax_tree(graph);

        if (asm_code)
        {
            m_context->vm->release_program();

            if (m_context->vm->load_program(std::move(asm_code)))
            {
                return true;
            }
        }
    }

    return false;
}

void App::vm_run()
{
    if ( vm_compile_and_load_program() )
    {
        m_context->vm->run_program();
    }
}

void App::vm_debug()
{
    if ( vm_compile_and_load_program() )
    {
        m_context->vm->debug_program();
    }
}

void App::vm_step_over()
{
    m_context->vm->step_over();
    if (!m_context->vm->is_there_a_next_instr() )
    {
        NodeView::SetSelected(nullptr);
    }
    else if ( auto view = m_context->vm->get_next_node()->get<NodeView>() )
    {
        NodeView::SetSelected(view);
    }
}

void App::vm_stop()
{
    m_context->vm->stop_program();
}

void App::vm_reset()
{
    if ( auto currFile = get_curr_file() )
    {
        if(m_context->vm->is_program_running())
            m_context->vm->stop_program();

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
    m_view->handle_events();

    /*
     * Nodable events
     *
     * SDL_ API inspired, but with custom events.
     */
    Nodable::Event event{};
    NodeView*      selected_view = NodeView::GetSelected();
    while( EventManager::poll_event(event) )
    {
        switch ( event.type )
        {
            case EventType::delete_node_action_triggered:
            {
                if ( selected_view )
                {
                    selected_view->get_owner()->flag_for_deletion();
                }
                break;
            }
            case EventType::arrange_node_action_triggered:
            {
                if ( selected_view )
                {
                    selected_view->arrangeRecursively();
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
                            NodeView::SetSelected(successor_view);
                        }
                    }
                }
                break;
            }
            case EventType::expand_selected_node_action_triggered:
            {
                if ( selected_view )
                {
                    selected_view->toggleExpansion();
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
                    History *curr_file_history = get_curr_file()->get_history();
                    curr_file_history->push_command(cmd);
                }
                break;
            }
            case EventType::member_connector_dropped:
            {
                const MemberConnector *src = event.member_connectors.src;
                const MemberConnector *dst = event.member_connectors.dst;
                std::shared_ptr<const R::MetaType> src_meta_type = src->get_member_type();
                std::shared_ptr<const R::MetaType> dst_meta_type = dst->get_member_type();

                if ( src->share_parent_with(dst) )
                {
                    LOG_WARNING( "App", "Unable to drop_on two connectors from the same Member.\n" )
                }
                else if (src->m_display_side == dst->m_display_side)
                {
                    LOG_WARNING( "App", "Unable to drop_on two connectors with the same nature (in and in, out and out)\n" )
                }
                else if ( !R::MetaType::is_convertible( src_meta_type, dst_meta_type ) )
                {
                    LOG_WARNING( "App", "Unable to drop_on %s to %s\n",
                                 src_meta_type->get_fullname().c_str(),
                                 dst_meta_type->get_fullname().c_str())
                }
                else
                {
                    if (src->m_way != Way_Out) std::swap(src, dst); // guarantee src to be the output
                    auto cmd = std::make_shared<Cmd_ConnectMembers>(src->get_member(), dst->get_member());
                    History *curr_file_history = get_curr_file()->get_history();
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

                History *curr_file_history = get_curr_file()->get_history();
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

                History *curr_file_history = get_curr_file()->get_history();
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
    m_view->draw();
}

