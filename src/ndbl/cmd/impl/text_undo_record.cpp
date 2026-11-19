#include "ndbl/cmd/Command.h"

#include "ndbl/Nodable.h"
#include "ndbl/File.h"

namespace ndbl 
{

void _text_undo_record_do(Command*);
void _text_undo_record_undo(Command*);
void _text_undo_record_release(Command*);

inline auto* _text_undo_record_undo_payload_data(Command* cmd)
{
    return reinterpret_cast<Cmd_Data__Text_Undo_Record*>(cmd->payload.data.ptr);
}

Command make_cmd__text_undo_record(const Cmd_Data__Text_Undo_Record& text_undo_record)
{
    Command cmd{};

    cmd.proc_do             = _text_undo_record_do;
    cmd.proc_undo           = _text_undo_record_undo;
    cmd.proc_release        = _text_undo_record_release;
    cmd.payload.data.ptr    = memory_new<Cmd_Data__Text_Undo_Record>(bdc::allocator, text_undo_record);
    cmd.description         = "Wraps a Text_Editor's Undo Record";

    return cmd;
}

void _text_undo_record_do(Command* cmd)
{
    Text_Editor*                text_editor  = &app_state()->current_file->view.text_editor;
    Cmd_Data__Text_Undo_Record* payload_data = _text_undo_record_undo_payload_data(cmd);

    payload_data->undo_record.Redo(text_editor);
}

void _text_undo_record_undo(Command* cmd)
{
    Text_Editor*                text_editor  = &app_state()->current_file->view.text_editor;
    Cmd_Data__Text_Undo_Record* payload_data = _text_undo_record_undo_payload_data(cmd);

    payload_data->undo_record.Undo(text_editor);
}

void _text_undo_record_release(Command* cmd)
{
    memory_delete( _text_undo_record_undo_payload_data(cmd) );
    cmd->payload.data.ptr = nullptr;
}

} // namespace ndbl