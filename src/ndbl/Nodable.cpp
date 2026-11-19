#include "Nodable.h"
#include "IconsFontAwesome5.h"
#include "ndbl/texteditor/Text_Editor.h"
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "bdc/Allocators.hpp"
#include "Action_Manager_View.h"
#include "Action_Manager.h"
#include "Asserts.h"
#include "cmd/Command_Manager.h"
#include "cmd/Command.h"
#include "Config.h"
#include "Event_Manager.h"
#include "Event.h"
#include "Event.h"
#include "File_View.h"
#include "File.h"
#include "Flags.h"
#include "Graph_View.h"
#include "Graph.h"
#include "Parser.h"
#include "Log.h"
#include "Nodable_View.h"
#include "Node_Slot_View.h"
#include "Node_Slot.h"
#include "Node.h"
#include "reflection/index.h"
#include "Scope_View.h"
#include "Task_Manager.h"
#include "View.h"

namespace ndbl
{

using namespace bdc;

static App_State* g_app = {}; // The main loop needs a static function pointer to run, we have to grab App_View_State* globaly.

#define VERIFY_NODABLE_IS_INITIALIZED() NDBL_ASSERT(g_app != nullptr, "Nodable is not initialized, did you call nodable_init() ?")

template<typename Type>
static Type_Descriptor* create_variable_node_signature()
{
    static Type_Descriptor* descriptor = type_create<Type(Type)>("variable");
    return descriptor;
}

App_State* app_state()
{
    VERIFY_NODABLE_IS_INITIALIZED();
    return g_app;
}

App_State* app_init()
{
    memory_manager_init(1024 * 1024 * 100); // must be first
    
    log_init();

    NDBL_LOG(Verbosity_Diagnostic, "Nodable", "app_init() ...\n");

    reflection_init();
    
    DEFINE_REFLECT(Node_View);
    DEFINE_REFLECT(Graph_View);

    // Expose a global pointer
    NDBL_ASSERT(g_app == nullptr);
    g_app = memory_new<App_State>();

    // Init manager(s)
    config_init();
    appview_init();
    langdef_init();
    task_manager_init();
    command_manager_init();

    // Add actions from config
    for(const Action& action : config()->actions)
    {
        action_manager_register_action(action);
    }

    NDBL_LOG(Verbosity_Diagnostic, "Nodable", "init " NDBL_OK "\n");

    return g_app;
}

void app_shutdown()
{
    App_State* app = app_state();

    NDBL_LOG(Verbosity_Diagnostic, "Nodable", "_handle_deinit ...\n");

    // Deinit and release files
    for( File* each_file : app->files )
    {
        NDBL_LOG(Verbosity_Diagnostic, "App", "Delete file %s ...\n", each_file->path.c_str());
        file_deinit(each_file);
        memory_delete(each_file);
    }

    // Shutdown managers & co.
    command_manager_shutdown();
    langdef_shutdown();
    config_shutdown();
    task_manager_shutdown();
    appview_shutdown();

    array_release(g_app->files);
    array_release(g_app->files_to_delete);

    memory_delete(g_app);
    g_app = nullptr;

    reflection_shutdown();
    NDBL_LOG(Verbosity_Diagnostic, "Nodable", "_handle_deinit " NDBL_OK "\n");
    
    log_shutdown(); // After that point, we can still use NDBL_LOG(), however messages won't be stored in memory.  

    #ifdef BDC_DEBUG_ALLOCATORS

        bdc::Memory_Manager_Report report;
        memory_manager_generate_report(&report);
        memory_manager_report_print(&report, { .asserts_no_leaks = true });
        NDBL_LOG( NDBL_MESSAGE, "Nodable.cpp", "Memory manager reported no leaks.\n");

    #endif

    memory_manager_shutdown(); // must be last
}

void app_do_frame()
{
    memory_manager_reset_temp_allocator_buffer(); // the intend of a temporary allocator, is to use data quickly after allocation, we want to clear that buffer at the begining of each frame.
    app_update();
    app_draw();
}

void app_run()
{
    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(&app_do_frame, 0, true);
    #else
        while( !app_should_stop() )
        {
            app_do_frame();
        }    
    #endif
}

void app_update()
{
    auto app = app_state();

    appview_update();
    task_manager_update();

    // Delete flagged files
    for( File* file : app->files_to_delete )
    {
        NDBL_LOG(Verbosity_Diagnostic, "Nodable", "Delete files flagged to delete: %s\n", file->name.data );
        file_deinit(file);
        memory_delete(file);
    }

    array_clear(app->files_to_delete);

    // Update current file
    if (app->current_file)
    {
        file_update(app->current_file, HAS_FLAGS( config()->flags, Config_Flag_ISOLATION_ON) );
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

    while( event = event_manager_pop_event(), event.type != Event_Type_NULL )
    {
        switch ( event.type )
        {            
            case Event_Type_EXIT_PROGRAM_REQUEST:
            {
                SET_FLAGS(app_state()->flags, App_Flag_SHOULD_STOP);
                break;
            }

            case Event_Type_TOGGLE_FULLSCREEN_REQUEST:
            {
                appview_set_fullscreen( !appview_is_fullscreen() );
                break;
            }

            case Event_Type_FILE_CLOSE_REQUEST:
            {
                app_close_current_file();
                break;
            }

            case Event_Type_UNDO_REQUEST:
            {
                command_manager_undo();
                break;
            }

            case Event_Type_REDO_REQUEST:
            {
                command_manager_redo();
                break;
            }

            case Event_Type_FILE_BROWSE_REQUEST:
            {
                Path path;
                if( appview_pick_file_path(path, Dialog_Type_Browse) )
                {
                    app_open_file(path);
                    break;
                }
                NDBL_LOG(Verbosity_Diagnostic, "App", "Browse file aborted by user.\n");
                break;

            }

            case Event_Type_FILE_NEW_REQUEST:
            {
                app_new_file();
                break;
            }

            case Event_Type_FILE_SAVE_AS_REQUEST:
            {
                if (app->current_file != nullptr)
                {
                    Path path;
                    if( appview_pick_file_path(path, Dialog_Type_SaveAs))
                    {
                       app_save_file_as(app->current_file, path);
                    }
                }

                break;
            }

            case Event_Type_FILE_SAVE_REQUEST:
            {
                if (!app->current_file)
                {
                    break;
                }

                if( !app->current_file->path.empty())
                {
                    app_save_file(app->current_file);
                    break;
                }

                Path path;
                if( appview_pick_file_path(path, Dialog_Type_SaveAs))
                {
                    app_save_file_as(app->current_file, path);
                }

                break;
            }

            case Event_Type_TOGGLE_HELP_REQUEST:
            {
                appview()->show_splashscreen ^= true;
                break;
            }

            case Event_Type_FILE_OPENED:
            {
                NDBL_ASSERT(app->current_file != nullptr );
                fileview_clear_overlay(&app->current_file->view);
                fileview_refresh_overlay(&app->current_file->view);
                break;
            }

            case Event_Type_RESET_GRAPH_VIEW_REQUEST:
            {
                // TODO: this should undoable
                graph_view->flags |= Graph_View_Flag_NEEDS_TO_BE_RESET | Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
                break;
            }

            case Event_Type_TOGGLE_ISOLATION_REQUEST:
            {
                // TODO: this should undoable
                config()->flags ^= Config_Flag_ISOLATION_ON;
                if(app->current_file)
                {
                    SET_FLAGS(app->current_file->flags, File_Flag_GRAPH_IS_DIRTY);
                }
                break;
            }

            case Event_Type_FRAME_SELECTION_REQUEST:
            {
                // TODO: this should undoable
                if( !graph_view ) break;
                graph_view->flags |= Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
                break;
            }

            case Event_Type_DELETE_REQUEST:
            {
                // TODO: this should undoable

                // TODO: code from graph view that needs to be integrated.
                //       this comes from the contextual menu, when deleting a Scope_View:
                // viewselection_clear(graphview->selection);
                // viewselection_add(graphview->selection, scopeview);
                // event_manager_push_event({ Event_Type_SELECTION_CHANGE_REQUEST, selection_change })

                for( const View& selected_item : graph_view->selection.items )
                {
                    switch ( selected_item.type )
                    {
                        case View_Type_NODE:    { graph_flag_node_to_delete(selected_item.nodeview->node        , Graph_Flag_NONE);               break; }
                        case View_Type_SCOPE:   { graph_flag_node_to_delete(selected_item.scopeview->scope->node, Graph_Flag_ALLOW_SIDE_EFFECTS); break; }
                    }
                }
                break;
            }

            case Event_Type_RESET_LAYOUT_REQUEST:
            {
                // TODO: this should undoable
                for( const View& selected_item : graph_view->selection.items )
                {
                    switch ( selected_item.type )
                    {
                        case View_Type_NODE:    { nodeview_arrange_recursively(selected_item.nodeview);   break; }
                        case View_Type_SCOPE:   { scopeview_arrange_content(selected_item.scopeview);     break; }
                    }
                }
                break;
            }

            case Event_Type_SELECT_NEXT_REQUEST:
            {
                if(!graph_view)
                {
                    break;
                }
                
                // make the list of successor(s)
                Resizable_Array<View> successors{};
                successors.allocator = &temp_allocator;
                for(View& selected_item : graph_view->selection.items )
                    if (selected_item.type == View_Type_NODE)
                        for (Node* successor_node : selected_item.nodeview->node->flow_outputs() )
                            if ( successor_node->view )
                                array_append(successors, successor_node->view );

                Command cmd = make_cmd__selection_change({
                    .old_selection = graph_view->selection.items,
                    .new_selection = successors
                });
                push_command(cmd);

                break;
            }

            case Event_Type_TOGGLE_FOLDING_REQUEST:
            {
                // TODO: this should undoable
                for(View& selected_item : graph_view->selection.items )
                    if (selected_item.type == View_Type_NODE)
                        nodeview_toggle_expandcollapse( selected_item.nodeview );
                break;
            }

            default:
            {
                NDBL_DEBUG_LOG( NDBL_WARNING, "Nodable.cpp","Event ignored (type= %u)\n", event.type );
                break;
            }
        }
    }
}

void app_draw()
{
    appview_draw();
}

File* app_open_asset_file(const Path& path)
{
    auto app = app_state();

    if ( path.is_absolute() )
        return app_open_file(path);

    return app_open_file(Path::absolute(path) );
}

File* app_open_file(const Path& _path)
{
    auto app = app_state();

    File* file = memory_new<File>();
    file_init(file);
    
    if ( file_read(file, _path ) )
    {
        return app_add_file(file);
    }

    file_deinit(file);
    memory_delete(file);
    NDBL_LOG(Verbosity_Error, "File", "Unable to open file %s (%s)\n", _path.filename().c_str(), _path.c_str());
    return nullptr;
}

File* app_add_file(File* file)
{
    auto app = app_state();
    NDBL_ASSERT(file, "File is nullptr");
    array_append(app->files, file);
    app->current_file = file;
    event_manager_push_event({ .type = Event_Type_FILE_OPENED, .data1 = file });
    return file;
}

void app_save_file(File* file)
{
    auto app = app_state();

    NDBL_ASSERT(file, "file must be defined");

	if ( !file_write(file, file->path) )
    {
        NDBL_LOG(Verbosity_Error, "App", "Unable to save %s (%s)\n", file->name.data, file->path.c_str());
        return;
    }
    NDBL_LOG(Verbosity_Message, "App", "File saved: %s\n", file->path.c_str());
}

void app_save_file_as(File* file, const Path& _path)
{
    if ( !file_write(file, _path) )
    {
        NDBL_LOG(Verbosity_Error, "App", "Unable to save %s (%s)\n", _path.filename().c_str(), _path.c_str());
        return;
    }
    NDBL_LOG(Verbosity_Message, "App", "File saved: %s\n", _path.c_str());
}

void app_close_current_file()
{
    auto app = app_state();

    if ( app->current_file == nullptr )
        return;

    app_close_file(app->current_file);
}

void app_close_file(File* file)
{
    assert(file != nullptr);

    auto app = app_state();

    // Find and delete the file
    auto found = array_find(app->files, file);
    NDBL_ASSERT(found.ok, "Unable to find the file in the loaded_files");
    array_remove_ordered(app->files, found.at_pos);
    array_append(app->files_to_delete, file);

    // Switch to the next file if possible
    // Note: why do I have a duplicated state? I could use an index?
    app->current_file = app->files.size ? array_back(app->files) : nullptr;
}

void app_reset_current_graph()
{
    auto app = app_state();

    if( !app->current_file )
    {
        return;
    }

    // n.b. nodable is still text oriented
    SET_FLAGS(app->current_file->flags, File_Flag_GRAPH_IS_DIRTY);
}

File* app_new_file()
{
    using namespace bdc;

    auto app = app_state();

    app->untitled_file_count++;

    String name = string_printf( "Untitled_%i.cpp", app->untitled_file_count);
    
    auto* file = memory_new<File>();
    file_init(file);
    file->path = name.c_str();
    file->name = name;

    return app_add_file(file);
}

bool app_should_stop()
{
    return HAS_FLAGS( app_state()->flags, App_Flag_SHOULD_STOP);
}

void app_set_current_file(File* file)
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

} // namespace ndbl
