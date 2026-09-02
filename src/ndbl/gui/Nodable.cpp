#include "Nodable.h"
#include "IconsFontAwesome5.h"
#include "ImGuiColorTextEdit/TextEditor.h"
#include "gui/Command.h"
#include "gui/View.h"
#include "tools/core/Event.h"
#include "tools/core/Flags.h"
#include "ndbl/core/Graph.h"
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

#include "Node_Slot_View.h"
#include "Config.h"
#include "Event.h"
#include "File.h"
#include "File_View.h"
#include "Graph_View.h"
#include "Command_Manager.h"

// private
namespace ndbl
{
    static App_State* g_app = {}; // The main loop needs a static function pointer to run, we have to grab App_View_State* globaly.
};

#define VERIFY_NODABLE_IS_INITIALIZED() VERIFY(g_app != nullptr, "Nodable is not initialized, did you call nodable_init() ?")

template<typename Type>
static Type_Descriptor* create_variable_node_signature()
{
    static Type_Descriptor* descriptor = type_create<Type(Type)>("variable");
    return descriptor;
}

ndbl::App_State* ndbl::app_state()
{
    VERIFY_NODABLE_IS_INITIALIZED();
    return g_app;
}

ndbl::App_State* ndbl::app_init()
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "ndbl::app_init() ...\n");

    // Expose a global pointer
    ASSERT(g_app == nullptr);
    g_app = bdc::memory_new<App_State>();

    // Initialize config (must be done first)
    auto* cfg = config_init();
    g_app->config     = cfg;

    // Create and init App_View
    auto* view = ndbl::appview_init();   

    // Init App
    tools::app_init_ex(&g_app->base, &view->base, cfg->tools_cfg ); // the pointers are owned by this class, base app just use them.
    
    language_init();


    // Init manager(s)
    ndbl::command_manager_init();

    // Add actions from config
    for(const Action& action : cfg->actions)
    {
        action_manager_register_action(action);
    }

    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "init " TOOLS_OK "\n");

    return g_app;
}

void ndbl::app_shutdown()
{
    App_State* app = app_state();

    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "_handle_deinit ...\n");

    // Deinit and release files
    for( File* each_file : app->files )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::App", "Delete file %s ...\n", each_file->path.c_str());
        file_deinit(each_file);
        bdc::memory_delete(each_file);
    }

    // Shutdown managers & co.
    ndbl::command_manager_shutdown();
    ndbl::language_shutdown();
    tools::app_shutdown();
    ndbl::appview_shutdown();
    ndbl::config_shutdown();

    bdc::memory_delete(g_app);
    g_app = nullptr;

    TOOLS_LOG(tools::Verbosity_Diagnostic, "ndbl::Nodable", "_handle_deinit " TOOLS_OK "\n");
}

void ndbl::app_do_frame()
{
    bdc::temp_allocator_buffer_reset(); // the intend of a temporary allocator, is to use data quickly after allocation, we want to clear that buffer at the begining of each frame.
    app_update();
    app_draw();
}

void ndbl::app_run()
{
    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(&ndbl::app_do_frame, 0, true);
    #else
        while( !app_should_stop() )
        {
            app_do_frame();
        }    
    #endif
}

