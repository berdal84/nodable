#include "Nodable.h"
#include "gui/App.h"

#include <utility>
#include <algorithm>

using namespace ndbl;
using namespace tools;


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "tools/core/Asserts.h"
#include "tools/core/Event_Manager.h"
#include "tools/core/Log.h"
#include "tools/core/System.h"
#include "tools/gui/Action_Manager_View.h"
#include "tools/gui/App_View.h"
#include "tools/gui/Config.h"
#include "tools/gui/Font_Manager.h"
#include "tools/gui/Texture.h"
#include "tools/gui/Texture_Manager.h"

#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Slot.h"
#include "ndbl/core/language/Nodlang.h"

#include "commands/Cmd_Connect.h"
#include "commands/Cmd_Disconnect.h"
#include "commands/Cmd_Group.h"

#include "Node_Slot_View.h"
#include "Node_View.h"
#include "Condition.h"
#include "Config.h"
#include "Event.h"
#include "File.h"
#include "File_View.h"
#include "Graph_View.h"
#include "History.h"

using namespace ndbl;
using namespace tools;

static ndbl::App_State* g_nodable_state = nullptr;

ndbl::App_State* ndbl::nodable_state()
{
    ASSERT(g_nodable_state != nullptr);
    return g_nodable_state;
}

template<typename T>
static Function_Descriptor* create_variable_node_signature()
{
    static Function_Descriptor* descriptor = Function_Descriptor::create<T(T)>("variable");
    return descriptor;
}

void ndbl::nodable_init(App_State* app)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "init ...\n");
 
    ASSERT(g_nodable_state == nullptr);
    g_nodable_state = app;

    app->config = init_config();
    app->view = new ndbl::App_View_State();

    app_init_ex(app, app->view, app->config->tools_cfg ); // the pointers are owned by this class, base app just use them.
    app->language = init_language();

    // Initialize Base View
    tools::appview_init(app->view, app);

    app->view->signal_reset_layout.connect(&_nodable_on_reset_layout);
    app->view->signal_draw_splashscreen_content.connect(&_nodable_on_draw_splashscreen_content);

    // Load splashscreen image
    Config* cfg = get_config();
    tools::Path path = Path::get_asset_path(cfg->ui_splashscreen_imagePath );
    app->view->logo = get_texture_manager()->load(path);

    // Add a bunch of new actions
    tools::Action_Manager* action_manager = get_action_manager();
    ASSERT(action_manager != nullptr); // initialized by base_view
    // (With shortcut)
    action_manager->new_action<Event_DeleteSelection>("Delete", Shortcut{SDLK_DELETE, KMOD_NONE } );
    action_manager->new_action<Event_ArrangeSelection>("Arrange", Shortcut{SDLK_a, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager->new_action<Event_ToggleFolding>("Fold", Shortcut{SDLK_x, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager->new_action<Event_SelectNext>("Next", Shortcut{SDLK_n, KMOD_NONE } );
    action_manager->new_action<Event_FileSave>(ICON_FA_SAVE " Save", Shortcut{SDLK_s, KMOD_CTRL } );
    action_manager->new_action<Event_FileSaveAs>(ICON_FA_SAVE " Save as", Shortcut{SDLK_s, KMOD_CTRL } );
    action_manager->new_action<Event_FileClose>(ICON_FA_TIMES "  Close", Shortcut{SDLK_w, KMOD_CTRL } );
    action_manager->new_action<Event_FileBrowse>(ICON_FA_FOLDER_OPEN " Open", Shortcut{SDLK_o, KMOD_CTRL } );
    action_manager->new_action<Event_FileNew>(ICON_FA_FILE " New", Shortcut{SDLK_n, KMOD_CTRL } );
    action_manager->new_action<Event_ShowWindow>("Splashscreen", Shortcut{SDLK_F1 }, Event_Payload__Show_Window{"splashscreen" } );
    action_manager->new_action<Event_Exit>(ICON_FA_SIGN_OUT_ALT " Exit", Shortcut{SDLK_F4, KMOD_ALT } );
    action_manager->new_action<Event_Undo>("Undo", Shortcut{SDLK_z, KMOD_CTRL } );
    action_manager->new_action<Event_Redo>("Redo", Shortcut{SDLK_y, KMOD_CTRL } );
    action_manager->new_action<Event_ToggleIsolationFlags>("Isolation", Shortcut{SDLK_i, KMOD_CTRL }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_TEXT_EDITOR );
    action_manager->new_action<Event_MoveSelection>("Drag whole graph", Shortcut{SDLK_SPACE, KMOD_NONE, "Space + Drag" }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager->new_action<Event_FrameSelection>("Frame Selection", Shortcut{SDLK_f, KMOD_NONE }, EventPayload_FrameNode_Views{Frame_Mode::Selected_Node_Views }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager->new_action<Event_FrameSelection>("Frame All", Shortcut{SDLK_f, KMOD_LCTRL }, EventPayload_FrameNode_Views{Frame_Mode::Root_Node_View} );
    // (to create block nodes)
    action_manager->new_action<Event_CreateNode>(ICON_FA_CODE " Condition", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__BLOCK_CONDITION } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_CODE " For Loop", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__BLOCK_FOR_LOOP } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_CODE " While Loop", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__BLOCK_WHILE_LOOP } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_CODE " Scope", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__BLOCK_SCOPE } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_CODE " Entry Point", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__ROOT } );
    // (to create variables)
    action_manager->new_action<Event_CreateNode>(ICON_FA_DATABASE " Boolean Variable", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__VARIABLE_BOOLEAN, create_variable_node_signature<bool>() } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_DATABASE " Double Variable", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__VARIABLE_DOUBLE, create_variable_node_signature<double>() } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_DATABASE " Integer Variable", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__VARIABLE_INTEGER, create_variable_node_signature<int>() } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_DATABASE " String Variable", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__VARIABLE_STRING, create_variable_node_signature<std::string>() } );
    //(to create literals)
    action_manager->new_action<Event_CreateNode>(ICON_FA_FILE " Boolean Literal", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__LITERAL_BOOLEAN, create_variable_node_signature<bool>() } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_FILE " Double Literal", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__LITERAL_DOUBLE, create_variable_node_signature<double>() } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_FILE " Integer Literal", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__LITERAL_INTEGER, create_variable_node_signature<int>() } );
    action_manager->new_action<Event_CreateNode>(ICON_FA_FILE " String Literal", Shortcut{}, EventPayload_CreateNode{Create_Node_Type__LITERAL_STRING, create_variable_node_signature<std::string>() } );
    // (to create functions/operators from the API)
    // TODO: add a list of preset to create operators/functions
    // action_manager->new_action<Event_CreateNode>(label.c_str(), Shortcut{}, EventPayload_CreateNode{Create_Node_Type__FUNCTION, invokable->get_sig() } );

    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::NodableView", "init_ex " TOOLS_OK "\n");


    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "init " TOOLS_OK "\n");
}

