#pragma once

#include <future>
#include <memory>
#include <string>

#include "tools/gui/BaseApp.h"

#include "Config.h"
#include "NodableView.h"
#include "types.h"

namespace ndbl
{
    class Nodlang;
    class VirtualMachine;
    class NodeFactory;
    class ComponentFactory;

    class Nodable : public tools::BaseApp
    {
	public:
        Nodable(): tools::BaseApp() {};
        ~Nodable() override {};

        File*           current_file = nullptr;

        // Common

        void            init();
        void            update() override;
        void            shutdown() override;

        // File related:

        File*           open_asset_file(const std::filesystem::path&_path);
        File*           open_file(const std::filesystem::path&_path);
        File*           new_file();
        void            save_file( File*pFile) const;
        void            save_file_as(const std::filesystem::path &_path) const;
        File*           add_file( File*_file);
        void            close_file( File*);
        bool            is_current(const File* _file ) const { return current_file == _file; }
        const std::vector<File*>&
                        get_files() const { return m_loaded_files; }
        bool            has_files() const { return !m_loaded_files.empty(); }

        // Virtual Machine related:

        void            run_program();
        void            debug_program();
        void            step_over_program();
        void            stop_program();
        void            reset_program();
        bool            compile_and_load_program() const;

        NodableView* get_view() const;

    private:
        Config*            m_config { nullptr };
        Nodlang*           m_language { nullptr };
        VirtualMachine*    m_virtual_machine  { nullptr };
        NodeFactory*       m_node_factory { nullptr };
        ComponentFactory*  m_component_factory { nullptr };
        std::vector<File*> m_loaded_files;
        u8_t               m_untitled_file_count = 0;
    };
}
