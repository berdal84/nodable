#pragma once

#include <future>
#include <memory>
#include <string>

#include "fw/core/reflection/reflection"
#include "fw/gui/App.h"

#include "nodable/core/VirtualMachine.h"
#include "nodable/core/NodeFactory.h"
#include "nodable/core/language/Nodlang.h"

#include "NodableView.h"
#include "Config.h"
#include "types.h"

namespace ndbl
{
    // forward declarations
    class NodableView;

    // Nodable application
    // - Only a single instance can exist at the same time
    // - Instantiate it as you want (stack or heap)
    // - The instance will be available statically via: App* App::get_instance()
    // - Is based on fw::App, but extends it using composition instead of inheritance
    class Nodable : public fw::App
    {
	public:
        Nodable();
        ~Nodable();

        Config            config;
        NodeFactory       node_factory;
        HybridFile*       current_file;
        VirtualMachine    virtual_machine;// Virtual Machine to compile/debug/run/pause/... programs

        // File related:

        HybridFile*     open_file(const ghc::filesystem::path&_path);
        HybridFile*     new_file();
        void            save_file(HybridFile *pFile) const;
        void            save_file_as(const ghc::filesystem::path &_path) const;
        HybridFile*     add_file(HybridFile *_file);
        void            close_file(HybridFile*);
        bool            is_current(const HybridFile* _file ) const { return current_file == _file; }
        const std::vector<HybridFile*>&
                        get_files() const { return m_loaded_files; }
        bool            has_files() const { return !m_loaded_files.empty(); }

        // Virtual Machine related:

        void            run_program();
        void            debug_program();
        void            step_over_program();
        void            stop_program();
        void            reset_program();
        bool            compile_and_load_program();

        static Nodable& get_instance(); // singleton
    private:
        bool            on_init() override;
        bool            on_shutdown() override;
        void            on_update() override;

        static Nodable *   s_instance;
        std::vector<HybridFile*> m_loaded_files;
        u8_t               m_untitled_file_count;

    };
}