void ndbl::app_update()
{
    auto app = app_state();

    tools::app_update();
    ndbl::appview_update();

    // Delete flagged files
    for( File* file : app->files_to_delete )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "Nodable", "Delete files flagged to delete: %s\n", file->name.data );
        file_deinit(file);
        bdc::memory_delete(file);
    }
    app->files_to_delete.clear();

    // Update current file
    if (app->current_file)
    {
        file_update(app->current_file, HAS_FLAGS(app->config->flags, Config_Flag_ISOLATION_ON) );
    }

    // Handle events
    //--------------
    
    // Nodable events
    Event           event = {};
    Graph_View*     graph_view          = nullptr; 

    if ( app->current_file )
    {
        graph_view = app->current_file->graph->view; // Q&A: Should be included in the event? No, because a event applies on current context, adnd the history can mutate the context via Commands.
    } 

    while( (event = event_manager_pop_event()) )
    {
        switch ( event.type )
        {
            case Event_Type_REQUEST_EXIT:
            {
                tools::app_request_stop();
                break;
            }

            case Event_Type_FILE_CLOSE:
            {
                app_close_file();
                break;
            }
            case Event_Type_UNDO:
            {
                command_manager_undo();
                break;
            }

            case Event_Type_REDO:
            {
                command_manager_redo();
                break;
            }

            case Event_Type_FILE_BROWSE:
            {
                Path path;
                if( pick_file_path(path, Dialog_Type_Browse) )
                {
                    app_open_file(path);
                    break;
                }
                TOOLS_LOG(tools::Verbosity_Diagnostic, "App", "Browse file aborted by user.\n");
                break;

            }

            case Event_Type_FILE_NEW:
            {
                app_new_file();
                break;
            }

            case Event_Type_FILE_SAVE_AS:
            {
                if (app->current_file != nullptr)
                {
                    Path path;
                    if( pick_file_path(path, Dialog_Type_SaveAs))
                    {
                       app_save_file_as(app->current_file, path);
                    }
                }

                break;
            }

            case Event_Type_FILE_SAVE:
            {
                if (!app->current_file) break;
                if( !app->current_file->path.empty())
                {
                    app_save_file(app->current_file);
                }
                else
                {
                    Path path;
                    if( pick_file_path(path, Dialog_Type_SaveAs))
                    {
                        app_save_file_as(app->current_file, path);
                    }
                }
                break;
            }

            case Event_Type_TOGGLE_HELP:
            {
                appview()->base.show_splashscreen ^= true;
                break;
            }

            case Event_Type_FILE_OPENED:
            {
                ASSERT(app->current_file != nullptr );
                fileview_clear_overlay(&app->current_file->view);
                fileview_refresh_overlay(&app->current_file->view, Condition_ENABLE_IF_HAS_NO_SELECTION );
                break;
            }

            case Event_Type_RESET_GRAPH_VIEW:
            {
                graph_view->flags |= Graph_View_Flag_NEEDS_TO_BE_RESET | Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
                break;
            }

            case Event_Type_TOGGLE_ISOLATION_FLAGS:
            {
                app->config->flags ^= Config_Flag_ISOLATION_ON;
                if(app->current_file)
                {
                    app->current_file->set_flags(File_Flag_GRAPH_IS_DIRTY);
                }
                break;
            }

            case Event_Type_FRAME_SELECTION:
            {
                if( !graph_view ) break;
                graph_view->flags |= Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
                break;
            }

            case Event_Type_DELETE:
            {
                for( const View& selected_item : graph_view->selection )
                {
                    switch ( selected_item.type )
                    {
                        case View_Type_NODE:    { graph_flag_node_to_delete(selected_item.nodeview->node, Graph_Flag_NONE);                       break; }
                        case View_Type_SCOPE:   { graph_flag_node_to_delete(selected_item.scopeview->scope->node, Graph_Flag_ALLOW_SIDE_EFFECTS); break; }
                    }
                }
                break;
            }

            case Event_Type_RESET_LAYOUT:
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

            case Event_Type_SELECT_NEXT:
            {
                if(!graph_view)
                {
                    break;
                }

                view_selection_clear(&graph_view->selection);

                // Append all the successors to the selection
                for(View& selected_item : graph_view->selection )
                    if (selected_item.type == View_Type_NODE)
                        for (Node* successor_node : selected_item.nodeview->node->flow_outputs() )
                            if ( successor_node->view )
                                view_selection_add( &graph_view->selection, successor_node->view );
                break;
            }

            case Event_Type_TOGGLE_FOLDING:
            {
                for(View& selected_item : graph_view->selection)
                    if (selected_item.type == View_Type_NODE)
                        nodeview_toggle_expandcollapse( selected_item.nodeview );
                break;
            }

            case Event_Type_SLOT_DROPPED_ONTO_ANOTHER:
            {
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
                Command cmd = command_connect({tail, head});
                command_manager_push_command(cmd);

                break;
            }

            case Event_Type_DELETE_LINK:
            {
                auto tail = static_cast<Node_Slot*>(event.user.data1);
                auto head = static_cast<Node_Slot*>(event.user.data2);
                Command cmd = command_disconnect({tail, head});
                command_manager_push_command(cmd);
                break;
            }

            case Event_Type_DELETE_ALL_LINKS:
            {
                auto slot = static_cast<Node_Slot*>(event.user.data1);

                command_manager_begin_transaction();
                for(Node_Slot* adjacent_slot : slot->adjacent )
                {
                    Command disconnect_cmd = command_disconnect({slot, adjacent_slot});
                    command_manager_push_command( disconnect_cmd );
                }
                command_manager_end_transaction();
                break;
            }

            case Event_Type_NEW_NODE:
            {
                command_manager_begin_transaction();

                auto event_data = static_cast<Event_Data__Create_Node*>(event.user.data1);

                // 1) Create Node
                Command cmd_new_node = command_new_node({ &event_data->node_state });
                command_manager_push_command(cmd_new_node);                        
                Node* new_node = graph_get_latest_created_node(app->current_file->graph);

                // 2) clear selection and select the new node
                //    TODO: replace selection (from current to the new_node's view)
                if ( new_node->view )
                {
                    View_Selection new_selection;
                    view_selection_add(&new_selection, new_node->view);

                    Command cmd_selection_change = command_selection_change(&new_selection);
                }

                // 3) Set Node_View position
                spatialnode_set_position(&new_node->view->shape.spatial_node, event_data->desired_screen_pos, WORLD_SPACE);

                // 4) Connect the new node to the code flow if a slot is being dragged
                if ( Node_Slot_View* slot_view = event_data->active_slotview )
                {
                    Node_Slot::Flags        complementary_flags = node_slot_flags_toggle_order(slot_view->slot->type_and_order());
                    const Type_Descriptor*  type                = slot_view->property()->type;
                    Node_Slot*              complementary_slot  = node_find_slot_by_property_type(new_node, complementary_flags, type);

                    ASSERT(complementary_slot != nullptr); // TODO: this case should not happens, instead we should check ahead of time whether or not this not can be attached

                    Node_Slot* out = slot_view->slot;
                    Node_Slot* in  = complementary_slot;

                    if ( HAS_FLAGS(out->flags, Node_Slot::Flag_ORDER_2ND ) )
                    {
                        std::swap( out, in );
                    }

                    Command cmd_connect = command_connect({ out, in});
                    command_manager_push_command(cmd_connect);

                    // Ensure has a "\n" when connecting using CODEFLOW (to split lines)
                    if (node_is_instruction(out->node ) && out->type() == Node_Slot::Flag_TYPE_FLOW )
                    {
                        if ( bdc::string_rfind( out->node->suffix.view(), '\n') == bdc::String::invalid_pos )
                        {
                            out->node->suffix.suffix_push_back("\n");
                        }
                    }
                }

                command_manager_end_transaction();
                break;
            }

            default:
            {
                TOOLS_UNREACHABLE("Unexpected Event_Type %i\n", event.type);
            }
        }
    }
}

