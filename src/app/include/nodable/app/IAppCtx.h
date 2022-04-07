#pragma once
#include <string>
#include <vector>
#include <ghc/filesystem.hpp>

namespace Nodable
{
    // forward decl
    class VirtualMachine;
    class Settings;
    class Language;
    class TextureManager;
    class File;

    class IAppCtx
    {
    public:
        using fs_path = ghc::filesystem::path;
        using Files   = std::vector<File*>;

        virtual ~IAppCtx() = default;

        virtual bool            open_file(const fs_path& _filePath)= 0;
        virtual File*           new_file()= 0;
        virtual void            save_file()const= 0;
        virtual void            save_file_as(const fs_path &_path)= 0;
        virtual void            close_file(File*)= 0;
        virtual bool            is_current(const File* _file ) const = 0;
        virtual void            current_file(File *_file)= 0;
        virtual File*           current_file()const = 0;
        virtual const Files&    get_files() const = 0;
        virtual bool            has_files() const = 0;
        virtual void            run_program() = 0;
        virtual void            debug_program() = 0;
        virtual void            step_over_program() = 0;
        virtual void            stop_program() = 0;
        virtual void            reset_program() = 0;
        virtual void            flag_to_stop() = 0;
        virtual bool            compile_and_load_program() = 0;
        virtual std::string     compute_asset_path(const char* _relative_path) const = 0;
        virtual VirtualMachine&             virtual_machine() = 0;
        virtual Settings&       settings() = 0;
        virtual Language&       language() = 0;
        virtual const Language& language() const = 0;
        virtual TextureManager& texture_manager() = 0;
        virtual u64_t           elapsed_time() const = 0;
    };
}