void ndbl::nodable_do_frame(App_State* app)
{
    nodable_update(app);
    nodable_draw(app);
}

#ifdef __EMSCRIPTEN__
namespace ndbl
{
    void emscripten_loop()
    {
        VERIFY( g_nodable_state != nullptr, "Did you forgot to set g_instance prior to set_main_loop?");
        nodable_do_frame(g_nodable_state);
    }
}
#endif

void ndbl::nodable_run(App_State* app)
{
  #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(&ndbl::emscripten_loop, 0, true);
  #else
    while( !nodable_should_stop(app) )
    {
        nodable_do_frame(app);
    }    
  #endif
}

void ndbl::nodable_update(App_State* app)
{
    tools::app_update(app);

    const float dt = app->view->dt_in_s;

    // 0. Update Views (TODO: is this the right moment to do this? )
    if( app->current_file )
        app->current_file->view.update( dt);

    // 1. delete flagged files
    for( File* file : app->flagged_to_delete_file )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "Nodable", "Delete files flagged to delete: %s\n", file->filename().c_str());
        delete file;
    }
    app->flagged_to_delete_file.clear();

    // 2. Update current file
    if (app->current_file)
    {
        app->current_file->set_isolation( app->config->isolation ); // might change
        app->current_file->update();
    }

    // 3. Handle events

    // Nodable events
    IEvent*       event = nullptr;
    Event_Manager* event_manager     = get_event_manager();
    Graph_View*    graph_view        = app->current_file ? app->current_file->graph()->component<Graph_View>() : nullptr; // TODO: should be included in the event
    History*      curr_file_history = app->current_file ? &app->current_file->history : nullptr; // TODO: should be included in the event
    while( (event = event_manager->poll_event()) )
    {
        switch ( event->id )
        {
            case Event_ID_RESET_GRAPH:
            {
                app->current_file->set_graph_dirty();
                break;
            }

            case Event_ID_TOGGLE_ISOLATION_FLAGS:
            {
                app->config->isolation = ~app->config->isolation;
                if(app->current_file)
                {
                    app->current_file->set_graph_dirty();
                }
                break;
            }

            case Event_ID_REQUEST_EXIT:
            {
                tools::app_request_stop(app);
                break;
            }

            case Event_ID_FILE_CLOSE:
            {
                nodable_close_file(app);
                break;
            }
            case Event_ID_UNDO:
            {
                if(curr_file_history) curr_file_history->undo();
                break;
            }

            case Event_ID_REDO:
            {
                if(curr_file_history) curr_file_history->redo();
                break;
            }

            case Event_ID_FILE_BROWSE:
            {
                Path path;
                if( pick_file_path(path, Dialog_Type_Browse) )
                {
                    nodable_open_file(app, path);
                    break;
                }
                TOOLS_LOG(tools::Verbosity_Diagnostic, "App", "Browse file aborted by user.\n");
                break;

            }

            case Event_ID_FILE_NEW:
            {
                nodable_new_file(app);
                break;
            }

            case Event_ID_FILE_SAVE_AS:
            {
                if (app->current_file != nullptr)
                {
                    Path path;
                    if( pick_file_path(path, Dialog_Type_SaveAs))
                    {
                       nodable_save_file_as(app, app->current_file, path);
                    }
                }

                break;
            }

            case Event_ID_FILE_SAVE:
            {
                if (!app->current_file) break;
                if( !app->current_file->path.empty())
                {
                    nodable_save_file(app, app->current_file);
                }
                else
                {
                    Path path;
                    if( pick_file_path(path, Dialog_Type_SaveAs))
                    {
                        nodable_save_file_as(app, app->current_file, path);
                    }
                }
                break;
            }

            case Event_ShowWindow::id:
            {
                auto _event = reinterpret_cast<Event_ShowWindow*>(event);
                if ( _event->data.window_id == "splashscreen" )
                {
                    app->view->show_splashscreen = _event->data.visible;
                }
                break;
            }

            case Event_FrameSelection::id:
            {
                auto _event = reinterpret_cast<Event_FrameSelection*>( event );
                VERIFY(graph_view, "a graph_view is required");
                graph_view->frame_content(_event->data.mode);
                break;
            }

            case Event_ID_FILE_OPENED:
            {
                ASSERT(app->current_file != nullptr );
                app->current_file->view.clear_overlay();
                app->current_file->view.refresh_overlay(Condition_ENABLE_IF_HAS_NO_SELECTION );
                break;
            }
            case Event_DeleteSelection::id:
            {
                if ( graph_view )
                {
                    for(const Selectable& elem : graph_view->selection() )
                    {
                        if ( auto nodeview = elem.get_if<Node_View*>() )
                            graph_view->graph()->flag_node_to_delete(nodeview->node(), Graph_Flag_NONE);
                        else if ( auto scopeview = elem.get_if<Scope_View*>() )
                            graph_view->graph()->flag_node_to_delete(scopeview->node(), Graph_Flag_ALLOW_SIDE_EFFECTS);
                    }
                }

                break;
            }

            case Event_ArrangeSelection::id:
            {
                if ( graph_view )
                {
                    for( const Selectable& elem : graph_view->selection() )
                    {
                        switch ( elem.index() )
                        {
                            case Selectable::index_of<Node_View*>():
                                elem.get<Node_View*>()->arrange_recursively();
                                break;
                            case Selectable::index_of<Scope_View*>():
                                elem.get<Scope_View*>()->arrange_content();
                                break;
                        }
                    }
                }

                break;
            }

            case Event_SelectNext::id:
            {
                if ( graph_view && graph_view->selection().contains<Node_View*>() )
                {
                    graph_view->selection().clear();
                    for(auto* _view : graph_view->selection().collect<Node_View*>() )
                        for (auto* _successor : _view->node()->component<Node>()->flow_outputs() )
                            if (auto* _successor_view = _successor->component<Node_View>() )
                                graph_view->selection().append( _successor_view );
                }
                break;
            }

            case Event_ToggleFolding::id:
            {
                if ( graph_view )
                    break;

                for( Node_View* view : graph_view->selection().collect<Node_View*>() )
                {
                    auto _event = reinterpret_cast<Event_ToggleFolding*>(event);
                    _event->data.mode == RECURSIVELY ? view->expand_toggle_rec()
                                                     : view->expand_toggle();
                }
                break;
            }

            case Event_Node_SlotDropped::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto _event = reinterpret_cast<Event_Node_SlotDropped*>(event);
                Node_Slot* tail = _event->data.first;
                Node_Slot* head = _event->data.second;
                ASSERT(head != tail);
                if ( tail->order() == Node_Slot::Flag_ORDER_2ND )
                {
                    if ( head->order() == Node_Slot::Flag_ORDER_2ND )
                    {
                        TOOLS_LOG(tools::Verbosity_Error, "Nodable", "Unable to connect incompatible edges\n");
                        break; // but if it still the case, that's because edges are incompatible
                    }
                    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Nodable", "Swapping edges to try to connect them\n");
                    std::swap(tail, head);
                }
                auto cmd = std::make_shared<Cmd_Connect>(tail, head);
                curr_file_history->push_command(cmd);

                break;
            }

            case Event_DeleteEdge::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto _event = reinterpret_cast<Event_DeleteEdge*>(event);
                Node_Slot* tail = _event->data.first;
                Node_Slot* head = _event->data.second;
                auto command = std::make_shared<Cmd_DeleteEdge>(tail, head);
                curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(command));
                break;
            }

            case Event_Node_SlotDisconnectAll::id:
            {
                ASSERT(curr_file_history != nullptr);
                auto _event = static_cast<Event_Node_SlotDisconnectAll*>(event);
                Node_Slot* slot = _event->data.first;

                auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Edges");
                for(Node_Slot* adjacent_slot : slot->adjacent )
                {
                    auto each_cmd = std::make_shared<Cmd_DeleteEdge>(slot, adjacent_slot );
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
                if ( !graph->root_node() )
                {
                    TOOLS_LOG(tools::Verbosity_Error, "Nodable", "Unable to create_new primary_child, no root found on this graph.\n");
                    continue;
                }

                Node* new_node  = graph->create_node( _event->data.node_type,
                                                         _event->data.node_signature,
                                                         graph->root_scope() );

                // Insert an end of line and end of instruction
                switch ( _event->data.node_type )
                {
                    case Create_Node_Type__BLOCK_CONDITION:
                    case Create_Node_Type__BLOCK_FOR_LOOP:
                    case Create_Node_Type__BLOCK_WHILE_LOOP:
                    case Create_Node_Type__BLOCK_SCOPE:
                    case Create_Node_Type__ROOT:
                        new_node->suffix = Token::s_end_of_line;
                        break;
                    case Create_Node_Type__VARIABLE_BOOLEAN:
                    case Create_Node_Type__VARIABLE_DOUBLE:
                    case Create_Node_Type__VARIABLE_INTEGER:
                    case Create_Node_Type__VARIABLE_STRING:
                        new_node->suffix = Token::s_end_of_instruction;
                        break;
                    case Create_Node_Type__LITERAL_BOOLEAN:
                    case Create_Node_Type__LITERAL_DOUBLE:
                    case Create_Node_Type__LITERAL_INTEGER:
                    case Create_Node_Type__LITERAL_STRING:
                    case Create_Node_Type__FUNCTION:
                        break;
                }

                // 2) handle connections
                if ( Node_Slot_View* slot_view = _event->data.active_slotview )
                {
                    Node_Slot::Flags         complementary_flags = node_slot_flags_toggle_order(slot_view->slot->type_and_order());
                    const Type_Descriptor*  type                = slot_view->property()->type;
                    Node_Slot*              complementary_slot  = node_find_slot_by_property_type(new_node, complementary_flags, type);

                    if ( !complementary_slot )
                    {
                        // TODO: this case should not happens, instead we should check ahead of time whether or not this not can be attached
                        TOOLS_LOG(tools::Verbosity_Error,  "Graph_View", "unable to connect this primary_child" );
                    }
                    else
                    {
                        Node_Slot* out = slot_view->slot;
                        Node_Slot* in  = complementary_slot;

                        if ( out->has_flags( Node_Slot::Flag_ORDER_2ND ) )
                            std::swap( out, in );

                        graph->connect(out, in, Graph_Flag_ALLOW_SIDE_EFFECTS );

                        // Ensure has a "\n" when connecting using CODEFLOW (to split lines)
                        if (node_is_instruction(out->node ) && out->type() == Node_Slot::Flag_TYPE_FLOW )
                        {
                            std::string buffer = out->node->suffix.string();
                            if ( buffer.empty() || std::find(buffer.rbegin(), buffer.rend(), '\n') == buffer.rend() )
                                out->node->suffix.suffix_push_back("\n");
                        }
                    }
                }

                // set new_node's view position, select it
                if ( auto view = new_node->component<Node_View>() )
                {
                    view->spatial_node()->set_position(_event->data.desired_screen_pos, WORLD_SPACE);
                    graph_view->selection().clear();
                    graph_view->selection().append(view);
                }
                break;
            }

            default:
            {
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "App", "Ignoring and event, this case is not handled\n");
            }
        }

        // clean memory
        delete event;
    }
}

