#include "Nodable.h"

#include <algorithm>

#include "tools/core/assertions.h"
#include "tools/core/System.h"
#include "tools/core/EventManager.h"

#include "ndbl/core/FunctionNode.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/Slot.h"
#include "ndbl/core/Interpreter.h"
#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/ComponentFactory.h"

#include "commands/Cmd_ConnectEdge.h"
#include "commands/Cmd_DeleteEdge.h"
#include "commands/Cmd_Group.h"

#include "Condition.h"
#include "Config.h"
#include "Event.h"
#include "File.h"
#include "GraphView.h"
#include "NodableView.h"
#include "NodeView.h"
#include "SlotView.h"
#include "ndbl/core/Utils.h"

using namespace ndbl;
using namespace tools;

void Nodable::init()
{
    LOG_VERBOSE("ndbl::Nodable", "init_ex ...\n");

    m_config = init_config();
    m_view = new NodableView();
    m_base_app.init_ex(m_view->get_base_view_handle(), m_config->tools_cfg ); // the pointers are owned by this class, base app just use them.
    m_language          = init_language();
    m_interpreter       = init_interpreter();
    m_node_factory      = init_node_factory();
    m_component_factory = init_component_factory();
    m_view->init(this); // must be last

    log::set_verbosity("Physics", log::Verbosity_Error );
    log::set_verbosity("FileView", log::Verbosity_Error );

    LOG_VERBOSE("ndbl::Nodable", "init_ex OK\n");
}

