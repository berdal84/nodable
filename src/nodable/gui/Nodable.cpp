#include "Nodable.h"

#include "Action.h"
#include "Condition.h"
#include "Event.h"
#include "GraphView.h"
#include "HybridFile.h"
#include "HybridFileView.h"
#include "NodableView.h"
#include "NodeView.h"
#include "Physics.h"
#include "SlotView.h"
#include "commands/Cmd_ConnectEdge.h"
#include "commands/Cmd_DisconnectEdge.h"
#include "commands/Cmd_Group.h"
#include "core/DataAccess.h"
#include "core/InvokableComponent.h"
#include "core/LiteralNode.h"
#include "core/NodeUtils.h"
#include "core/Slot.h"
#include "core/VariableNode.h"
#include "fw/core/assertions.h"
#include "fw/core/system.h"
#include "fw/gui/EventManager.h"
#include <algorithm>

using namespace ndbl;
using namespace fw;
using fw::View;

Nodable *Nodable::s_instance = nullptr;

template<typename T>
static fw::func_type* create_variable_node_signature()
{ return fw::func_type_builder<T(T)>::with_id("variable"); }

template<typename T>
static fw::func_type* create_literal_node_signature()
{ return fw::func_type_builder<T(/*void*/)>::with_id("literal"); }

Nodable::Nodable()
    : App(config.common, new NodableView(this) )
    , current_file(nullptr)
    , virtual_machine()
{
    LOG_VERBOSE("ndbl::App", "Constructor ...\n");

    fw::type_register::log_statistics();

    // set this instance as s_instance to access it via App::get_instance()
    FW_EXPECT(s_instance == nullptr, "Can't create two concurrent App. Delete first instance.");
    s_instance = this;

    // Bind methods to framework events
    LOG_VERBOSE("ndbl::App", "Binding framework ...\n");

    node_factory.override_post_process_fct( [&]( PoolID<Node> node ) -> void {
        // Code executed after node instantiation

        // add a view with physics
        auto *pool = Pool::get_pool();
        PoolID<NodeView> new_view_id = pool->create<NodeView>();
        PoolID<Physics> physics_id = pool->create<Physics>( new_view_id );
        node->add_component( new_view_id );
        node->add_component( physics_id );

        // Set fill_color
        ImVec4* fill_color;
        if ( fw::extends<VariableNode>( node.get() ) )
        {
            fill_color = &config.ui_node_variableColor;
        }
        else if ( node->has_component<InvokableComponent>() )
        {
            fill_color = &config.ui_node_invokableColor;
        }
        else if ( node->is_instruction() )
        {
            fill_color = &config.ui_node_instructionColor;
        }
        else if ( fw::extends<LiteralNode>( node.get() ) )
        {
            fill_color = &config.ui_node_literalColor;
        }
        else if ( fw::extends<IConditional>( node.get() ) )
        {
            fill_color = &config.ui_node_condStructColor;
        }
        else
        {
            fill_color = &config.ui_node_fillColor;
        }
        new_view_id->set_color( fill_color );
    } );

    LOG_VERBOSE("ndbl::App", "Constructor " OK "\n");
}

Nodable::~Nodable()
{
    delete m_view;
    s_instance = nullptr;
    LOG_VERBOSE("ndbl::App", "Destructor " OK "\n");
}

