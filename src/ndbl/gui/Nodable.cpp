#include "Nodable.h"
#include "IconsFontAwesome5.h"
#include "ImGuiColorTextEdit/TextEditor.h"
#include "core/Event.h"
#include "core/Graph.h"
#include "gui/Action_Manager.h"
#include "gui/App.h"
#include "gui/Nodable_View.h"
#include "gui/Scope_View.h"

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
#include "tools/gui/Action_Manager_View.h"
#include "tools/gui/App_View.h"

#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Slot.h"
#include "ndbl/core/language/Nodlang.h"

#include "commands/Cmd_Connect.h"
#include "commands/Cmd_Disconnect.h"
#include "commands/Cmd_Group.h"

#include "Node_Slot_View.h"
#include "Config.h"
#include "Event.h"
#include "File.h"
#include "File_View.h"
#include "Graph_View.h"
#include "History.h"

using namespace ndbl;
using namespace tools;

#if __EMSCRIPTEN__
static ndbl::App_State* g_nodable_state = nullptr; // The main loop needs a static function pointer to run, we have to grab App_View_State* globaly.
#endif

template<typename T>
static Function_Descriptor* create_variable_node_signature()
{
    static Function_Descriptor* descriptor = Function_Descriptor::create<T(T)>("variable");
    return descriptor;
}

void ndbl::nodable_init(App_State* app)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "init ...\n");

#if __EMSCRIPTEN__
    // Expose a global pointer
    ASSERT(g_nodable_state == nullptr);
    g_nodable_state = app;