void Nodable::update()
{
    m_base_app.update();
    m_view->update();

    // 1. Update current file
    if (m_current_file && !m_interpreter->is_program_running())
    {
        m_current_file->set_isolation( m_config->isolation ); // might change
        m_current_file->update();
    }

    // 2. Handle events

    // Nodable events
    IEvent*       event = nullptr;
    EventManager* event_manager     = get_event_manager();
    GraphView*    graph_view        = m_current_file ? m_current_file->graph().view() : nullptr; // TODO: should be included in the event
    History*      curr_file_history = m_current_file ? &m_current_file->history : nullptr; // TODO: should be included in the event
    while( (event = event_manager->poll_event()) )
    {
        switch ( event->id )
        {
            case EventID_RESET_GRAPH:
            {
                if ( !m_interpreter->is_program_stopped() )
                    m_interpreter->stop_program();
                m_current_file->set_graph_dirty();
                break;
            }

            case EventID_TOGGLE_ISOLATION_FLAGS:
            {
                m_config->isolation = ~m_config->isolation;
                if(m_current_file)
                {
                    m_current_file->set_graph_dirty();
                }
                break;
            }

            case EventID_REQUEST_EXIT:
            {
                m_base_app.request_stop();
                break;
            }

            case EventID_FILE_CLOSE:
            {
                if(m_current_file) close_file(m_current_file);
                break;
            }
            case EventID_UNDO:
            {
                if(curr_file_history) curr_file_history->undo();
                break;
            }

            case EventID_REDO:
            {
                if(curr_file_history) curr_file_history->redo();
                break;
            }

            case EventID_FILE_BROWSE:
            {
                Path path;
                if( m_view->pick_file_path(path, AppView::DIALOG_Browse))
                {
                    open_file(path);
                    break;
                }
                LOG_MESSAGE("App", "Browse file aborted by user.\n");
                break;

            }

            case EventID_FILE_NEW:
            {
                new_file();
                break;
            }

            case EventID_FILE_SAVE_AS:
            {
                if (m_current_file != nullptr)
                {
                    Path path;
                    if( m_view->pick_file_path(path, AppView::DIALOG_SaveAs))
                    {
                        save_file_as(m_current_file, path);
                    }
                }

                break;
            }

            case EventID_FILE_SAVE:
            {
                if (!m_current_file) break;
                if( !m_current_file->path.empty())
                {
                    save_file(m_current_file);
                }
                else
                {
                    Path path;
                    if( m_view->pick_file_path(path, AppView::DIALOG_SaveAs))
                    {
                        save_file_as(m_current_file, path);
                    }
                }
                break;
            }

            case Event_ShowWindow::id:
            {
                auto _event = reinterpret_cast<Event_ShowWindow*>(event);
                if ( _event->data.window_id == "splashscreen" )
                {
                    m_view->show_splashscreen(_event->data.visible);
                }
                break;
            }

            case Event_FrameSelection::id:
            {
                auto _event = reinterpret_cast<Event_FrameSelection*>( event );
                VERIFY(graph_view, "a graph_view is required");
                graph_view->frame_nodes(_event->data.mode);
                break;
            }

            case Event_SelectionChange::id:
            {
                if (m_current_file == nullptr )
                    break;
                auto _event = reinterpret_cast<Event_SelectionChange*>( event );

                Condition_ condition = _event->data.new_selection.empty() ? Condition_ENABLE_IF_HAS_NO_SELECTION
                                                                          : Condition_ENABLE_IF_HAS_SELECTION;
                m_current_file->view.clear_overlay();
                m_current_file->view.refresh_overlay(condition );
                break;
            }
            case EventID_FILE_OPENED:
            {
                ASSERT(m_current_file != nullptr );
                m_current_file->view.clear_overlay();
                m_current_file->view.refresh_overlay(Condition_ENABLE_IF_HAS_NO_SELECTION );
                break;
            }
            case Event_DeleteNode::id:
            {
                if ( graph_view )
                    for(NodeView* view : graph_view->get_selected())
                        graph_view->graph()->destroy_next_frame(view->node());
                break;
            }

            case Event_ArrangeNode::id:
            {
                if ( graph_view )
                    for(NodeView* view : graph_view->get_selected())
                        view->arrange_recursively();
                break;
            }

            case Event_SelectNext::id:
            {
                if (graph_view && !graph_view->get_selected().empty())
                {
                    std::vector<NodeView*> successors;
                    if (!graph_view->get_selected().empty())
                        for(NodeView* view : graph_view->get_selected() )
                            for (NodeView* successor : Utils::get_components<NodeView>( view->node()->flow_outputs() ) )
                                successors.push_back( successor );

                    graph_view->set_selected(successors, SelectionMode_REPLACE);
                }

                break;
            }

            case Event_ToggleFolding::id:
            {
                if ( graph_view && !graph_view->selection_empty() )
                    for(NodeView* view : graph_view->get_selected())
                    {
                        auto _event = reinterpret_cast<Event_ToggleFolding*>(event);
                        _event->data.mode == RECURSIVELY ? view->expand_toggle_rec()
                                                         : view->expand_toggle();
                    }

                break;
            }

            case Event_SlotDropped::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto _event = reinterpret_cast<Event_SlotDropped*>(event);
                Slot* tail = _event->data.first;
                Slot* head = _event->data.second;
                ASSERT(head != tail);
                if ( tail->order() == SlotFlag_ORDER_2ND )
                {
                    if ( head->order() == SlotFlag_ORDER_2ND )
                    {
                        LOG_ERROR("Nodable", "Unable to connect incompatible edges\n");
                        break; // but if it still the case, that's because edges are incompatible
                    }
                    LOG_VERBOSE("Nodable", "Swapping edges to try to connect them\n");
                    std::swap(tail, head);
                }
                DirectedEdge edge(tail, head);
                auto cmd = std::make_shared<Cmd_ConnectEdge>(edge);
                curr_file_history->push_command(cmd);

                break;
            }

            case Event_DeleteEdge::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto command = std::make_shared<Cmd_DeleteEdge>( static_cast<Event_DeleteEdge*>(event) );
                curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(command));
                break;
            }

            case Event_SlotDisconnectAll::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto _event = static_cast<Event_SlotDisconnectAll*>(event);
                Slot* slot = _event->data.first;

                auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Edges");
                Graph* graph = _event->data.first->node->graph();
                for(Slot* adjacent_slot: slot->adjacent() )
                {
                    auto each_cmd = std::make_shared<Cmd_DeleteEdge>(DirectedEdge{slot, adjacent_slot}, graph );
                    cmd_grp->push_cmd( std::static_pointer_cast<AbstractCommand>(each_cmd) );
                }
                curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(cmd_grp));
                break;
            }

            case Event_CreateNode::id:
            {
                auto _event = reinterpret_cast<Event_CreateNode*>(event);
                Graph* graph = _event->data.graph;

                // 1) create the node
                if ( !graph->root() )
                {
                    LOG_ERROR("Nodable", "Unable to create_new child_node, no root found on this graph.\n");
                    continue;
                }

                Node* new_node  = graph->create_node(_event->data.node_type, _event->data.node_signature );

                // Insert an end of line and end of instruction
                switch ( _event->data.node_type )
                {
                    case CreateNodeType_BLOCK_CONDITION:
                    case CreateNodeType_BLOCK_FOR_LOOP:
                    case CreateNodeType_BLOCK_WHILE_LOOP:
                    case CreateNodeType_BLOCK_SCOPE:
                    case CreateNodeType_BLOCK_ENTRY_POINT:
                        new_node->set_suffix( Token::s_end_of_line );
                        break;
                    case CreateNodeType_VARIABLE_BOOLEAN:
                    case CreateNodeType_VARIABLE_DOUBLE:
                    case CreateNodeType_VARIABLE_INTEGER:
                    case CreateNodeType_VARIABLE_STRING:
                        new_node->set_suffix( Token::s_end_of_instruction );
                        break;
                    case CreateNodeType_LITERAL_BOOLEAN:
                    case CreateNodeType_LITERAL_DOUBLE:
                    case CreateNodeType_LITERAL_INTEGER:
                    case CreateNodeType_LITERAL_STRING:
                    case CreateNodeType_FUNCTION:
                        break;
                }

                // 2) handle connections
                SlotView* slot_view = _event->data.active_slotview;
                if ( !slot_view)
                {
                    // Experimental: we try to connect a parent-less child
                    if ( !graph->is_root( new_node ) && m_config->has_flags( ConfigFlag_EXPERIMENTAL_GRAPH_AUTOCOMPLETION ) )
                    {
                        Scope* root_scope = graph->root()->internal_scope();
                        VERIFY( root_scope != nullptr, "inner main_scope is expected on a root child_node");
                        root_scope->push_back(new_node);
                    }
                }
                else
                {
                    SlotFlags             complementary_flags = switch_order(slot_view->slot->type_and_order());
                    const TypeDescriptor* type                = slot_view->property()->get_type();
                    Slot*                 complementary_slot  = new_node->find_slot_by_property_type(complementary_flags, type);

                    if ( complementary_slot )
                    {
                        // TODO: this case should not happens, instead we should check ahead of time whether or not this not can be attached
                        LOG_ERROR( "GraphView", "unable to connect this child_node" );
                    }
                    if ( complementary_slot )
                    {
                        Slot* out = slot_view->slot;
                        Slot* in  = complementary_slot;

                        if ( out->has_flags( SlotFlag_ORDER_2ND ) )
                            std::swap( out, in );

                        graph->connect(out, in, ConnectFlag_ALLOW_SIDE_EFFECTS );

                        // Ensure has a "\n" when connecting using CODEFLOW (to split lines)
                        if ( Utils::is_instruction( out->node ) && out->type() == SlotFlag_TYPE_FLOW )
                        {
                            Token& token = out->node->suffix();
                            std::string buffer = token.string();
                            if ( buffer.empty() || std::find(buffer.rbegin(), buffer.rend(), '\n') == buffer.rend() )
                                token.suffix_push_back("\n");
                        }
                    }
                }

                // set new_node's view position, select it
                if ( auto view = new_node->get_component<NodeView>() )
                {
                    view->xform()->set_position(_event->data.desired_screen_pos, WORLD_SPACE);
                    graph->view()->set_selected({view});
                }
                break;
            }

            default:
            {
                LOG_VERBOSE("App", "Ignoring and event, this case is not handled\n");
            }
        }

        // clean memory
        delete event;
    }
}

