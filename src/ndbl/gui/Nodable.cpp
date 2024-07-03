#include "Nodable.h"

#include <algorithm>

#include "tools/core/assertions.h"
#include "tools/core/system.h"
#include "tools/core/EventManager.h"

#include "ndbl/core/InvokableNode.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/Slot.h"
#include "ndbl/core/Interpreter.h"
#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/ComponentFactory.h"

#include "commands/Cmd_ConnectEdge.h"
#include "commands/Cmd_DisconnectEdge.h"
#include "commands/Cmd_Group.h"

#include "Condition.h"
#include "Config.h"
#include "Event.h"
#include "File.h"
#include "GraphView.h"
#include "NodableView.h"
#include "NodeView.h"
#include "SlotView.h"

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

    LOG_VERBOSE("ndbl::Nodable", "init_ex OK\n");
}

void Nodable::update()
{
    m_base_app.update();

    // 1. Update current file
    if (m_current_file && !m_interpreter->is_program_running())
    {
        //
        // When history is dirty we update the graph from the text.
        // (By default undo/redo are text-based only, if hybrid_history is ON, the behavior is different
        if (m_current_file->history.is_dirty && !m_config->has_flags(ConfigFlag_EXPERIMENTAL_HYBRID_HISTORY) )
        {
            m_current_file->update_graph_from_text(m_config->isolation);
            m_current_file->history.is_dirty = false;
        }
        // Run the main update loop for the file
        m_current_file->update(m_config->isolation );
    }

    // 2. Handle events

    // Nodable events
    IEvent*       event = nullptr;
    EventManager* event_manager     = get_event_manager();
    GraphView*    graph_view        = m_current_file ? m_current_file->get_graph().get_view() : nullptr; // TODO: should be included in the event
    History*      curr_file_history = m_current_file ? &m_current_file->history : nullptr; // TODO: should be included in the event
    while( (event = event_manager->poll_event()) )
    {
        switch ( event->id )
        {
            case EventID_TOGGLE_ISOLATION_FLAGS:
            {
                m_config->isolation = ~m_config->isolation;
                if(m_current_file)
                {
                    m_current_file->update_graph_from_text(m_config->isolation );
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
                std::string path;
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
                if (!m_current_file) break;
                std::string path;
                if( m_view->pick_file_path(path, AppView::DIALOG_SaveAs))
                {
                    save_file_as(path);
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
                    std::string path;
                    if( m_view->pick_file_path(path, AppView::DIALOG_SaveAs))
                    {
                        save_file_as(path);
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
                EXPECT(graph_view, "a graph_view is required");
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
                ASSERT(m_current_file != nullptr )
                m_current_file->view.clear_overlay();
                m_current_file->view.refresh_overlay(Condition_ENABLE_IF_HAS_NO_SELECTION );
                break;
            }
            case Event_DeleteNode::id:
            {
                // TODO: store a ref to the view in the event, use selected as fallback if not present
                auto& selected = graph_view->get_selected();
                if ( !selected.empty() && !ImGui::IsAnyItemFocused() )
                {
                    Node* selected_node = selected[0]->get_owner();
                    selected_node->set_flags(NodeFlag_TO_DELETE);
                }
                break;
            }

            case Event_ArrangeNode::id:
            {
                // TODO: views should be included in the event
                for(auto& each_selected_view: graph_view->get_selected())
                {
                    each_selected_view->arrange_recursively();
                };
                break;
            }

            case Event_SelectNext::id:
            {
                // TODO: views should be included in the event
                auto& selected = graph_view->get_selected();
                if (selected.empty()) break;
                std::vector<Node*> successors = selected[0]->get_owner()->successors();
                if (!successors.empty())
                    if (NodeView* successor_view = successors.front()->get_component<NodeView>() )
                        graph_view->set_selected({successor_view}, SelectionMode_REPLACE); EXPECT(false, "not implemented for multi-selection")
                break;
            }

            case Event_ToggleFolding::id:
            {
                // TODO: views should be included in the event
                auto& selected = graph_view->get_selected();
                if ( selected.empty() ) break;
                auto _event = reinterpret_cast<Event_ToggleFolding*>(event);
                _event->data.mode == RECURSIVELY ? selected[0]->expand_toggle_rec()
                                                 : selected[0]->expand_toggle();
                break;
            }

            case Event_SlotDropped::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto _event = reinterpret_cast<Event_SlotDropped*>(event);
                Slot* tail = _event->data.first;
                Slot* head = _event->data.second;
                if ( tail->order() == SlotFlag_ORDER_SECOND )
                {
                    if ( head->order() == SlotFlag_ORDER_SECOND )
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
                auto* _event = reinterpret_cast<Event_DeleteEdge*>(event);
                DirectedEdge edge{ _event->data.first, _event->data.second };
                Graph* graph = _event->data.first->get_node()->get_parent_graph();
                auto command = std::make_shared<Cmd_DisconnectEdge>(edge, graph );
                curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(command));
                break;
            }

            case Event_SlotDisconnected::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto _event = reinterpret_cast<Event_SlotDisconnected*>(event);
                Slot* slot = _event->data.first;

                auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Edges");
                Graph* graph = _event->data.first->get_node()->get_parent_graph();
                for( const auto& adjacent_slot: slot->adjacent() )
                {
                    DirectedEdge edge{slot, adjacent_slot};
                    auto each_cmd = std::make_shared<Cmd_DisconnectEdge>(edge, graph );
                    cmd_grp->push_cmd( std::static_pointer_cast<AbstractCommand>(each_cmd) );
                }
                curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(cmd_grp));

                break;
            }

            case Event_CreateNode::id:
            {
                auto _event = reinterpret_cast<Event_CreateNode*>(event);

                // 1) create the node
                 if ( !_event->data.graph->get_root() )
                {
                    LOG_ERROR("Nodable", "Unable to create node, no root found on this graph.\n");
                    continue;
                }

                Node* new_node  = _event->data.graph->create_node( _event->data.node_type, _event->data.node_signature );

                if ( !_event->data.active_slotview )
                {
                    // Insert an end of line and end of instruction
                    switch ( _event->data.node_type )
                    {
                        case CreateNodeType_BLOCK_CONDITION:
                        case CreateNodeType_BLOCK_FOR_LOOP:
                        case CreateNodeType_BLOCK_WHILE_LOOP:
                        case CreateNodeType_BLOCK_SCOPE:
                        case CreateNodeType_BLOCK_PROGRAM:
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
                        case CreateNodeType_INVOKABLE:
                            break;
                    }
                }
                // 2) handle connections
                if ( !_event->data.active_slotview )
                {
                    // Experimental: we try to connect a parent-less child
                    Node* root = _event->data.graph->get_root();
                    if (new_node != root && m_config->has_flags( ConfigFlag_EXPERIMENTAL_GRAPH_AUTOCOMPLETION ) )
                    {
                        _event->data.graph->connect(
                            *root->find_slot(SlotFlag_CHILD),
                            *new_node->find_slot(SlotFlag_PARENT),
                            ConnectFlag_ALLOW_SIDE_EFFECTS
                        );
                    }
                }
                else
                {
                    Slot* complementary_slot = new_node->find_slot_by_property_type(
                            get_complementary_flags(_event->data.active_slotview->get_slot().type_and_order() ),
                            _event->data.active_slotview->get_property()->get_type() );

                    if ( !complementary_slot )
                    {
                        // TODO: this case should not happens, instead we should check ahead of time whether or not this not can be attached
                        LOG_ERROR( "GraphView", "unable to connect this node" )
                    }
                    else
                    {
                        Slot* out = &_event->data.active_slotview->get_slot();
                        Slot* in = complementary_slot;

                        if ( out->has_flags( SlotFlag_ORDER_SECOND ) ) std::swap( out, in );

                        _event->data.graph->connect( *out, *in, ConnectFlag_ALLOW_SIDE_EFFECTS );
                    }
                }

                // set new_node's view position, select it
                if ( auto view = new_node->get_component<NodeView>() )
                {
                    view->set_pos(_event->data.desired_screen_pos, SCREEN_SPACE);
                    _event->data.graph->get_view()->set_selected({view});
                }
                break;
            }

            default:
            {
                LOG_VERBOSE("App", "Ignoring and event, this case is not handled\n")
            }
        }
    }
}

