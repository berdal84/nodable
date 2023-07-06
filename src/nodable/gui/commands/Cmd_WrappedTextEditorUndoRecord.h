#pragma once
#include "gui/Command.h"

namespace ndbl
{
    /**
     * Command triggered when user modify text in the text editor.
     */
    class Cmd_WrappedTextEditorUndoRecord : public AbstractCommand
    {
    public:
        Cmd_WrappedTextEditorUndoRecord(
                TextEditor::UndoRecord& _undoRecord,
                TextEditor* _textEditor)
                : m_text_editor_undo_record(_undoRecord)
                , m_text_editor(_textEditor)
        {
            snprintf(m_description
                    , sizeof(m_description)
                    , "ReplaceText\n"
                      " - replaced: \"%s\"\n"
                      " - by: \"%s\"\n"
                    , m_text_editor_undo_record.mRemoved.c_str()
                    , m_text_editor_undo_record.mAdded.c_str() );
        }

        ~Cmd_WrappedTextEditorUndoRecord() override = default;

        void execute() override
        {
            m_text_editor_undo_record.Redo(m_text_editor);
        }

        void undo() override
        {
            m_text_editor_undo_record.Undo(m_text_editor);
        }

        const char* get_description() const override
        {
            return m_description;
        }

    private:
        TextEditor::UndoRecord m_text_editor_undo_record;
        TextEditor*            m_text_editor;
        char                   m_description[255];
    };
}