#pragma once
#include "gui/Command.h"
#include <string>
#include <TextEditor.h>

namespace ndbl
{
    /**
     * Command triggered when user modify text in the text editor.
     */
    class Cmd_ReplaceText : public AbstractCommand
    {
    public:
        Cmd_ReplaceText(
                const std::string& _old_content,
                const std::string& _new_content,
                TextEditor* _textEditor)
                : m_old_content(_old_content)
                , m_new_content(_new_content)
                , m_text_editor(_textEditor)
        {
            snprintf(m_description
                    , sizeof(m_description)
                    , "ReplaceText\n"
                      " - replaced: \"%s\"\n"
                      " - by: \"%s\"\n"
                    , m_old_content.c_str()
                    , m_new_content.c_str() );
        }

        ~Cmd_ReplaceText() override = default;

        void execute() override
        {
            m_text_editor->SetText(m_new_content);
        }

        void undo() override
        {
            m_text_editor->SetText(m_old_content);
        }

        const char* get_description() const override
        {
            return m_description;
        }

    private:
        const std::string      m_old_content;
        const std::string      m_new_content;
        TextEditor*            m_text_editor;
        char                   m_description[255];
    };
}