void ndbl::app_draw()
{
    appview_draw();
}

File* ndbl::app_open_asset_file(const tools::Path& path)
{
    auto app = app_state();

    if ( path.is_absolute() )
        return app_open_file(path);

    return app_open_file(Path::absolute(path) );
}

File* ndbl::app_open_file(const tools::Path& _path)
{
    auto app = app_state();

    File* file = bdc::memory_new<File>();
    file_init(file);
    
    // Currently, we rely on the LanguageDefinition provided by the text editor to perform syntax highlighting
    // In case we want to handle different languages, we'll have to do some detection here.
    // Right now, I'll always use C language definition.
    //
    static TextEditor::LanguageDefinition c_lang_def = TextEditor::LanguageDefinition::C();
    file->view.text_editor.SetLanguageDefinition( c_lang_def );

    if ( file_read(file, _path ) )
    {
        return app_add_file(file);
    }

    file_deinit(file);
    bdc::memory_delete(file);
    TOOLS_LOG(tools::Verbosity_Error, "File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
    return nullptr;
}

File* ndbl::app_add_file(File* file)
{
    auto app = app_state();
    VERIFY(file, "File is nullptr");
    app->files.push_back( file );
    app->current_file = file;
    event_manager_push_event( event_from_type(Event_Type_FILE_OPENED) );
    return file;
}

void ndbl::app_save_file(File* file)
{
    auto app = app_state();

    VERIFY(file, "file must be defined");

	if ( !file_write(file, file->path) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "ndbl::App", "Unable to save %s (%s)\n", file->name.data, file->path.c_str());
        return;
    }
    TOOLS_LOG(tools::Verbosity_Message, "ndbl::App", "File saved: %s\n", file->path.c_str());
}

void ndbl::app_save_file_as(File* file, const tools::Path& _path)
{
    if ( !file_write(file, _path) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "ndbl::App", "Unable to save %s (%s)\n", _path.filename().c_str(), _path.c_str());
        return;
    }
    TOOLS_LOG(tools::Verbosity_Message, "ndbl::App", "File saved: %s\n", _path.c_str());
}

void ndbl::app_close_file()
{
    auto app = app_state();

    if ( app->current_file == nullptr )
        return;

    app_close_file(app->current_file);
}
void ndbl::app_close_file(File* _file)
{
    auto app = app_state();

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

void ndbl::app_reset_current_graph()
{
    auto app = app_state();

    if( !app->current_file )
    {
        return;
    }

    // n.b. nodable is still text oriented
    app->current_file->set_flags(File_Flag_GRAPH_IS_DIRTY);
}

File* ndbl::app_new_file()
{
    using namespace bdc;

    auto app = app_state();

    app->untitled_file_count++;

    bdc::String temp_name = bdc::string_printf( bdc::temp_allocator(), "Untitled_%i.cpp", app->untitled_file_count);
    
    auto* file = bdc::memory_new<File>();
    file_init(file);
    file->path = temp_name.c_str();

    return app_add_file(file);
}

bool ndbl::app_should_stop()
{
    return tools::app_should_stop();
}

void ndbl::app_set_current_file(File* file)
{
    auto app = app_state();
    
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
