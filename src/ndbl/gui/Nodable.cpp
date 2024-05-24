#include "Nodable.h"

#include <algorithm>

#include "tools/core/assertions.h"
#include "tools/core/system.h"
#include "tools/gui/EventManager.h"
#include "tools/gui/Config.h"
#include "ndbl/core/InvokableComponent.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/Slot.h"
#include "ndbl/core/VariableNode.h"
#include "ndbl/core/VirtualMachine.h"

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
#include "Physics.h"
#include "SlotView.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

void Nodable::init()
{
    LOG_VERBOSE("ndbl::Nodable", "init ...\n");

    type_register::log_statistics();

    BaseAppFlags flags = BaseAppFlag_SKIP_VIEW    // we want to init/shutdown manually
                       | BaseAppFlag_SKIP_CONFIG; // (same)
    view = new NodableView(this);
    BaseApp::init( view, flags );

    Config* cfg = init_config();
    init_language();
    init_virtual_machine();
    init_node_factory();
    view->init();

    get_node_factory()->override_post_process_fct( [cfg]( PoolID<Node> node ) -> void {
        // Code executed after node instantiation

        // add a view with physics
        auto* pool = get_pool();
        PoolID<NodeView> new_view_id = pool->create<NodeView>();
        PoolID<Physics> physics_id = pool->create<Physics>( new_view_id );
        node->add_component( new_view_id );
        node->add_component( physics_id );

        // Set fill_color
        Vec4* fill_color;
        if ( extends<VariableNode>( node.get() ) )
        {
            fill_color = &cfg->ui_node_variableColor;
        }
        else if ( node->has_component<InvokableComponent>() )
        {
            fill_color = &cfg->ui_node_invokableColor;
        }
        else if ( node->is_instruction() )
        {
            fill_color = &cfg->ui_node_instructionColor;
        }
        else if ( extends<LiteralNode>( node.get() ) )
        {
            fill_color = &cfg->ui_node_literalColor;
        }
        else if ( extends<IConditional>( node.get() ) )
        {
            fill_color = &cfg->ui_node_condStructColor;
        }
        else
        {
            fill_color = &cfg->ui_node_fillColor;
        }
        new_view_id->set_color( fill_color );
    });

    LOG_VERBOSE("ndbl::Nodable", "init OK\n");
}