void ndbl::nodable_shutdown(App_State* app)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "_handle_shutdown ...\n");

    for( File* each_file : app->loaded_files )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::App", "Delete file %s ...\n", each_file->path.c_str());
        delete each_file;
    }

    // shutdown managers & co.
    shutdown_language(app->language);
    // We could do this there, but the base view is responsible for shutdow the texture manager we used, so all textures will be released.
    // get_texture_manager()->release(app->view->logo);
    app->view->signal_reset_layout.disconnect();
    app->view->signal_draw_splashscreen_content.disconnect();
    appview_shutdown(app->view);
    tools::app_shutdown(app);
    ndbl::shutdown_config(app->config);

    delete app->view;

    g_nodable_state = nullptr;
    
    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "_handle_shutdown " TOOLS_OK "\n");
}

File* ndbl::nodable_open_asset_file(App_State* app, const tools::Path& _path)
{
    if ( _path.is_absolute() )
        return nodable_open_file(app, _path);

    return nodable_open_file(app, Path::absolute(_path) );
}

File* ndbl::nodable_open_file(App_State* app, const tools::Path& _path)
{
    File* file = new File();

    if ( File::read( *file, _path ) )
    {
        return nodable_add_file(app, file);
    }

    delete file;
    TOOLS_LOG(tools::Verbosity_Error, "File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
    return nullptr;
}

File* ndbl::nodable_add_file(App_State* app, File* _file)
{
    VERIFY(_file, "File is nullptr");
    app->loaded_files.push_back( _file );
    app->current_file = _file;
    get_event_manager()->dispatch( Event_ID_FILE_OPENED );
    return _file;
}

void ndbl::nodable_save_file(const App_State* app, File* _file)
{
    VERIFY(_file, "file must be defined");

	if ( !File::write(*_file, _file->path) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "ndbl::App", "Unable to save %s (%s)\n", _file->filename().c_str(), _file->path.c_str());
        return;
    }
    TOOLS_LOG(tools::Verbosity_Message, "ndbl::App", "File saved: %s\n", _file->path.c_str());
}

