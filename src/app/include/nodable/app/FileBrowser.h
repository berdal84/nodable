#pragma once

namespace Nodable
{
    /**
     * Placeholder, to implement
     */
    class FileBrowser
    {
    public:
        FileBrowser(){}
        ~FileBrowser(){}

        bool HasSelected() {
            return false;
        }

        bool Display() {
            return false;
        }

        const std::vector<std::string>& GetMultiSelected() {
            return m_selected;
        }

        void ClearSelected() {
            m_selected.empty();
        }

        void Close() {}

        void Open() {}

    private:
        std::vector<std::string> m_selected;
    };
}