void Nodable::update()
{
    BaseApp::update();

    LOG_VERBOSE("ndbl::App", "update ...\n");
    Config* cfg = get_config();

    // 1. Update current file
    if (current_file && !get_virtual_machine()->is_program_running())
    {
        //
        // When history is dirty we update the graph from the text.
        // (By default undo/redo are text-based only, if hybrid_history is ON, the behavior is different
        if ( current_file->history.is_dirty && !cfg->experimental_hybrid_history )
        {
            current_file->update_graph_from_text( cfg->isolation);
            current_file->history.is_dirty = false;
        }
        // Run the main update loop for the file
        current_file->update( cfg->isolation );
    }

    // 2. Handle events

    // Nodable events
    auto       selected_view       = NodeView::get_selected();
    GraphView* graph_view          = current_file ? current_file->graph_view : nullptr;
    History*   curr_file_history   = current_file ? &current_file->history : nullptr;

    IEvent* event = nullptr;
    while( (event = view->event_manager.poll_event()) )
    {
        switch ( event->id )
        {
            case EventID_TOGGLE_ISOLATION_FLAGS:
            {
                cfg->isolation = ~cfg->isolation;
                if(current_file)
                {
                    current_file->update_graph_from_text( cfg->isolation );
                }
                break;
            }

            case EventID_REQUEST_EXIT:
            {
                should_stop = true;
                break;
            }

            case EventID_FILE_CLOSE:
            {
                if(current_file) close_file(current_file);
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
                if( view->pick_file_path(path, AppView::DIALOG_Browse))
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
                if (current_file)
                {
                    std::string path;
                    if( view->pick_file_path(path, AppView::DIALOG_SaveAs))
                    {
                        save_file_as(path);
                        break;
                    }
                }
                break;
            }

            case EventID_FILE_SAVE:
            {
                if (!current_file) break;
                if( !current_file->path.empty())
                {
                    save_file(current_file);
                }
                else
                {
                    std::string path;
                    if( view->pick_file_path(path, AppView::DIALOG_SaveAs))
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
                    view->show_splashscreen = _event->data.visible;
                }
                break;
            }

            case Event_FrameSelection::id:
            {
                auto _event = reinterpret_cast<Event_FrameSelection*>( event );
                EXPECT(graph_view, "a graph_view is required");
                graph_view->frame(_event->data.mode);
                break;
            }

            case Event_SelectionChange::id:
            {
                auto _event = reinterpret_cast<Event_SelectionChange*>( event );

                Condition_ condition = _event->data.new_selection ? Condition_ENABLE_IF_HAS_SELECTION
                                                                  : Condition_ENABLE_IF_HAS_NO_SELECTION;
                current_file->view.clear_overlay();
                current_file->view.refresh_overlay( condition );
                break;
            }
            case EventID_FILE_OPENED:
            {
                current_file->view.clear_overlay();
                current_file->view.refresh_overlay( Condition_ENABLE_IF_HAS_NO_SELECTION );
                break;
            }
            case Event_DeleteNode::id:
            {
                // TODO: store a ref to the view in the event, use selected as fallback if not present

                if ( selected_view && !ImGui::IsAnyItemFocused() )
                {
                    Node* selected_node = selected_view->get_owner().get();
                    selected_node->flagged_to_delete = true;
                }
                break;
            }

            case Event_ArrangeNode::id:
            {
                if ( selected_view ) selected_view->arrange_recursively();
                break;
            }

            case Event_SelectNext::id:
            {
                // TODO: store a ref to the view in the event, use selected as fallback if not present

                if (!selected_view) break;
                std::vector<PoolID<Node>> successors = selected_view->get_owner()->successors();
                if (!successors.empty())
                {
                    if (PoolID<NodeView> successor_view = successors.front()->get_component<NodeView>() )
                    {
                        NodeView::set_selected(successor_view);
                    }
                }
                break;
            }

            case Event_ToggleFolding::id:
            {
                if ( !selected_view ) break;
                auto _event = reinterpret_cast<Event_ToggleFolding*>(event);
                _event->data.mode == RECURSIVELY ? selected_view->expand_toggle_rec()
                                                  : selected_view->expand_toggle();
                break;
            }

            case Event_SlotDropped::id:
            {
                auto _event = reinterpret_cast<Event_SlotDropped*>(event);
                SlotRef tail = _event->data.first;
                SlotRef head = _event->data.second;
                if (tail.flags & SlotFlag_ORDER_SECOND ) std::swap(tail, head); // guarantee src to be the output
                DirectedEdge edge(tail, head);
                auto cmd = std::make_shared<Cmd_ConnectEdge>(edge);
                curr_file_history->push_command(cmd);

                break;
            }

            case Event_SlotDisconnected::id:
            {
                auto _event = reinterpret_cast<Event_SlotDisconnected*>(event);
                SlotRef slot = _event->data.first;

                auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Edges");
                for( const auto& adjacent_slot: slot->adjacent() )
                {
                    DirectedEdge edge{slot, adjacent_slot};
                    auto each_cmd = std::make_shared<Cmd_DisconnectEdge>(edge);
                    cmd_grp->push_cmd( std::static_pointer_cast<AbstractCommand>(each_cmd) );
                }
                curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(cmd_grp));

                break;
            }

            case Event_CreateNode::id:
            {
                auto _event = reinterpret_cast<Event_CreateNode*>(event);

                // 1) create the node
                Graph* graph = current_file->graph;

                 if ( !graph->get_root() )
                {
                    LOG_ERROR("Nodable", "Unable to create node, no root found on this graph.\n");
                    continue;
                }

                PoolID<Node> new_node_id  = graph->create_node( _event->data.node_type, _event->data.node_signature );

                if ( !_event->data.dragged_slot )
                {
                    // Insert an end of line and end of instruction
                    switch ( _event->data.node_type )
                    {
                        case NodeType_BLOCK_CONDITION:
                        case NodeType_BLOCK_FOR_LOOP:
                        case NodeType_BLOCK_WHILE_LOOP:
                        case NodeType_BLOCK_SCOPE:
                        case NodeType_BLOCK_PROGRAM:
                            new_node_id->after_token = Token::s_end_of_line;
                            break;
                        case NodeType_VARIABLE_BOOLEAN:
                        case NodeType_VARIABLE_DOUBLE:
                        case NodeType_VARIABLE_INTEGER:
                        case NodeType_VARIABLE_STRING:
                            new_node_id->after_token = Token::s_end_of_instruction;
                            break;
                        case NodeType_LITERAL_BOOLEAN:
                        case NodeType_LITERAL_DOUBLE:
                        case NodeType_LITERAL_INTEGER:
                        case NodeType_LITERAL_STRING:
                        case NodeType_INVOKABLE:
                            break;
                    }
                }
                // 2) handle connections
                if ( !_event->data.dragged_slot )
                {
                    // Experimental: we try to connect a parent-less child
                    PoolID<Node> root = graph->get_root();
                    if ( new_node_id != root && cfg->experimental_graph_autocompletion )
                    {
                        graph->connect(
                            *root->find_slot(SlotFlag_CHILD),
                            *new_node_id->find_slot(SlotFlag_PARENT),
                            ConnectFlag_ALLOW_SIDE_EFFECTS
                        );
                    }
                }
                else
                {
                    Slot* complementary_slot = new_node_id->find_slot_by_property_type(
                            get_complementary_flags( _event->data.dragged_slot->slot().static_flags() ),
                            _event->data.dragged_slot->get_property()->get_type() );

                    if ( !complementary_slot )
                    {
                        // TODO: this case should not happens, instead we should check ahead of time whether or not this not can be attached
                        LOG_ERROR( "GraphView", "unable to connect this node" )
                    }
                    else
                    {
                        Slot* out = &_event->data.dragged_slot->slot();
                        Slot* in = complementary_slot;

                        if ( out->has_flags( SlotFlag_ORDER_SECOND ) ) std::swap( out, in );

                        _event->data.graph->connect( *out, *in, ConnectFlag_ALLOW_SIDE_EFFECTS );
                    }
                }

                // set new_node's view position, select it
                if ( auto view = new_node_id->get_component<NodeView>() )
                {
                    view->position( _event->data.node_view_local_pos, PARENT_SPACE );
                    NodeView::set_selected( view );
                }
                break;
            }

            default:
            {
                LOG_VERBOSE("App", "Ignoring and event, this case is not handled\n")
            }
        }
    }
    LOG_VERBOSE("ndbl::App", "update " OK "\n");
}