void ndbl::nodable_save_file_as(const App_State* app, File* _file, const tools::Path& _path)
{
    if ( !File::write(*_file, _path) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "ndbl::App", "Unable to save %s (%s)\n", _path.filename().c_str(), _path.c_str());
        return;
    }
    TOOLS_LOG(tools::Verbosity_Message, "ndbl::App", "File saved: %s\n", _path.c_str());
}

void ndbl::nodable_close_file(App_State* app)
{
    if ( app->current_file == nullptr )
        return;
    nodable_close_file(app, app->current_file);
}
void ndbl::nodable_close_file(App_State* app, File* _file)
{
    // Find and delete the file
    VERIFY(_file, "Cannot close a nullptr File!");
    auto it = std::find(app->loaded_files.begin(), app->loaded_files.end(), _file);
    VERIFY(it != app->loaded_files.end(), "Unable to find the file in the loaded_files");
    it = app->loaded_files.erase(it);
    app->flagged_to_delete_file.push_back(_file);

    // Switch to the next file if possible
    if ( it != app->loaded_files.end() )
    {
        app->current_file = *it;
    }
    else
    {
        app->current_file = nullptr;
    }
}

void ndbl::nodable_reset_current_graph(App_State* app)
{
    if(!app->current_file) return;

    // n.b. nodable is still text oriented
    app->current_file->set_graph_dirty();
}

File*ndbl::nodable_new_file(App_State* app)
{
    app->untitled_file_count++;

    String_32 name;
    name.append_fmt("Untitled_%i.cpp", app->untitled_file_count);
    auto* file = new File();
    file->path = name.c_str();

    return nodable_add_file(app, file);
}

bool ndbl::nodable_should_stop(const App_State* app)
{
    return tools::app_should_stop(app);
}

void ndbl::nodable_set_current_file(App_State* app, File* file)
{
    if ( app->current_file == nullptr )
    {
        app->current_file = file;
        return;
    }

    // TODO:
    //  - unload current file?
    //  - keep the last N files loaded?
    //  - save graph to a temp file to restore it later without using memory and altering original source file?
    // close_file(app->current_file); ??

    app->current_file = file;
}

void ndbl::_nodable_on_draw_splashscreen_content()
{
    App_State* app = nodable_state();
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // Image
    ImGui::SameLine((ImGui::GetContentRegionAvail().x - (float)app->view->logo->width) * 0.5f); // center img
    ImGuiEx::Image(app->view->logo);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {50.0f, 30.0f});

    // disclaimer
    ImGui::TextWrapped("DISCLAIMER: This software is a prototype, do not expect too much from it. Use at your own risk.");

    ImGui::NewLine();
    ImGui::NewLine();

    // credits
    const char *credit = "by Berdal84";
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(credit).x);
    ImGui::TextWrapped("%s", credit);

    // close on left/rightmouse btn click
    if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
    {
        app->view->show_splashscreen = false;
    }
    ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
}

void ndbl::_nodable_on_reset_layout()
{
    Config*         cfg   = get_config();
    App_State*   app = nodable_state();

    // Dock windows to specific dockspace
    appview_dock_window( app->view, cfg->ui_help_window_label             , Dockspace_RIGHT );
    appview_dock_window( app->view, cfg->ui_config_window_label           , Dockspace_RIGHT );
    appview_dock_window( app->view, cfg->ui_file_info_window_label        , Dockspace_RIGHT );
    appview_dock_window( app->view, cfg->ui_node_properties_window_label  , Dockspace_RIGHT );
    appview_dock_window( app->view, cfg->ui_interpreter_window_label      , Dockspace_RIGHT );
    appview_dock_window( app->view, cfg->ui_imgui_config_window_label     , Dockspace_RIGHT );
    appview_dock_window( app->view, cfg->ui_toolbar_window_label          , Dockspace_TOP   );
};