void Nodable::shutdown()
{
    LOG_VERBOSE("ndbl::Nodable", "shutdown ...\n");

    for( File* each_file : m_loaded_files )
    {
        LOG_VERBOSE("ndbl::App", "Delete file %s ...\n", each_file->path.c_str());
        delete each_file;
    }

    // shutdown managers & co.
    shutdown_interpreter(m_interpreter);
    shutdown_node_factory(m_node_factory);
    shutdown_component_factory(m_component_factory);
    shutdown_language(m_language);
    m_view->shutdown();
    m_base_app.shutdown();
    shutdown_config(m_config);

    delete m_view;

    LOG_VERBOSE("ndbl::Nodable", "shutdown " OK "\n");
}

File* Nodable::open_asset_file(const tools::Path& _path)
{
    if ( _path.is_absolute() )
        return open_file(_path);

    Path path = _path;
    App::make_absolute(path);
    return open_file(path);
}

File* Nodable::open_file(const tools::Path& _path)
{
    File* file = new File();

    if ( File::read( *file, _path ) )
    {
        return add_file(file);
    }

    delete file;
    LOG_ERROR("File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
    return nullptr;
}

File* Nodable::add_file( File* _file)
{
    VERIFY(_file, "File is nullptr");
    m_loaded_files.push_back( _file );
    m_current_file = _file;
    get_event_manager()->dispatch( EventID_FILE_OPENED );
    return _file;
}

void Nodable::save_file( File* _file) const
{
    VERIFY(_file, "file must be defined");

	if ( !File::write(*_file, _file->path) )
    {
        LOG_ERROR("ndbl::App", "Unable to save %s (%s)\n", _file->filename().c_str(), _file->path.c_str());
        return;
    }
    LOG_MESSAGE("ndbl::App", "File saved: %s\n", _file->path.c_str());
}

void Nodable::save_file_as(File* _file, const tools::Path& _path) const
{
    if ( !File::write(*_file, _path) )
    {
        LOG_ERROR("ndbl::App", "Unable to save %s (%s)\n", _path.filename().c_str(), _path.c_str());
        return;
    }
    LOG_MESSAGE("ndbl::App", "File saved: %s\n", _path.c_str());
}

void Nodable::close_file( File* _file)
{
    // Find and delete the file
    VERIFY(_file, "Cannot close a nullptr File!");
    auto it = std::find(m_loaded_files.begin(), m_loaded_files.end(), _file);
    VERIFY(it != m_loaded_files.end(), "Unable to find the file in the loaded_files");
    it = m_loaded_files.erase(it);
    delete _file;

    // Switch to the next file if possible
    if ( it != m_loaded_files.end() )
    {
        m_current_file = *it;
    }
    else
    {
        m_current_file = nullptr;
    }
}

bool Nodable::compile_and_load_program() const
{
    if (!m_current_file)
    {
        return false;
    }

    Compiler compiler{};
    auto asm_code = compiler.compile_syntax_tree(&m_current_file->graph());
    if (!asm_code)
    {
        return false;
    }

    m_interpreter->release_program();
    bool loaded = m_interpreter->load_program(asm_code);
    return loaded;
}

void Nodable::run_program()
{
    if (compile_and_load_program() )
    {
        m_interpreter->run_program();
    }
}

void Nodable::debug_program()
{
    if (compile_and_load_program() )
    {
        m_interpreter->debug_program();
    }
}

void Nodable::step_over_program()
{
    m_interpreter->debug_step_over();
    GraphView* graph_view = m_current_file->graph().view();

    if (!m_interpreter->is_there_a_next_instr() )
    {
        graph_view->set_selected({}, SelectionMode_REPLACE);
        return;
    }

    const Node* next_node = m_interpreter->get_next_node();
    if ( !next_node )
        return;

    NodeView* view = next_node->get_component<NodeView>();
    graph_view->set_selected({view});
}

void Nodable::stop_program()
{
    m_interpreter->stop_program();
}

void Nodable::reset_program()
{
    if(!m_current_file) return;

    if (m_interpreter->is_program_running() )
    {
        m_interpreter->stop_program();
    }

    // n.b. nodable is still text oriented
    m_current_file->set_graph_dirty();
}

File*Nodable::new_file()
{
    m_untitled_file_count++;

    string32 name;
    name.append_fmt("Untitled_%i.cpp", m_untitled_file_count);
    auto* file = new File();
    file->path = name.c_str();

    return add_file(file);
}

NodableView* Nodable::get_view() const
{
    return reinterpret_cast<NodableView*>(m_view);
}

bool Nodable::should_stop() const
{
    return m_base_app.should_stop();
}

void Nodable::draw()
{
    // we have our own view, so we bypass base App
    // m_base_app.draw();
    m_view->draw();
}

void Nodable::set_current_file(File* file)
{
    if ( m_current_file == nullptr )
    {
        m_current_file = file;
        return;
    }

    // TODO:
    //  - unload current file?
    //  - keep the last N files loaded?
    //  - save graph to a temp file to restore it later without using memory and altering original source file?
    // close_file(m_current_file); ??

    m_current_file = file;
}
