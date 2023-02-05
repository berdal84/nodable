#pragma once

// std
#include <future>
#include <memory>
#include <string>

#include <ghc/filesystem.hpp>
#include <fw/gui/AppView.h>
#include <fw/gui/Texture.h>
#include "fw/core/types.h"

namespace fw
{
	class App
	{
	public:
        using fs_path = ghc::filesystem::path;

		App(fs_path _assets_folder_path, AppView* _view);
        virtual ~App() {};

		bool            init();
		void            shutdown();
		void            update();
		void            draw();

        virtual bool    onInit() = 0;
        virtual void    onShutdown() = 0;
        virtual void    onUpdate() = 0;

        bool            should_stop() const { return m_should_stop; }
        void            flag_to_stop();
        u64_t           elapsed_time() const { return m_start_time.time_since_epoch().count(); };
        std::string     compute_asset_path(const char *_relative_path) const;
        TextureManager& texture_manager() { return m_texture_manager; };

    protected:
        const std::chrono::time_point<std::chrono::system_clock>
                        m_start_time = std::chrono::system_clock::now();

        AppView*        m_view;
        TextureManager  m_texture_manager;
        bool            m_should_stop;
        fs_path         m_assets_folder_path;

    };
}
