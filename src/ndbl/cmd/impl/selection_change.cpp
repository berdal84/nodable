#include "ndbl/cmd/Command.h"

#include "ndbl/Nodable.h"
#include "ndbl/File.h"

namespace ndbl 
{

void _selection_change__release(Command*);
void _selection_change__do(Command*);
void _selection_change__undo(Command*);

struct Selection_Change
{
    bdc::Array<View>     old_selection;
    bdc::Array<View>     new_selection;
};

Command make_cmd__selection_change(const Array<View>& view_to_select)
{
    Command cmd{};

    cmd.proc_do          = _selection_change__do;
    cmd.proc_undo        = _selection_change__undo;
    cmd.proc_release     = _selection_change__release;

    View_Selection& current_selection = app_state()->current_file->view.graph_view->selection;

    auto payload_data = bdc::memory_new<Selection_Change>(); // we should allocate the command data in an Arena.
    payload_data->old_selection = array_copy(current_selection.items);
    payload_data->new_selection = array_copy(view_to_select);

    cmd.payload.data.ptr = payload_data;
    cmd.description      = "Selection change";

    return cmd;
}

inline auto* _selection_change_payload_data(Command* cmd)
{
    return reinterpret_cast<Selection_Change*>(cmd->payload.data.ptr);
}

void _selection_change__release(Command* cmd)
{
    auto payload_data =  _selection_change_payload_data(cmd);
    bdc::memory_free(payload_data->old_selection.data);
    bdc::memory_free(payload_data->new_selection.data);
    bdc::memory_delete(payload_data);    
    cmd->payload.data.ptr = nullptr;
}

inline void _selection_change__replace_selection(const Array<View>& items)
{
    View_Selection& current_selection = app_state()->current_file->view.graph_view->selection;
    viewselection_clear( current_selection );
    viewselection_add( current_selection, items);
}

void _selection_change__do(Command* cmd)
{
    _selection_change__replace_selection(
        _selection_change_payload_data(cmd)->new_selection
    );
}

void _selection_change__undo(Command* cmd)
{
    _selection_change__replace_selection(
        _selection_change_payload_data(cmd)->old_selection
    );
}

} // namespace ndbl