void Nodable::shutdown()
{
    LOG_VERBOSE("ndbl::Nodable", "shutdown ...\n");

    for( File* each_file : m_loaded_files )
    {
        LOG_VERBOSE("ndbl::App", "Delete file %s ...\n", each_file->path.c_str())
        delete each_file;
    }

    shutdown_config();
    shutdown_virtual_machine();
    shutdown_node_factory();
    shutdown_language();
    view->shutdown();
    delete view;

    // Base class
    BaseApp::shutdown();

    LOG_VERBOSE("ndbl::Nodable", "shutdown " OK "\n");
}

File* Nodable::open_asset_file(const std::filesystem::path& _path)
{
    std::filesystem::path absolute_path = asset_path(_path);
    return open_file(absolute_path);
}

File* Nodable::open_file(const std::filesystem::path& _path)
{
    auto file = new File();
    Config* cfg = get_config();
    if ( !File::read( *file, _path ) )
    {
        LOG_ERROR("File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
        delete file;
        return nullptr;
    }
    add_file(file);
    file->update_graph_from_text( cfg->isolation);
    return file;
}

File*Nodable::add_file( File* _file)
{
    EXPECT(_file, "File is nullptr");
    m_loaded_files.push_back( _file );
    current_file = _file;
    view->event_manager.dispatch( EventID_FILE_OPENED );
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
    if ( !File::write(*current_file, _path) )
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
        current_file = *it;
    }
    else
    {
        current_file = nullptr;
    }
}

bool Nodable::compile_and_load_program()
{
    if (!current_file || !current_file->graph)
    {
        return false;
    }

    assembly::Compiler compiler{};
    auto asm_code = compiler.compile_syntax_tree(current_file->graph);
    if (!asm_code)
    {
        return false;
    }

    VirtualMachine* virtual_machine = get_virtual_machine();
    virtual_machine->release_program();
    bool loaded = virtual_machine->load_program(asm_code);
    return loaded;
}

void Nodable::run_program()
{
    if (compile_and_load_program() )
    {
        get_virtual_machine()->run_program();
    }
}

void Nodable::debug_program()
{
    if (compile_and_load_program() )
    {
        get_virtual_machine()->debug_program();
    }
}

void Nodable::step_over_program()
{
    VirtualMachine* virtual_machine = get_virtual_machine();
    virtual_machine->step_over();
    if (!virtual_machine->is_there_a_next_instr() )
    {
        NodeView::set_selected({});
        return;
    }

    const Node* next_node = virtual_machine->get_next_node();
    if ( !next_node ) return;

    if( PoolID<NodeView> next_node_view = next_node->get_component<NodeView>() )
    {
        NodeView::set_selected( next_node_view );
    }
}

void Nodable::stop_program()
{
   get_virtual_machine()->stop_program();
}

void Nodable::reset_program()
{
    if(!current_file) return;

    Config* cfg = get_config();
    VirtualMachine* virtual_machine = get_virtual_machine();
    if (virtual_machine->is_program_running() )
    {
        virtual_machine->stop_program();
    }

    current_file->update_graph_from_text( cfg->isolation );
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
