#include <nodable/App.h>

#include <algorithm>

#include <nodable/NodeView.h>
#include <nodable/AppView.h>
#include <nodable/File.h>
#include <nodable/BuildInfo.h>
#include <nodable/VariableNode.h>
#include <nodable/DataAccess.h>
#include <nodable/AppContext.h>
#include <nodable/Event.h>
#include <nodable/commands/Cmd_ConnectMembers.h>
#include <nodable/MemberConnector.h>
#include <nodable/NodeConnector.h>
#include <nodable/commands/Cmd_ConnectNodes.h>
#include <nodable/commands/Cmd_DisconnectNodes.h>
#include <nodable/commands/Cmd_DisconnectMembers.h>
#include <nodable/commands/Cmd_Group.h>

using namespace Nodable;

App::App(const char* _name)
    : m_currentFileIndex(0)
    , m_assetsFolderPath( ghc::filesystem::current_path() / BuildInfo::assets_dir )
    , m_name(_name)
    , m_should_stop(false)
{
    LOG_MESSAGE("App", "Asset folder is %s\n", m_assetsFolderPath.c_str() )

    m_context = AppContext::create_default(this);
	m_view = new AppView(m_context, _name);
}

App::~App()
{
	delete m_view;
	delete m_context;
}

bool App::init()
{
    m_view->init();
	return true;
}

void App::update()
{
	File* file = get_curr_file();

    handle_events();

    if (file)
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
    m_view->shutdown();
}

bool App::open_file(const ghc::filesystem::path& _filePath)
{		
	auto file = File::OpenFile(m_context, _filePath.string() );

	if (file != nullptr)
	{
		m_loadedFiles.push_back(file);
        set_curr_file(m_loadedFiles.size() - 1);
	}

	return file != nullptr;
}

void App::save_file() const
{
	auto currentFile = m_loadedFiles.at(m_currentFileIndex);
	if (currentFile)
		currentFile->save();
}

void App::close_file()
{
    this->close_file_at(this->m_currentFileIndex);
}

File* App::get_curr_file()const {

	if (m_loadedFiles.size() > m_currentFileIndex) {
		return m_loadedFiles.at(m_currentFileIndex);
	}
	return nullptr;
}

void App::set_curr_file(size_t _index)
{
	if (m_loadedFiles.size() > _index)
	{
        m_currentFileIndex = _index;
	}
}

std::string App::get_asset_path(const char* _fileName)const
{
    ghc::filesystem::path assetPath(m_assetsFolderPath);
	assetPath /= _fileName;
	return assetPath.string();
}

size_t App::get_file_count() const
{
	return m_loadedFiles.size();
}

File *App::get_file_at(size_t _index) const
{
	return m_loadedFiles[_index];
}

size_t App::get_curr_file_index() const
{
	return m_currentFileIndex;
}

void App::close_file_at(size_t _fileIndex)
{
    auto currentFile = m_loadedFiles.at(_fileIndex);
    if (currentFile != nullptr)
    {
        auto it = std::find(m_loadedFiles.begin(), m_loadedFiles.end(), currentFile);
        m_loadedFiles.erase(it);
        delete currentFile;
        if (m_currentFileIndex > 0)
            set_curr_file(m_currentFileIndex - 1);
        else
            set_curr_file(m_currentFileIndex);
    }
}

Node* App::get_curr_file_program_root() const
{
    if ( File* file = get_curr_file())
        return file->getGraph()->get_root();
    return nullptr;
}

void App::vm_run()
{
    Node* program = get_curr_file_program_root();
    if (program && m_context->vm->load_program(program) )
    {
        m_context->vm->run_program();
    }
}

void App::vm_debug()
{
    Node* program = get_curr_file_program_root();
    if (program && m_context->vm->load_program(program) )
    {
        m_context->vm->debug_program();
    }
}

void App::vm_step_over()
{
    m_context->vm->step_over();
    if (m_context->vm->is_program_over() )
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
        currFile->evaluateSelectedExpression();
    }
}

void App::handle_events()
{
    // SDL_ API inspired
    Event nodable_event;
    while( EventManager::poll_event(nodable_event) )
    {
        switch ( nodable_event.type )
        {
            case EventType::delete_node_triggered:
            {
                if ( NodeView* selected_view = NodeView::GetSelected() )
                {
                    selected_view->get_owner()->flag_for_deletion();
                }
                break;
            }
            case EventType::arrange_node_triggered:
            {
                if ( NodeView* selected_view = NodeView::GetSelected() )
                {
                    selected_view->arrangeRecursively();
                }
                break;
            }
            case EventType::select_successor_node_triggered:
            {
                if ( NodeView* selected_view = NodeView::GetSelected() )
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
            case EventType::expand_selected_node_triggered:
            {
                if ( NodeView* selected_view = NodeView::GetSelected() )
                {
                    selected_view->toggleExpansion();
                }
                break;
            }
            case EventType::node_connector_dropped:
            {
                const NodeConnector *src = nodable_event.node_connectors.src;
                const NodeConnector *dst = nodable_event.node_connectors.dst;
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
                    auto cmd = std::make_shared<Cmd_ConnectNodes>(src->get_node(), dst->get_node(), Relation_t::IS_PREDECESSOR_OF);
                    History *curr_file_history = get_curr_file()->getHistory();
                    curr_file_history->push_command(cmd);
                }
                break;
            }
            case EventType::member_connector_dropped:
            {
                const MemberConnector *src = nodable_event.member_connectors.src;
                const MemberConnector *dst = nodable_event.member_connectors.dst;
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
                    History *curr_file_history = get_curr_file()->getHistory();
                    curr_file_history->push_command(cmd);
                }
                break;
            }
            case EventType::node_connector_disconnected:
            {
                const NodeConnector* src_connector = nodable_event.node_connectors.src;
                Node* src = src_connector->get_node();
                Node* dst = src_connector->get_connected_node();

                if (src_connector->m_way != Way_Out ) std::swap(src, dst); // ensure src is predecessor

                auto cmd = std::make_shared<Cmd_DisconnectNodes>(src, dst, Relation_t::IS_PREDECESSOR_OF);

                History *curr_file_history = get_curr_file()->getHistory();
                curr_file_history->push_command(cmd);

                break;
            }
            case EventType::member_connector_disconnected:
            {
                const MemberConnector* src_connector = nodable_event.member_connectors.src;
                Member* src = src_connector->get_member();
                auto wires = src->get_owner()->get_parent_graph()->filter_wires(src, src_connector->m_way);

                auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Wires");
                for(Wire* each_wire : wires )
                {
                    auto each_cmd = std::make_shared<Cmd_DisconnectMembers>(each_wire);
                    cmd_grp->push_cmd( std::static_pointer_cast<ICommand>(each_cmd) );
                }

                History *curr_file_history = get_curr_file()->getHistory();
                curr_file_history->push_command(std::static_pointer_cast<ICommand>(cmd_grp));

                break;
            }
        }
    }
}