bool Nodable::on_init()
{
    LOG_VERBOSE("ndbl::App", "on_init ...\n");

    fw::Pool::init();

    // Bind commands to shortcuts
    event_manager.bind<Action_DeleteNode>( "Delete", Shortcut{ SDLK_DELETE, KMOD_NONE } );
    event_manager.bind<Action_ArrangeNode>( "Arrange", Shortcut{ SDLK_a, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    event_manager.bind<Action_ToggleFolding>( "Fold", Shortcut{ SDLK_x, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    event_manager.bind<Action_SelectNext>( "Next", Shortcut{ SDLK_n, KMOD_NONE } );
    event_manager.bind<Action_FileSave>( ICON_FA_SAVE " Save", Shortcut{ SDLK_s, KMOD_CTRL } );
    event_manager.bind<Action_FileSaveAs>( ICON_FA_SAVE " Save as", Shortcut{ SDLK_s, KMOD_CTRL } );
    event_manager.bind<Action_FileClose>( ICON_FA_TIMES "  Close", Shortcut{ SDLK_w, KMOD_CTRL } );
    event_manager.bind<Action_FileBrowse>( ICON_FA_FOLDER_OPEN " Open", Shortcut{ SDLK_o, KMOD_CTRL } );
    event_manager.bind<Action_FileNew>( ICON_FA_FILE " New", Shortcut{ SDLK_n, KMOD_CTRL } );
    event_manager.bind<Action_ShowWindow>( "Splashscreen", Shortcut{ SDLK_F1 }, EventPayload_ShowWindow{ "splashscreen" } );
    event_manager.bind<Action_Exit>( ICON_FA_SIGN_OUT_ALT " Exit", Shortcut{ SDLK_F4, KMOD_ALT } );
    event_manager.bind<Action_Undo>( "Undo", Shortcut{ SDLK_z, KMOD_CTRL } );
    event_manager.bind<Action_Redo>( "Redo", Shortcut{ SDLK_y, KMOD_CTRL } );
    event_manager.bind<Action_ToggleIsolate>( "Isolate", Shortcut{ SDLK_i, KMOD_CTRL }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_TEXT_EDITOR );
    event_manager.bind<Action_SelectionChange>("Deselect", Shortcut{ 0, KMOD_NONE, "Double click on bg" }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    event_manager.bind<Action_MoveGraph>("Move Graph", Shortcut{ 0, KMOD_NONE, "Drag background" }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    event_manager.bind<Action_FrameGraph>("Frame Selection", Shortcut{ SDLK_f, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    event_manager.bind<Action_FrameGraph >("Frame All", Shortcut{ SDLK_f, KMOD_LCTRL } );

    // Prepare context menu items
    // 1) Blocks
    event_manager.bind<Action_CreateBlock>( ICON_FA_CODE " Condition", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_CONDITION} );
    event_manager.bind<Action_CreateBlock>( ICON_FA_CODE " For Loop", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_FOR_LOOP} );
    event_manager.bind<Action_CreateBlock>( ICON_FA_CODE " While Loop", Shortcut{}, EventPayload_CreateNode{ NodeType_BLOCK_WHILE_LOOP} );
    event_manager.bind<Action_CreateBlock>( ICON_FA_CODE " Scope", Shortcut{}, EventPayload_CreateNode{NodeType_BLOCK_SCOPE} );
    event_manager.bind<Action_CreateBlock>( ICON_FA_CODE " Program", Shortcut{}, EventPayload_CreateNode{NodeType_BLOCK_PROGRAM} );

    // 2) Variables
    event_manager.bind<Action_CreateNode>( ICON_FA_DATABASE " Boolean Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_BOOLEAN, create_variable_node_signature<bool>() } );
    event_manager.bind<Action_CreateNode>( ICON_FA_DATABASE " Double Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_DOUBLE, create_variable_node_signature<double>() } );
    event_manager.bind<Action_CreateNode>( ICON_FA_DATABASE " Integer Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_INTEGER, create_variable_node_signature<int>() } );
    event_manager.bind<Action_CreateNode>( ICON_FA_DATABASE " String Variable", Shortcut{}, EventPayload_CreateNode{ NodeType_VARIABLE_STRING, create_variable_node_signature<std::string>() } );

    // 3) Literals
    event_manager.bind<Action_CreateNode>( ICON_FA_FILE " Boolean Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_BOOLEAN, create_variable_node_signature<bool>() } );
    event_manager.bind<Action_CreateNode>( ICON_FA_FILE " Double Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_DOUBLE, create_variable_node_signature<double>() } );
    event_manager.bind<Action_CreateNode>( ICON_FA_FILE " Integer Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_INTEGER, create_variable_node_signature<int>() } );
    event_manager.bind<Action_CreateNode>( ICON_FA_FILE " String Literal", Shortcut{}, EventPayload_CreateNode{ NodeType_LITERAL_STRING, create_variable_node_signature<std::string>() } );

    // 4) Functions/Operators from the API
    const Nodlang& language = Nodlang::get_instance();
    for ( auto& each_fct: language.get_api() )
    {
        const fw::func_type* func_type = each_fct->get_type();
        std::string label;
        language.serialize_func_sig( label, func_type );
        event_manager.bind<Action_CreateNode>( label.c_str(), Shortcut{}, EventPayload_CreateNode{ NodeType_INVOKABLE, func_type } );
    }
    return true;
}

void Nodable::on_update()
{
    LOG_VERBOSE("ndbl::App", "on_update ...\n");

    // 1. Update current file
    if (current_file && !virtual_machine.is_program_running())
    {
        //
        // When history is dirty we update the graph from the text.
        // (By default undo/redo are text-based only, if hybrid_history is ON, the behavior is different
        if ( current_file->get_history()->is_dirty() && !config.experimental_hybrid_history )
        {
            current_file->update_graph_from_text(config.isolate_selection);
            current_file->get_history()->set_dirty(false);
        }
        // Run the main update loop for the file
        current_file->update();
    }

    // 2. Handle events

    // shorthand to push all shortcuts to a file view overlay depending on conditions
    auto push_overlay_shortcuts = [&](ndbl::HybridFileView& _view, Condition _condition) -> void {
        for (const auto _action: event_manager.get_actions())
        {
            if( ( _action->userdata & _condition) == _condition && (_action->userdata & Condition_HIGHLIGHTED_IN_GRAPH_EDITOR|Condition_HIGHLIGHTED_IN_TEXT_EDITOR) )
            {
                std::string  label        = _action->label.substr(0, 12);
                std::string  shortcut_str = _action->shortcut.to_string();
                OverlayType_ overlay_type = _action->userdata & Condition_HIGHLIGHTED_IN_TEXT_EDITOR ? OverlayType_GRAPH
                                                                                                   : OverlayType_TEXT;
                _view.push_overlay({label, shortcut_str}, overlay_type);
            }
        }
    };

    // Nodable events
    auto       selected_view       = NodeView::get_selected();
    GraphView* graph_view          = current_file ? current_file->get_graph_view() : nullptr;
    History*   curr_file_history   = current_file ? current_file->get_history() : nullptr;

    IEvent* event = nullptr;
    while( (event = event_manager.poll_event()) )
    {
        switch ( event->id )
        {
            case EventID_REQUEST_TOGGLE_ISOLATE:
            {
                config.isolate_selection = !config.isolate_selection;
                if(current_file)
                {
                    current_file->update_graph_from_text(config.isolate_selection);
                }
                break;
            }

            case fw::EventID_REQUEST_EXIT:
            {
                should_stop = true;
                break;
            }

            case fw::EventID_REQUEST_FILE_CLOSE:
            {
                if(current_file) close_file(current_file);
                break;
            }
            case fw::EventID_REQUEST_UNDO:
            {
                if(curr_file_history) curr_file_history->undo();
                break;
            }

            case fw::EventID_REQUEST_REDO:
            {
                if(curr_file_history) curr_file_history->redo();
                break;
            }

            case fw::EventID_REQUEST_FILE_BROWSE:
            {
                std::string path;
                if( m_view->pick_file_path(path, fw::AppView::DIALOG_Browse))
                {
                    open_file(path);
                    break;
                }
                LOG_MESSAGE("App", "Browse file aborted by user.\n");
                break;

            }

            case fw::EventID_REQUEST_FILE_NEW:
            {
                new_file();
                break;
            }

            case fw::EventID_REQUEST_FILE_SAVE_AS:
            {
                if (current_file)
                {
                    std::string path;
                    if( m_view->pick_file_path(path, fw::AppView::DIALOG_SaveAs))
                    {
                        save_file_as(path);
                        break;
                    }
                }
                break;
            }

            case fw::EventID_REQUEST_FILE_SAVE:
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
                        if( m_view->pick_file_path(path, fw::AppView::DIALOG_SaveAs))
                        {
                            save_file_as(path);
                        }
                    }
                }
                break;
            }

            case Event_ShowWindow::id:
            {
                auto _event = reinterpret_cast<Event_ShowWindow*>(event);
                if ( _event->data.window_id == "splashscreen" )
                {
                    config.common.splashscreen = _event->data.visible;
                }
                break;
            }

            case Event_FrameNodeViews::id:
            {
                auto _event = reinterpret_cast<Event_FrameNodeViews*>( event );
                FW_ASSERT(_event->data.graph_view);
                _event->data.graph_view->frame(_event->data.mode);
                break;
            }

            case NodeViewSelectionChangeEvent::id:
            {
                auto _event = reinterpret_cast<NodeViewSelectionChangeEvent*>( event );
                current_file->view.clear_overlay();
                Condition_ condition = _event->data.new_selection ? Condition_ENABLE_IF_HAS_SELECTION
                                                                  : Condition_ENABLE_IF_HAS_NO_SELECTION;
                push_overlay_shortcuts(current_file->view, condition );
                break;
            }
            case EventID_FILE_OPENED:
            {
                if (!current_file) break;
                current_file->view.clear_overlay();
                push_overlay_shortcuts(current_file->view, Condition_ENABLE_IF_HAS_NO_SELECTION);
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
                if (head.flags & SlotFlag_ORDER_SECOND ) std::swap(tail, head); // guarantee src to be the output
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

            case Event_CreateBlock::id:
            case Event_CreateNode::id:
            {
                auto _event = reinterpret_cast<Event_CreateNode*>(event);

                // 1) create the node
                PoolID<Node> new_node_id  = current_file->get_graph()->create_node( _event->data.node_type, _event->data.node_signature );

                // 2) handle connections
                if ( !_event->data.dragged_slot )
                {
                    // Experimental: we try to connect a parent-less child
                    if ( new_node_id != _event->data.graph->get_root() && config.experimental_graph_autocompletion )
                    {
                        _event->data.graph->ensure_has_root();
                        // m_graph->connect( new_node, m_graph->get_root(), RelType::CHILD  );
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
                    view->set_position( _event->data.node_view_local_pos, fw::Space_Local );
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
    LOG_VERBOSE("ndbl::App", "on_update " OK "\n");
}

bool Nodable::on_shutdown()
{
    LOG_VERBOSE("ndbl::App", "on_shutdown ...\n");
    for( HybridFile* each_file : m_loaded_files )
    {
        LOG_VERBOSE("ndbl::App", "Delete file %s ...\n", each_file->path.c_str())
        delete each_file;
    }
    LOG_VERBOSE("ndbl::App", "on_shutdown " OK "\n");

    fw::Pool::shutdown();

    return true;
}

HybridFile *Nodable::open_file(const ghc::filesystem::path& _path)
{
    auto file = new HybridFile(fw::App::asset_path(_path) );

    if ( !file->load() )
    {
        LOG_ERROR("File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
        delete file;
        return nullptr;
    }
    add_file(file);
    file->update_graph_from_text(config.isolate_selection);
    return file;
}

HybridFile *Nodable::add_file(HybridFile* _file)
{
    FW_EXPECT(_file, "File is nullptr");
    m_loaded_files.push_back( _file );
    current_file = _file;
    event_manager.dispatch( fw::EventID_FILE_OPENED );
    return _file;
}

void Nodable::save_file(HybridFile* _file) const
{
    FW_EXPECT(_file,"file must be defined");

	if ( !_file->write_to_disk() )
    {
        LOG_ERROR("ndbl::App", "Unable to save %s (%s)\n", _file->name.c_str(), _file->path.c_str());
        return;
    }
    LOG_MESSAGE("ndbl::App", "File saved: %s\n", _file->path.c_str());
}

void Nodable::save_file_as(const ghc::filesystem::path& _path) const
{
    ghc::filesystem::path absolute_path = fw::App::asset_path(_path);
    current_file->path = absolute_path.string();
    current_file->name = absolute_path.filename().string();
    if( !current_file->write_to_disk() )
    {
        LOG_ERROR("App", "Unable to save as %s (%s)\n", absolute_path.filename().c_str(), absolute_path.c_str());
    }
}

void Nodable::close_file(HybridFile* _file)
{
    // Find and delete the file
    FW_EXPECT(_file, "Cannot close a nullptr File!");
    auto it = std::find(m_loaded_files.begin(), m_loaded_files.end(), _file);
    FW_EXPECT(it != m_loaded_files.end(), "Unable to find the file in the loaded_files");
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
    if ( current_file )
    {
        const Graph* graph = current_file->get_graph();

        if (graph)
        {
            assembly::Compiler compiler;
            auto asm_code = compiler.compile_syntax_tree(graph);

            if (asm_code)
            {
                virtual_machine.release_program();

                if (virtual_machine.load_program(std::move(asm_code)))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void Nodable::run_program()
{
    if (compile_and_load_program() )
    {
        virtual_machine.run_program();
    }
}

void Nodable::debug_program()
{
    if (compile_and_load_program() )
    {
        virtual_machine.debug_program();
    }
}

void Nodable::step_over_program()
{
    virtual_machine.step_over();
    if (!virtual_machine.is_there_a_next_instr() )
    {
        NodeView::set_selected({});
        return;
    }

    const Node* next_node = virtual_machine.get_next_node();
    if ( !next_node ) return;

    if( PoolID<NodeView> next_node_view = next_node->get_component<NodeView>() )
    {
        NodeView::set_selected( next_node_view );
    }
}

void Nodable::stop_program()
{
    virtual_machine.stop_program();
}

void Nodable::reset_program()
{
    if(!current_file) return;

    if (virtual_machine.is_program_running() )
    {
        virtual_machine.stop_program();
    }
    current_file->update_graph_from_text(config.isolate_selection);
}

HybridFile *Nodable::new_file()
{
    m_untitled_file_count++;
    std::string name{"Untitled_"};
    name.append(std::to_string(m_untitled_file_count));
    name.append(".cpp");

    HybridFile* file = new HybridFile(ghc::filesystem::path{name});
    // file->set_text( "// " + name);

    return add_file(file);
}

Nodable &Nodable::get_instance()
{
    FW_EXPECT(s_instance, "No App instance available. Did you forget App app(...) or App* app = new App(...)");
    return *s_instance;
}