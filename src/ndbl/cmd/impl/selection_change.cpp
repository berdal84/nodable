#include "ndbl/cmd/Command.h"

#include "ndbl/Nodable.h"
#include "ndbl/File.h"

namespace ndbl 
{

void _selection_change__release(Command*);
void _selection_change__do(Command*);
void _selection_change__undo(Command*);

Command make_cmd__selection_change(const Cmd_Data__Selection_Change& selection_change)
{
    Command cmd{};

    cmd.proc_do          = _selection_change__do;
    cmd.proc_undo        = _selection_change__undo;
    cmd.proc_release     = _selection_change__release;
    cmd.payload.data.ptr = cmd_selection_change_copy(bdc::memory_new<Cmd_Data__Selection_Change>(), &selection_change);
    cmd.description      = "Selection change";
    return cmd;
}

inline auto* _selection_change_payload_data(Command* cmd)
{
    return reinterpret_cast<Cmd_Data__Selection_Change*>(cmd->payload.data.ptr);
}

void _selection_change__release(Command* cmd)
{
    bdc::memory_delete( _selection_change_payload_data(cmd) );    
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