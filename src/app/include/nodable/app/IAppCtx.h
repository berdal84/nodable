#pragma once
#include <string>
#include <vector>
#include <ghc/filesystem.hpp>

namespace Nodable
{
    // forward decl
    namespace vm
    {
        class VM;
    }
    class Settings;
    class Language;
    class TextureManager;
    class File;

    class IAppCtx
    {
    public:
        using fs_path = ghc::filesystem::path;

        virtual ~IAppCtx() {};

        virtual bool            is_debugging_program() const = 0;
        virtual bool            is_next_node_in_program(Node*) const = 0;
        virtual bool            is_running_program() const = 0;
        virtual bool            open_file(const fs_path& _filePath)= 0;
        virtual File*           new_file()= 0;
        virtual void            save_file()const= 0;
        virtual void            save_file_as(const fs_path &_path)= 0;
        virtual void            close_file(File*)= 0;
        virtual bool            is_current(const File* _file ) const = 0;
        virtual void            set_curr_file(size_t _index)= 0;
        virtual void            set_curr_file(File *_file)= 0;
        virtual const std::vector<File*>& get_files() const = 0;
        virtual bool                has_files() const = 0;
        virtual File*               get_curr_file()const = 0;
        virtual void                run_program() = 0;
        virtual void                debug_program() = 0;
        virtual void                step_over_program() = 0;
        virtual void                stop_program() = 0;
        virtual void                reset_program() = 0;
        virtual void                flag_to_stop() = 0;
        virtual bool                compile_and_load_program() = 0;
        virtual std::string         get_absolute_asset_path(const char*)const = 0;
        virtual vm::VM&             get_vm() = 0;
        virtual Settings&           get_settings() = 0;
        virtual Language&           get_language() = 0;
        virtual const Language&     get_language() const = 0;
        virtual TextureManager&     get_texture_manager() = 0;
        virtual u64_t               get_elapsed_time() const = 0;
    };
}