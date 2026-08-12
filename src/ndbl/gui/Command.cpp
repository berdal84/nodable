#include "Command.h"
#include "core/Graph.h"

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_group(const char* name)
{
    Command command = {
        .type           = Command_Type_GROUP,
        .description    = name,
        .proc_do        = command_group_do,
        .proc_redo      = command_group_redo,
        .proc_undo      = command_group_undo,
        .proc_release   = command_group_release,
    };

    command.group = {
        .subcommands = new std::vector<Command>()
    };

    return command;
}

void ndbl::command_group_release(Command* command)
{
    delete command->group.subcommands;
}

void ndbl::command_group_do(Command* command, Context* context)
{
    for(Command& subcommand : *command->group.subcommands )
    {
        subcommand.proc_do(&subcommand, context);
    }
}

void ndbl::command_group_undo(Command* command, Context* context)
{
    auto it = command->group.subcommands->rbegin();
    while( it != command->group.subcommands->rend() )
    {
        command_undo(&*it, context);
    }
}

void ndbl::command_group_redo(Command* command, Context* context)
{
    for(Command& subcommand : *command->group.subcommands )
    {
        subcommand.proc_redo(&subcommand, context);
    }
}

//-----------------------------------------------------------------------------

void ndbl::command_release(Command* command)
{
    if( command->proc_release != nullptr)
    {
        command->proc_release(command);
    }
}

void ndbl::command_do(Command* command, Context* context)
{
    command->proc_do(command, context);
}

void ndbl::command_redo(Command* command, Context* context)
{

    if( command->proc_redo != nullptr)
    {
        return command->proc_redo(command, context);
    }
    
    command->proc_do(command, context);
}

void ndbl::command_undo(Command* command, Context* context)
{
    command->proc_undo(command, context);
};

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_connect(Command_Data__Link link)
{
    Command command;
    command.type        = Command_Type_CONNECT;
    command.proc_do     = command_connect_do;
    command.proc_undo   = command_connect_undo;
    command.link        = link;
    command.description = "Connect two slots";

    return command;
}

void ndbl::command_connect_do(Command* command, Context* context)
{
    graph_connect( command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void ndbl::command_connect_undo(Command* command, Context* context)
{
    graph_disconnect(command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_disconnect(Command_Data__Link link)
{
    Command command;
    command.type        = Command_Type_DISCONNECT;
    command.proc_do     = command_disconnect_do;
    command.proc_undo   = command_disconnect_undo;
    command.link        = link;
    command.description = "Disconnect two slots";

    return command;
}

void ndbl::command_disconnect_do(Command* command, Context* context)
{
    graph_disconnect( command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void ndbl::command_disconnect_undo(Command* command, Context* context)
{
    graph_connect(command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_text_undo_record(Command_Data__Text_Undo_Record text_undo_record)
{
    Command command;
    command.type                = Command_Type_TEXT_UNDO_RECORD;
    command.proc_do             = command_text_undo_record_do;
    command.proc_undo           = command_text_undo_record_undo;
    command.text_undo_record    = text_undo_record;
    command.description         = "Wraps a TextEditor's Undo Record";

    return command;
}

void ndbl::command_text_undo_record_do(Command* command, Context* context)
{
    command->text_undo_record.undo_record->Redo(context->text_editor);
}

void ndbl::command_text_undo_record_undo(Command* command, Context* context)
{
    command->text_undo_record.undo_record->Undo(context->text_editor);
}

//-----------------------------------------------------------------------------