#endif

    // Initialize config (must be done first)
    auto* cfg = init_config();

    // Create and init App_View
    auto* view = new ndbl::App_View_State();
    nodableview_init(view, app);    

    // Init App
    app_init_ex(&app->base, &view->base, cfg->tools_cfg ); // the pointers are owned by this class, base app just use them.
    app->language   = init_language();
    app->config     = cfg;

    // Add a bunch of new actions
    tools::Action_Manager* action_manager = action_manager_get();
    ASSERT(action_manager != nullptr); // Should have been initialized by tools::App_View
    // (With shortcut)
    action_manager_add_action( action_manager, "Delete", Event_Data__User(Event_Code_DELETE), Shortcut{SDLK_DELETE, KMOD_NONE } );
    action_manager_add_action( action_manager, "Arrange", Event_Data__User( Event_Code_ARRANGE_SELECTION ), Shortcut{SDLK_a, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager_add_action( action_manager, "Fold", Event_Data__User( Event_Code_ARRANGE_SELECTION ), Shortcut{SDLK_x, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager_add_action( action_manager, "Next", Event_Data__User( Event_Code_SELECT_NEXT ), Shortcut{SDLK_n, KMOD_NONE } );
    action_manager_add_action( action_manager, ICON_FA_SAVE " Save", Event_Type_FILE_SAVE,  Shortcut{SDLK_s, KMOD_CTRL } );
    action_manager_add_action( action_manager, ICON_FA_SAVE " Save as", Event_Type_FILE_SAVE_AS, Shortcut{SDLK_s, KMOD_CTRL } );
    action_manager_add_action( action_manager, ICON_FA_TIMES "  Close", Event_Type_FILE_CLOSE, Shortcut{SDLK_w, KMOD_CTRL } );
    action_manager_add_action( action_manager, ICON_FA_FOLDER_OPEN " Open", Event_Type_FILE_BROWSE, Shortcut{SDLK_o, KMOD_CTRL } );
    action_manager_add_action( action_manager, ICON_FA_FILE " New", Event_Type_FILE_NEW, Shortcut{SDLK_n, KMOD_CTRL } );
    action_manager_add_action( action_manager, "Splashscreen", Event{Event_Type_WINDOW, Event_Data__Window{"splashscreen" }}, Shortcut{SDLK_F1 } );
    action_manager_add_action( action_manager, ICON_FA_SIGN_OUT_ALT " Exit", Event_Type_REQUEST_EXIT, Shortcut{SDLK_F4, KMOD_ALT } );
    action_manager_add_action( action_manager, "Undo", Event_Type_UNDO, Shortcut{SDLK_z, KMOD_CTRL } );
    action_manager_add_action( action_manager, "Redo"               , Event_Type_REDO, Shortcut{SDLK_y, KMOD_CTRL } );
    action_manager_add_action( action_manager, "Isolation"          , Event_Data__User(Event_Code_TOGGLE_ISOLATION_FLAGS), Shortcut{SDLK_i, KMOD_CTRL }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_TEXT_EDITOR );
    action_manager_add_action( action_manager, "Drag whole graph"   , Event_Data__User(Event_Code_MOVE_SELECTION), Shortcut{SDLK_SPACE, KMOD_NONE, "Space + Drag" }, Condition_ENABLE | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager_add_action( action_manager, "Frame Selection"    , Event_Data__User(Event_Code_REQUEST_FRAME_SELECTION), Shortcut{SDLK_f, KMOD_NONE }, Condition_ENABLE_IF_HAS_SELECTION | Condition_HIGHLIGHTED_IN_GRAPH_EDITOR );
    action_manager_add_action( action_manager, "Frame All"          , Event_Data__User(Event_Code_REQUEST_FRAME_SELECTION), Shortcut{SDLK_f, KMOD_LCTRL });
    // (to create block nodes)
    action_manager_add_action( action_manager, ICON_FA_CODE " Condition"    , Event_Data__User(Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_BLOCK_CONDITION )) );
    action_manager_add_action( action_manager, ICON_FA_CODE " For Loop"     , Event_Data__User(Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_BLOCK_FOR_LOOP) ));
    action_manager_add_action( action_manager, ICON_FA_CODE " While Loop"   , Event_Data__User(Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_BLOCK_WHILE_LOOP)) );
    action_manager_add_action( action_manager, ICON_FA_CODE " Scope"        , Event_Data__User(Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_BLOCK_SCOPE )) );
    action_manager_add_action( action_manager, ICON_FA_CODE " Entry Point"  , Event_Data__User(Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_ROOT)) );
    // (misc)
    action_manager_add_action( action_manager, ICON_FA_CODE " Return Statement", Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_RETURN )));
    // (to create variables)
    action_manager_add_action( action_manager, ICON_FA_DATABASE " Boolean Variable" , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_VARIABLE_BOOLEAN, create_variable_node_signature<bool>() )) );
    action_manager_add_action( action_manager, ICON_FA_DATABASE " Double Variable"  , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_VARIABLE_DOUBLE, create_variable_node_signature<double>() )) );
    action_manager_add_action( action_manager, ICON_FA_DATABASE " Integer Variable" , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_VARIABLE_INTEGER, create_variable_node_signature<int>() )) );
    action_manager_add_action( action_manager, ICON_FA_DATABASE " String Variable"  , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_VARIABLE_STRING, create_variable_node_signature<std::string>() )) );
    //(to create literals)
    action_manager_add_action( action_manager, ICON_FA_FILE " Boolean Literal"  , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_LITERAL_BOOLEAN, create_variable_node_signature<bool>() )));
    action_manager_add_action( action_manager, ICON_FA_FILE " Double Literal"   , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_LITERAL_DOUBLE, create_variable_node_signature<double>() )));
    action_manager_add_action( action_manager, ICON_FA_FILE " Integer Literal"  , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_LITERAL_INTEGER, create_variable_node_signature<int>() )));
    action_manager_add_action( action_manager, ICON_FA_FILE " String Literal"   , Event_Data__User( Event_Code_REQUEST_CREATE_NODE, new Event_Data__Create_Node(Create_Node_Type_LITERAL_STRING, create_variable_node_signature<std::string>() )));
    // (to create functions/operators from the API)
    // TODO: add a list of preset to create operators/functions
    // action_manager_add_action( action_manager, label.c_str(), Shortcut{}, EventPayload_CreateNode{Create_Node_Type_FUNCTION, invokable->get_sig() } );

    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "init " TOOLS_OK "\n");
}