void Nodable::shutdown()
{
    LOG_VERBOSE("ndbl::Nodable", "shutdown ...\n");

    for( File* each_file : m_loaded_files )
    {
        LOG_VERBOSE("ndbl::App", "Delete file %s ...\n", each_file->path.c_str())
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

File* Nodable::open_asset_file(const std::filesystem::path& _path)
{
    std::filesystem::path absolute_path = App::asset_path(_path);
    return open_file(absolute_path);
}

File* Nodable::open_file(const std::filesystem::path& _path)
{
    File* file = new File();

    if ( File::read( *file, _path ) )
    {
        add_file(file);
        file->update_graph_from_text( m_config->isolation );
        return file;
    }

    delete file;
    LOG_ERROR("File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
    return nullptr;
}

File* Nodable::add_file( File* _file)
{
    EXPECT(_file, "File is nullptr");
    m_loaded_files.push_back( _file );
    m_current_file = _file;
    get_event_manager()->dispatch( EventID_FILE_OPENED );
    return _file;
}

void Nodable::save_file( File* _file) const
{
    EXPECT(_file,"file must be defined");

	if ( !File::write(*_file, _file->path) )
    {
        LOG_ERROR("ndbl::App", "Unable to save %s (%s)\n", _file->filename().c_str(), _file->path.c_str());
        return;
    }
    LOG_MESSAGE("ndbl::App", "File saved: %s\n", _file->path.c_str());
}

void Nodable::save_file_as(const std::filesystem::path& _path) const
{
    if ( !File::write(*m_current_file, _path) )
    {
        LOG_ERROR("ndbl::App", "Unable to save %s (%s)\n", _path.filename().c_str(), _path.c_str());
        return;
    }
    LOG_MESSAGE("ndbl::App", "File saved: %s\n", _path.c_str());
}

void Nodable::close_file( File* _file)
{
    // Find and delete the file
    EXPECT(_file, "Cannot close a nullptr File!");
    auto it = std::find(m_loaded_files.begin(), m_loaded_files.end(), _file);
    EXPECT(it != m_loaded_files.end(), "Unable to find the file in the loaded_files");
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
    auto asm_code = compiler.compile_syntax_tree(&m_current_file->get_graph());
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
    GraphView* graph_view = m_current_file->get_graph().get_view();

    if (!m_interpreter->is_there_a_next_instr() )
    {
        graph_view->set_selected({}, SelectionMode_REPLACE);
        return;
    }

    Node* next_node = m_interpreter->get_next_node();
    if ( !next_node ) return;

    auto view = next_node->get_component<NodeView>();
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

    m_current_file->update_graph_from_text(m_config->isolation );
}

File*Nodable::new_file()
{
    m_untitled_file_count++;

    string32 name;
    name.append_fmt("Untitled_%i.cpp", m_untitled_file_count);
    auto* file = new File();
    file->path = name.c_str();
    file->update_graph_from_text();

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
