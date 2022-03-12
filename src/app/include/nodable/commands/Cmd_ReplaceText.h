#pragma once
#include <nodable/Command.h>

namespace Nodable
{
    /**
     * Command triggered when user modify text in the text editor.
     */
    class Cmd_ReplaceText : public IUndoableCmd
    {
    public:
        Cmd_ReplaceText(
                TextEditor::UndoRecord& _undoRecord,
                TextEditor* _textEditor)
                : m_text_editor_undo_record(_undoRecord)
                , m_text_editor(_textEditor)
        {
            sprintf(m_description
                    , "ReplaceText\n"
                      " - replaced: \"%s\"\n"
                      " - by: \"%s\"\n"
                    , m_text_editor_undo_record.mRemoved.c_str()
                    , m_text_editor_undo_record.mAdded.c_str() );
        }

        ~Cmd_ReplaceText() override = default;

        void execute() override
        {
            /* Function empty because the first execution is made my the text editor itself */
        }

        void redo() override
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