void ndbl::nodable_deinit(App_State* app)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "_handle_deinit ...\n");

    // Deinit and release files
    for( File* each_file : app->files )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::App", "Delete file %s ...\n", each_file->path.c_str());
        file_deinit(each_file);
        delete each_file;
    }

    // Shutdown managers & co.
    shutdown_language(app->language);
    nodableview_deinit(app->view()); delete app->view();
    tools::app_deinit(&app->base);
    ndbl::shutdown_config(app->config);

#if __EMSCRIPTEN__
    g_nodable_state = nullptr;
#endif

    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "_handle_deinit " TOOLS_OK "\n");
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
    tools::app_update(&app->base);

    nodableview_update(app->view());

    // Delete flagged files
    for( File* file : app->files_to_delete )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "Nodable", "Delete files flagged to delete: %s\n", file_filename(file).c_str());
        file_deinit(file);
        delete file;
    }
    app->files_to_delete.clear();

    // Update current file
    if (app->current_file)
    {
        file_update(app->current_file, app->config->has_flags(Config_Flag_ISOLATION_ON));
    }

    // Handle events
    //--------------
    
    // Nodable events
    Event           event = {};
    Event_Manager*  event_manager       = event_manager_get();
    Graph_View*     graph_view          = nullptr; 
    History*        curr_file_history   = nullptr;

    if ( app->current_file )
    {
        graph_view        = componentbag_get<Graph_View>(&app->current_file->graph->component_bag); // TODO: should be included in the event?
        curr_file_history = &app->current_file->history; // TODO: should be included in the event?
    } 

    while( (event = event_manager_poll_event(event_manager)) )
    {
        switch ( event.type )
        {
            case Event_Type_REQUEST_EXIT:
            {
                tools::app_request_stop(&app->base);
                break;
            }

            case Event_Type_FILE_CLOSE:
            {
                nodable_close_file(app);
                break;
            }
            case Event_Type_UNDO:
            {
                if(curr_file_history) curr_file_history->undo();
                break;
            }

            case Event_Type_REDO:
            {
                if(curr_file_history) curr_file_history->redo();
                break;
            }

            case Event_Type_FILE_BROWSE:
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

            case Event_Type_FILE_NEW:
            {
                nodable_new_file(app);
                break;
            }

            case Event_Type_FILE_SAVE_AS:
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

            case Event_Type_FILE_SAVE:
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

            case Event_Type_WINDOW:
            {
                if ( strcmp(event.window.window_id, "splashscreen") == 0 )
                {
                    app->view()->base.show_splashscreen = event.window.visible;
                }
                break;
            }

            case Event_Type_FILE_OPENED:
            {
                ASSERT(app->current_file != nullptr );
                fileview_clear_overlay(&app->current_file->view);
                fileview_refresh_overlay(&app->current_file->view, Condition_ENABLE_IF_HAS_NO_SELECTION );
                break;
            }

            case Event_Type_USER:
            {
                // Note: these events might require some memory management

                switch( event.user.code )
                {
                    case Event_Code_RESET_GRAPH_VIEW:
                    {
                        Graph_View* graph_view = graph_component<Graph_View>(app->current_file->graph);
                        graph_view->flags |= Graph_View_Flag_NEEDS_TO_BE_RESET | Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
                        break;
                    }

                    case Event_Code_TOGGLE_ISOLATION_FLAGS:
                    {
                        app->config->flags ^= Config_Flag_ISOLATION_ON;
                        if(app->current_file)
                        {
                            app->current_file->set_flags(File_Flag_GRAPH_IS_DIRTY);
                        }
                        break;
                    }

                    case Event_Code_REQUEST_FRAME_SELECTION:
                    {
                        if( !graph_view ) break;
                        graph_view->flags |= Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
                        break;
                    }

                    case Event_Code_DELETE:
                    {
                        for( const View& selected_item : graph_view->selection )
                        {
                            switch ( selected_item.type )
                            {
                                case View_Type_NODE:    { graph_flag_node_to_delete(selected_item.nodeview->node(), Graph_Flag_NONE);                       break; }
                                case View_Type_SCOPE:   { graph_flag_node_to_delete(selected_item.scopeview->scope->node(), Graph_Flag_ALLOW_SIDE_EFFECTS); break; }
                            }
                        }
                        break;
                    }

                    case Event_Code_ARRANGE_SELECTION:
                    {
                        for( const View& selected_item : graph_view->selection )
                        {
                            switch ( selected_item.type )
                            {
                                case View_Type_NODE:    { nodeview_arrange_recursively(selected_item.nodeview);   break; }
                                case View_Type_SCOPE:   { scopeview_arrange_content(selected_item.scopeview);     break; }
                            }
                        }
                        break;
                    }

                    case Event_Code_SELECT_NEXT:
                    {
                        if(!graph_view)
                        {
                            break;
                        }

                        graph_view->selection.clear();

                        // Append all the successors to the selection
                        for(View& selected_item : graph_view->selection )
                            if (selected_item.type == View_Type_NODE)
                                for (Node* _successor : selected_item.nodeview->node()->flow_outputs() )
                                    if (Node_View* _successor_view = node_component<Node_View>(_successor) )
                                        graph_view->selection.push_back( _successor_view );
                        break;
                    }

                    case Event_Code_TOGGLE_FOLDING:
                    {
                        for(View& selected_item : graph_view->selection)
                            if (selected_item.type == View_Type_NODE)
                                nodeview_toggle_expandcollapse( selected_item.nodeview );
                        break;
                    }

                    case Event_Code_SLOT_DROPPED:
                    {
                        ASSERT(curr_file_history != nullptr);
                        auto tail = static_cast<Node_Slot*>(event.user.data1);
                        auto head = static_cast<Node_Slot*>(event.user.data2);
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

                    case Event_Code_DELETE_LINK:
                    {
                        ASSERT(curr_file_history != nullptr);
                        auto tail = static_cast<Node_Slot*>(event.user.data1);
                        auto head = static_cast<Node_Slot*>(event.user.data2);
                        auto command = std::make_shared<Cmd_DeleteEdge>(tail, head);
                        curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(command));
                        break;
                    }

                    case Event_Code_DELETE_ALL_LINKS:
                    {
                        ASSERT(curr_file_history != nullptr);
                        auto slot = static_cast<Node_Slot*>(event.user.data1);

                        auto cmd_grp = std::make_shared<Cmd_Group>("Disconnect All Edges");
                        for(Node_Slot* adjacent_slot : slot->adjacent )
                        {
                            auto each_cmd = std::make_shared<Cmd_DeleteEdge>(slot, adjacent_slot );
                            cmd_grp->push_cmd( std::static_pointer_cast<AbstractCommand>(each_cmd) );
                        }
                        curr_file_history->push_command(std::static_pointer_cast<AbstractCommand>(cmd_grp));
                        break;
                    }

                    case Event_Code_REQUEST_CREATE_NODE:
                    {
                        auto event_data = static_cast<Event_Data__Create_Node*>(event.user.data1);

                        Graph* graph = graph_view->graph();
                        
                        // 1) create the node
                        if ( !graph_root(graph) )
                        {
                            TOOLS_LOG(tools::Verbosity_Error, "Nodable", "Unable to create_new primary_child, no root found on this graph.\n");
                            continue;
                        }

                        Node* new_node  = graph_create_node(graph,
                                                            event_data->node_type,
                                                            event_data->node_signature,
                                                            graph_root_scope(graph) );

                        // Insert an end of line and end of instruction
                        switch ( event_data->node_type )
                        {
                            case Create_Node_Type_BLOCK_CONDITION:
                            case Create_Node_Type_BLOCK_FOR_LOOP:
                            case Create_Node_Type_BLOCK_WHILE_LOOP:
                            case Create_Node_Type_BLOCK_SCOPE:
                            case Create_Node_Type_ROOT:
                                new_node->suffix = Token::s_end_of_line;
                                break;
                            case Create_Node_Type_VARIABLE_BOOLEAN:
                            case Create_Node_Type_VARIABLE_DOUBLE:
                            case Create_Node_Type_VARIABLE_INTEGER:
                            case Create_Node_Type_VARIABLE_STRING:
                            case Create_Node_Type_RETURN:
                                new_node->suffix = Token::s_end_of_instruction;
                                break;
                            case Create_Node_Type_LITERAL_BOOLEAN:
                            case Create_Node_Type_LITERAL_DOUBLE:
                            case Create_Node_Type_LITERAL_INTEGER:
                            case Create_Node_Type_LITERAL_STRING:
                            case Create_Node_Type_FUNCTION:
                                break;
                            default:
                                TOOLS_UNREACHABLE("Unexpected node_type: %i\n", event_data->node_type);
                        }

                        // 2) handle connections
                        if ( Node_Slot_View* slot_view = event_data->active_slotview )
                        {
                            Node_Slot::Flags        complementary_flags = node_slot_flags_toggle_order(slot_view->slot->type_and_order());
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

                                graph_connect(out, in, Graph_Flag_ALLOW_SIDE_EFFECTS );

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
                        if ( auto view = node_component<Node_View>(new_node) )
                        {
                            spatialnode_set_position(&view->shape.spatial_node, event_data->desired_screen_pos, WORLD_SPACE);
                            graph_view->selection.clear();
                            graph_view->selection.push_back(view);
                        }

                        // The data associated live during the whole program
                        // delete event_data;
                        // event.user.data1 = nullptr;
                        break;
                    }
                }

            }

            default:
            {
                TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "App", "Ignoring and event, this case is not handled\n");
            }
        }
    }
}