void ndbl::nodable_draw(App_State* app)
{
    VERIFY(app->view->logo != nullptr, "Logo is nullptr, did you call init_ex() ?");

    const float dt = app->view->dt_in_s;

    // note: we draw this view nested in base view's begin/end (similar to ImGui API).
    tools::appview_begin(app->view);

    Event_Manager*   event_manager   = get_event_manager();
    Config*         cfg             = get_config();
    tools::Config*  tools_cfg       = tools::get_config();
    bool            redock_all      = true;
    File*           current_file    = app->current_file;

    // 1. Draw Menu Bar
    if (ImGui::BeginMenuBar())
    {
        History* current_file_history = current_file ? &current_file->history : nullptr;
        auto has_selection = current_file != nullptr ? !current_file->graph()->component<Graph_View>()->selection().empty()
                                                     : false;

        if (ImGui::BeginMenu("File"))
        {
            bool has_file = current_file != nullptr;
            bool is_current_file_content_dirty = current_file != nullptr && current_file->needs_to_be_saved();
            ImGuiEx::MenuItem_EventTrigger<Event_FileNew>();
            ImGuiEx::MenuItem_EventTrigger<Event_FileBrowse>();
            ImGui::Separator();
            ImGuiEx::MenuItem_EventTrigger<Event_FileSaveAs>(false, has_file);
            ImGuiEx::MenuItem_EventTrigger<Event_FileSave>(false, has_file && is_current_file_content_dirty);
            ImGui::Separator();
            ImGuiEx::MenuItem_EventTrigger<Event_FileClose>(false, has_file);

            auto auto_paste = has_file && current_file->view.experimental_clipboard_auto_paste();

            if (ImGui::MenuItem(ICON_FA_COPY        "  Auto-paste clipboard", "", auto_paste, has_file ) && has_file ) {
                current_file->view.experimental_clipboard_auto_paste(!auto_paste);
            }

            ImGuiEx::MenuItem_EventTrigger<Event_Exit>();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (current_file_history)
            {
                ImGuiEx::MenuItem_EventTrigger<Event_Undo>();
                ImGuiEx::MenuItem_EventTrigger<Event_Redo>();
                ImGui::Separator();
            }
            if (ImGui::MenuItem("Delete Node", "Del.", false, has_selection ))
                event_manager->dispatch( Event_ID_DELETE_NODE );

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            //auto frame = ImGui::MenuItem("Frame All", "F");
            redock_all |= ImGui::MenuItem("Redock documents");

            ImGui::Separator();

            auto menu_item_node_view_detail = [current_file, cfg](View_Detail _detail, const char *_label) {
                if (ImGui::MenuItem(_label, "", cfg->ui_node_detail == _detail))
                {
                    cfg->ui_node_detail = _detail;
                    if (current_file != nullptr)
                        current_file->graph()->component<Graph_View>()->reset_all_properties();
                }
            };

            ImGui::Text("View Detail:");
            ImGui::Indent();
            menu_item_node_view_detail(View_Detail::MINIMALIST, "Minimalist");
            menu_item_node_view_detail(View_Detail::NORMAL,     "Normal");
            ImGui::Unindent();

            ImGui::Separator();
            app->view->show_properties_editor = ImGui::MenuItem(ICON_FA_COGS " Show Properties", "",
                                                       app->view->show_properties_editor);
            app->view->show_imgui_demo = ImGui::MenuItem("Show ImGui Demo", "", app->view->show_imgui_demo);

            ImGui::Separator();

            const bool is_fullscreen = appview_is_fullscreen(app->view);
            if (ImGui::MenuItem("Fullscreen", "", is_fullscreen ))
            {
                appview_set_fullscreen(app->view, !is_fullscreen);
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Reset Layout", ""))
            {
                app->view->should_reset_layout = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Code"))
        {
            ImGuiEx::MenuItem_EventTrigger<Event_ToggleIsolationFlags>(cfg->isolation);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Graph"))
        {

            if (ImGui::MenuItem("Reset"))
                event_manager->dispatch( Event_ID_RESET_GRAPH );

            ImGuiEx::MenuItem_EventTrigger<Event_ArrangeSelection>(false, has_selection);
            ImGuiEx::MenuItem_EventTrigger<Event_ToggleFolding>(false, has_selection);

            if (ImGui::MenuItem("Expand/Collapse recursive", nullptr, false, has_selection))
            {
                event_manager->dispatch<Event_ToggleFolding>( { RECURSIVELY } );
            }

            ImGui::Separator();

            ImGuiEx::MenuItem_EventTrigger<Event_ToggleIsolationFlags>(cfg->isolation);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Developer"))
        {
            bool debug = cfg->flags & Config_Flag_DRAW_DEBUG_LINES;
            if ( ImGui::MenuItem("Debug Mode", "", debug ) )
            {
                cfg->tools_cfg->runtime_debug = !debug;
                cfg->clear_flags( Config_Flag_DRAW_DEBUG_LINES );
                cfg->set_flags( !debug * Config_Flag_DRAW_DEBUG_LINES);
                ImGuiEx::set_debug( !debug );
            }

            if ( ImGui::MenuItem("Limit FPS", "", tools_cfg->fps_limit_on ) )
            {
                tools_cfg->fps_limit_on = !tools_cfg->fps_limit_on;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Verbosity"))
            {
                auto menu_item_verbosity = [](Verbosity verbosity, const char* label)
                {
                    if (ImGui::MenuItem(label, "", get_log_verbosity() == verbosity))
                    {
                        set_log_verbosity(verbosity);
                    }
                };

                menu_item_verbosity(Verbosity_Diagnostic, "Verbose");
                menu_item_verbosity(Verbosity_Message, "Message");
                menu_item_verbosity(Verbosity_Warning, "Warning");
                menu_item_verbosity(Verbosity_Error,   "Error");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Experimental"))
            {
                auto checkbox_flag = [&](const char* label, Config_Flag_ flag )
                {
                    bool enabled = cfg->has_flags(flag);
                    if ( ImGui::Checkbox(label, &enabled) )
                    {
                        if ( !enabled )
                            cfg->clear_flags(flag);
                        else
                            cfg->set_flags(flag);
                    }
                };
                checkbox_flag("Hybrid history"       , Config_Flag_EXPERIMENTAL_HYBRID_HISTORY);
                checkbox_flag("Multi-Selection"      , Config_Flag_EXPERIMENTAL_MULTI_SELECTION);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("An issue ?")) {
            if (ImGui::MenuItem("Report on Github.com")) {
                system_open_url_async("https://github.com/berdal84/nodable/issues");
            }

            if (ImGui::MenuItem("Report by email")) {
                system_open_url_async("mail:berenger@42borgata.com");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Show Splash Screen", "F1"))
            {
                app->view->show_splashscreen = true;
            }

            if (ImGui::MenuItem("Browse source code"))
            {
                system_open_url_async("https://www.github.com/berdal84/nodable");
            }

            if (ImGui::MenuItem("Credits"))
            {
                system_open_url_async("https://github.com/berdal84/nodable#credits-");
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // 2. Draw windows
    // All draw_xxx_window() are ImGui windows docked to a dockspace (defined in signal_reset_layout() )

    ImGuiID ds_root = app->view->dockspaces[Dockspace_ROOT];
    if( app->loaded_files.empty() )
    {
        bool show_startup_window = !app->view->show_splashscreen;
        if( show_startup_window )
        {
            nodable_draw_startup_window(app, ds_root);
        }
    }
    else
    {
        nodable_draw_toolbar_window(app);

        for ( File* each_file : app->loaded_files )
        {
            nodable_draw_file_window( app, ds_root, redock_all, each_file);
        }

        nodable_draw_file_info_window(app);
        nodable_draw_config_window(app);
        nodable_draw_imgui_config_window(app);

        if ( nodable_draw_node_properties_window(app) )
            app->current_file->set_text_dirty();
        nodable_draw_help_window(app);
    }

    // end the drawing
    appview_end(app->view);
}

void ndbl::nodable_draw_help_window(const App_State* app)
{
    Config*      cfg  = get_config();
    if (ImGui::Begin( cfg->ui_help_window_label))
    {
        Font_Manager* font_manager = get_font_manager();
        ImGui::PushFont(font_manager->get_font(Font_Slot_Heading));
        ImGui::Text("Welcome to Nodable!");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped(
                "Nodable is primary_child-able.\n"
                "\n"
                "Nodable allows you to edit a program using both text and graph paradigms."
                "More precisely, it means:"
        );
        ImGuiEx::BulletTextWrapped("any change on the text will affect the graph");
        ImGuiEx::BulletTextWrapped("any change (structure or values) on the graph will affect the text");
        ImGuiEx::BulletTextWrapped(
                "but keep in mind the app is the text, any change not affecting the text (such as child positions or orphan primary_child) will be lost.");
        ImGui::NewLine();
        ImGui::PushFont(font_manager->get_font(Font_Slot_Heading));
        ImGui::Text("Quick start");
        ImGui::PopFont();
        ImGui::NewLine();
        ImGui::TextWrapped("Nodable UI is designed as following:\n");
        ImGuiEx::BulletTextWrapped("On the left side a (light) text editor allows to edit source code.\n");
        ImGuiEx::BulletTextWrapped(
                "At the center, there is the graph editor where you can create_new/delete/connect primary_child\n");
        ImGuiEx::BulletTextWrapped(
                "On the right side (this side) you will find many tabs to manage additional config such as primary_child, interpreter, or app properties\n");
        ImGuiEx::BulletTextWrapped("At the top, between the menu and the editors, there is a tool bar."
                                       " There, few buttons will serve to compile, run and debug your program.");
        ImGuiEx::BulletTextWrapped("And at the bottom, below the editors, there is a status bar."
                                       " This bar will display important messages, warning, and errors. You can expand it to get older messages.");
    }
    ImGui::End();
}

void ndbl::nodable_draw_imgui_config_window(App_State* app)
{
    Config*      cfg  = get_config();
    tools::Config* tools_cfg = tools::get_config();
    if( !tools_cfg->runtime_debug )
    {
        return;
    }

    if (ImGui::Begin( cfg->ui_imgui_config_window_label))
    {
        ImGui::ShowStyleEditor();
    }
    ImGui::End();
}

void ndbl::nodable_draw_file_info_window(App_State* app)
{
    Config* cfg  = get_config();

    if ( app->current_file == nullptr )
    {
        return;
    }

    if (ImGui::Begin( cfg->ui_file_info_window_label))
    {
        app->current_file->view.draw_info_panel();
    }

    ImGui::End();
}

bool ndbl::nodable_draw_node_properties_window(App_State* app)
{
    bool changed = false;
    Config* cfg = get_config();
    if (ImGui::Begin( cfg->ui_node_properties_window_label))
    {
        if( app->current_file )
        {
            const Graph_View* graph_view = app->current_file->graph()->component<Graph_View>(); // Graph can't be null
            switch ( graph_view->selection().count<Node_View*>() )
            {
                case 0:
                    break;
                case 1:
                {
                    ImGui::Indent(10.0f);
                    auto* first_nodeview = graph_view->selection().first_of<Node_View*>();
                    changed |= Node_View::draw_as_properties_panel(first_nodeview, &app->view->show_advanced_node_properties);
                    break;
                }
                default:
                    ImGui::Indent(10.0f);
                    ImGui::Text("Multi-Selection");
            }
        }
    }
    ImGui::End();
    return changed;
}

void ndbl::nodable_draw_startup_window(App_State* app, ImGuiID dockspace_id)
{
    Config*      cfg  = get_config();

    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 1.f));

    ImGui::Begin( cfg->ui_startup_window_label);
    {
        Event_Manager* event_manager = get_event_manager();
        Font_Manager*  font_manager  = get_font_manager();

        ImGui::PopStyleColor();

        ImVec2 center_area(500.0f, 250.0f);
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::SetCursorPosX((avail.x - center_area.x) / 2);
        ImGui::SetCursorPosY((avail.y - center_area.y) / 2);

        ImGui::BeginChild("center_area", center_area);
        {
            ImGui::Indent(center_area.x * 0.05f);

            ImGui::PushFont(font_manager->get_font(Font_Slot_ToolBtn));
            ImGui::NewLine();

            ImVec2 btn_size(center_area.x * 0.44f, 40.0f);
            if (ImGui::Button(ICON_FA_FILE" New File", btn_size))
                event_manager->dispatch( Event_ID_FILE_NEW );
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER_OPEN" Open ...", btn_size))
                event_manager->dispatch( Event_ID_FILE_BROWSE );

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();

            ImGui::Text("%s", "Open an example");

            struct Example {
                const char* label;
                const char* path;
            };

            const std::array<Example, 4> examples = {
                Example{ ICON_FA_BOOK" Single expressions    ", "examples/arithmetic.cpp" },
                Example{ ICON_FA_BOOK" Multi instructions    ", "examples/multi-instructions.cpp" },
                Example{ ICON_FA_BOOK" Conditional Structures", "examples/if-else.cpp" },
                Example{ ICON_FA_BOOK" For Loop              ", "examples/for-loop.cpp" }
            };

            const ImVec2 example_btn_size(btn_size.x, btn_size.y * 0.66f);
            const int columns = 2;

            ImGui::NewLine();
            for (size_t i = 0; i < examples.size(); ++i )
            {
                if (i % columns != 0) ImGui::SameLine();
                if (ImGui::Button(examples[i].label, example_btn_size))
                {
                    nodable_open_asset_file(app, examples[i].path);
                }
            }

            ImGui::PopFont();

            ImGui::NewLine();
            ImGui::Unindent();
        }
        ImGui::EndChild();
    }
    ImGui::End(); // Startup Window
}

void ndbl::nodable_draw_file_window( App_State* app, ImGuiID dockspace_id, bool redock_all, File*file)
{
    Config* cfg = get_config();

    ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_Appearing);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar
                                  | ImGuiWindowFlags_UnsavedDocument * file->needs_to_be_saved();

    auto child_bg = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
    child_bg.w = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg);

    bool open    = true;
    bool visible = ImGui::Begin(file->filename().c_str(), &open, window_flags);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(1);

    if ( visible )
    {
        // Set current file if window is focused
        if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            if ( app->current_file != file )
                nodable_set_current_file(app, file);

        // Draw content
        file->view.draw( app->view->dt_in_s );
    }
    ImGui::End();

    if ( !open )
    {
        nodable_close_file(app, file);
    }
}

void ndbl::nodable_draw_config_window(App_State* app)
{
    Config*      cfg  = get_config();
    auto* tools_cfg = tools::get_config();

    if (ImGui::Begin( cfg->ui_config_window_label))
    {
        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

        ImGui::Text("Nodable Settings");
        if ( ImGui::Button("Reset Settings") )
        {
            cfg->reset();
        }

        if (ImGui::CollapsingHeader("Sizes", flags ))
        {
            ImGui::SliderFloat("set_size factor SM", &cfg->tools_cfg->size_factor[Size_SM], 0.0f, 5.0f);
            ImGui::SliderFloat("set_size factor MD", &cfg->tools_cfg->size_factor[Size_MD], 0.0f, 5.0f);
            ImGui::SliderFloat("set_size factor LR", &cfg->tools_cfg->size_factor[Size_LG], 0.0f, 5.0f);
            ImGui::SliderFloat("set_size factor XL", &cfg->tools_cfg->size_factor[Size_XL], 0.0f, 5.0f);
        }

        if (ImGui::CollapsingHeader("Nodes", flags ))
        {
            ImGui::Indent();
            if ( ImGui::CollapsingHeader("Colors", flags ))
            {
                ImGui::ColorEdit4("default"     , &cfg->ui_node_fill_color[Node_Type_NULL].x );
                ImGui::ColorEdit4("entry point" , &cfg->ui_node_fill_color[Node_Type_SCOPE].x );
                ImGui::ColorEdit4("condition"   , &cfg->ui_node_fill_color[Node_Type_IF_ELSE].x );
                ImGui::ColorEdit4("for loop"    , &cfg->ui_node_fill_color[Node_Type_FOR_LOOP].x );
                ImGui::ColorEdit4("while loop"  , &cfg->ui_node_fill_color[Node_Type_WHILE_LOOP].x );
                ImGui::ColorEdit4("variable"    , &cfg->ui_node_fill_color[Node_Type_VARIABLE].x );
                ImGui::ColorEdit4("literal"     , &cfg->ui_node_fill_color[Node_Type_LITERAL].x );
                ImGui::ColorEdit4("function"    , &cfg->ui_node_fill_color[Node_Type_FUNCTION].x );
                ImGui::ColorEdit4("operator"    , &cfg->ui_node_fill_color[Node_Type_OPERATOR].x );
                ImGui::Separator();
                ImGui::ColorEdit4("highlighted"         , &cfg->ui_node_highlightedColor.x);
                ImGui::ColorEdit4("shadow"              , &cfg->ui_node_shadowColor.x);
                ImGui::ColorEdit4("border"              , &cfg->ui_slot_border_color.x);
                ImGui::ColorEdit4("border (highlighted)", &cfg->ui_node_borderHighlightedColor.x);
                ImGui::ColorEdit4("slot (in)"           , &cfg->ui_slot_color_light.x);
                ImGui::ColorEdit4("slot (out)"          , &cfg->ui_slot_color_dark.x);
                ImGui::ColorEdit4("slot (hovered)"      , &cfg->ui_slot_hovered_color.x);
            }

            if ( ImGui::CollapsingHeader("Node_Slots", flags ))
            {
                ImGui::Text("Property Node_Slots:");
                ImGui::SliderFloat("slot radius", &cfg->ui_slot_circle_radius_base, 5.0f, 10.0f);

                ImGui::Separator();

                ImGui::Text("Code Flow Node_Slots:");
                ImGui::SliderFloat2("slot set_size##codeflow"   , &cfg->ui_slot_rectangle_size.x, 2.0f, 100.0f);
                ImGui::SliderFloat("slot padding##codeflow" , &cfg->ui_slot_gap, 0.0f, 100.0f);
                ImGui::SliderFloat("slot radius##codeflow"  , &cfg->ui_slot_border_radius, 0.0f, 40.0f);
            }

            if ( ImGui::CollapsingHeader("Misc.", flags ))
            {
                ImGui::SliderFloat2("gap app (x and y-axis)", &cfg->ui_node_gap_base.x, 0.0f, 400.0f);
                ImGui::SliderFloat("velocity" , &cfg->ui_node_speed, 1.0f, 10.0f);
                ImGui::SliderFloat4("padding" , &cfg->ui_node_padding.x, 0.0f, 20.0f);
                ImGui::SliderFloat("border width", &cfg->ui_node_borderWidth, 0.0f, 10.0f);
                ImGui::SliderFloat("border width ratio (instructions)", &cfg->ui_node_instructionBorderRatio, 0.0f, 10.0f);
            }
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Wires / Code Flow", flags ))
        {
            ImGui::Text("Wires");
            ImGui::SliderFloat("thickness", &cfg->ui_wire_bezier_thickness, 0.5f, 10.0f);
            ImGui::SliderFloat2("roundness (min,max)", &cfg->ui_wire_bezier_roundness.x, 0.0f, 1.0f);
            ImGui::SliderFloat2("fade length (min,max in lensqr)", &cfg->ui_wire_bezier_fade_lensqr_range.x, 0.0f, 100000.0f);
            ImGui::ColorEdit4("color", &cfg->ui_wire_color.x);
            ImGui::ColorEdit4("shadow color", &cfg->ui_wire_shadowColor.x);

            ImGui::Separator();

            ImGui::Text("Code Flow");
            ImGui::ColorEdit4("color##codeflow", &cfg->ui_codeflow_color.x);
            ImGui::SliderFloat("thickness (ratio)##codeflow", &cfg->ui_codeflow_thickness_ratio, 0.1, 1.0);
        }

        if (ImGui::CollapsingHeader("Graph", flags ))
        {
            ImGui::InputFloat("view unfold duration (sec)", &cfg->graph_view_unfold_duration);
            ImGui::ColorEdit4("grid color (major)", &cfg->ui_graph_grid_color_major.x);
            ImGui::ColorEdit4("grid color (minor)", &cfg->ui_graph_grid_color_minor.x);
            ImGui::SliderInt("grid set_size", &cfg->ui_grid_size, 1, 500);
            ImGui::SliderInt("grid subdivisions", &cfg->ui_grid_subdiv_count, 1, 16);
        }

        if (ImGui::CollapsingHeader("Scope", flags ))
        {
            ImGui::SliderFloat4("margins", &cfg->ui_scope_content_rect_margin.min.x, 2, 25);
            ImGui::SliderFloat4("primary_child margin", &cfg->ui_scope_child_margin, 2, 25);
            ImGui::SliderFloat("border radius", &cfg->ui_scope_border_radius, 0, 20);
            ImGui::SliderFloat("border thickness", &cfg->ui_scope_border_thickness, 0, 4);
            ImGui::ColorEdit4("fill color (light)", &cfg->ui_scope_fill_col_light.x);
            ImGui::ColorEdit4("fill color (dark)", &cfg->ui_scope_fill_col_dark.x);
            ImGui::ColorEdit4("border color", &cfg->ui_scope_border_col.x);
        }

        if (ImGui::CollapsingHeader("Shortcuts", flags ))
        {
            Action_Manager*  action_manager = get_action_manager();
            action_manager_view_draw(action_manager);
        }

#if TOOLS_POOL_ENABLE
        if ( tools_cfg->runtime_debug && ImGui::CollapsingHeader("Pool"))
        {
            ImGui::Text("Pool stats:");
            auto pool = get_pool_manager()->get_pool();
            ImGui::Text(" - Node.................... %8zu", pool->get_all<Node>().size() );
            ImGui::Text(" - Node_View................ %8zu", pool->get_all<Node_View>().size() );
            ImGui::Text(" - Physics................. %8zu", pool->get_all<Physics>().size() );
            ImGui::Text(" - Scope................... %8zu", pool->get_all<Scope>().size() );
        }
#endif
    }
    ImGui::End();
}

void ndbl::nodable_draw_toolbar_window(App_State* app)
{
    Config*      cfg  = get_config();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 5.0f});

    if ( ImGui::Begin( cfg->ui_toolbar_window_label, NULL, flags ) )
    {
        Font_Manager*  font_manager  = get_font_manager();
        Event_Manager* event_manager = get_event_manager();
        const Vec2&   button_size   = cfg->ui_toolButton_size;

        ImGui::PopStyleVar();
        ImGui::PushFont(font_manager->get_font(Font_Slot_ToolBtn));
        ImGui::BeginGroup();

        // reset
        if (ImGui::Button(ICON_FA_UNDO " regen. graph", button_size)) {
            event_manager->dispatch( Event_ID_RESET_GRAPH );
        }
        ImGui::SameLine();

        // enter isolation mode
        bool isolation_on = cfg->isolation & Isolation_ON;
        if (ImGui::Button(isolation_on ? ICON_FA_CROP " isolation mode: ON " : ICON_FA_CROP " isolation mode: OFF", button_size))
        {
            event_manager->dispatch( Event_ID_TOGGLE_ISOLATION_FLAGS );
        }
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::PopFont();
    }
    ImGui::End();
}