void ndbl::nodable_draw(App_State* app)
{
    nodableview_draw(app->view());
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

    file_init(file);
    
    // Currently, we rely on the LanguageDefinition provided by the text editor to perform syntax highlighting
    // In case we want to handle different languages, we'll have to do some detection here.
    // Right now, I'll always use C language definition.
    //
    static TextEditor::LanguageDefinition c_lang_def = TextEditor::LanguageDefinition::C();
    file->view.text_editor.SetLanguageDefinition( c_lang_def );

    if ( file_read(file, _path ) )
    {
        return nodable_add_file(app, file);
    }

    file_deinit(file);
    delete file;
    TOOLS_LOG(tools::Verbosity_Error, "File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
    return nullptr;
}

File* ndbl::nodable_add_file(App_State* app, File* _file)
{
    VERIFY(_file, "File is nullptr");
    app->files.push_back( _file );
    app->current_file = _file;
    event_manager_dispatch( event_manager_get(), Event_Type_FILE_OPENED );
    return _file;
}

void ndbl::nodable_save_file(const App_State* app, File* file)
{
    VERIFY(file, "file must be defined");

	if ( !file_write(file, file->path) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "ndbl::App", "Unable to save %s (%s)\n", file_filename(file).c_str(), file->path.c_str());
        return;
    }
    TOOLS_LOG(tools::Verbosity_Message, "ndbl::App", "File saved: %s\n", file->path.c_str());
}

void ndbl::nodable_save_file_as(const App_State* app, File* file, const tools::Path& _path)
{
    if ( !file_write(file, _path) )
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
    auto it = std::find(app->files.begin(), app->files.end(), _file);
    VERIFY(it != app->files.end(), "Unable to find the file in the loaded_files");
    it = app->files.erase(it);
    app->files_to_delete.push_back(_file);

    // Switch to the next file if possible
    if ( it != app->files.end() )
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
    app->current_file->set_flags(File_Flag_GRAPH_IS_DIRTY);
}

File*ndbl::nodable_new_file(App_State* app)
{
    app->untitled_file_count++;

    String_32 name;
    name.append_fmt("Untitled_%i.cpp", app->untitled_file_count);
    auto* file = new File();
    file_init(file);
    file->path = name.c_str();

    return nodable_add_file(app, file);
}

bool ndbl::nodable_should_stop(const App_State* app)
{
    return app_should_stop(&app->base);
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
