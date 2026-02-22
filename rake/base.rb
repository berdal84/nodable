require_relative '_utils'

# Provide a base target
def new_target_from_base(name, type)

    target = new_empty_target(name, type)
    
    if VERBOSE
        target.compiler_flags.append("-v")
        target.linker_flags.append("-v")
    end

    target.includes = FileList[
        # internal
        "src",
        "src/ndbl",
        "src/tools",
        # external
        "extern",
        "extern/imgui",
        "extern/IconFontCppHeaders",
        "extern/whereami/src",
    ]

    target.defines |= [
        "IMGUI_USER_CONFIG=\\\"tools/gui/ImGuiExConfig.h\\\"",
        "NDBL_APP_NAME=\\\"nodable\\\"",
        "NDBL_BUILD_REF=\\\"local\\\"",
        "NDBL_#{TARGET.upcase}",
        "NDBL_#{OS.upcase}",
        "CPPTRACE_STATIC_DEFINE" #  error LNK2019: unresolved external symbol "__declspec(dllimport) public: void __cdecl cpptrace::stacktrace::print_with_snippets...
    ]

    target.cxx_flags |= [
        "-x c++", # we use clang, not clang++, since behavior differs in windows and linux, we do NOT use clang++
        "-std=c++20",        
        "-fno-char8_t", # related to ImGui
    ]

    if WINDOWS
        
        target.compiler_flags |=[
            "-fms-extensions" # turn ON MSVC compatibility
        ]

        target.cxx_flags |= [
            "-Wno-nontrivial-memcall",
            "-Wno-nontrivial-memaccess", # ImGui has several warnings like this one: "warning: first argument in call to 'memset' is a pointer to non-trivially copyable type 'ImGuiListClipperData' [-Wnontrivial-memcall]"
            "-Wno-unused-function", # imstb_rectpack.h:233:16: error: unused function 'stbrp_setup_heuristic' [-Werror,-Wunused-function]  233 | STBRP_DEF void stbrp_setup_heuristic(stbrp_context *context, int heuristic)
        ]
    end

    if WEB
        target.compiler_flags |= [
            "-s USE_PTHREADS=1",
            "-s USE_FREETYPE=1",
            "-s USE_SDL=2",
            "-sNO_DISABLE_EXCEPTION_CATCHING",
			"-gsource-map",
        ]

        target.linker_flags |= [
            "-s PTHREAD_POOL_SIZE='navigator.hardwareConcurrency'",
            "-s EMBIND_STD_STRING_IS_UTF8=0",
            "-s ALLOW_MEMORY_GROWTH",
            "-Wno-pthreads-mem-growth", # emcc: warning: -pthread + ALLOW_MEMORY_GROWTH may run non-wasm code slowly, see https://github.com/WebAssembly/design/issues/1271 [-Wpthreads-mem-growth]
            "-s MIN_WEBGL_VERSION=2",
            "-s MAX_WEBGL_VERSION=2",
            "--emrun"
        ]

    elsif DESKTOP

        target.linker_flags |= [
            # Add libraries manually when there is no no *.pc file to work with pkg_config
            "-lnfd",
            "-lgl3w",
            "-llodepng",
        ]  

        if LINUX
            target.compiler_flags |= [
                pkg_config("--cflags sdl2 freetype2")
            ]

            target.linker_flags |= [
                `pkg-config --libs gtk+-3.0`, # required by -lnfd
                pkg_config("--libs --static sdl2 freetype2 gl dbus-1"),
            ]

        elsif WINDOWS      

            target.compiler_flags |= [
               pkg_config("--cflags sdl2 freetype2 opengl")
            ]

            target.defines |= [
                "SDL_MAIN_HANDLED",
                "NOMINMAX", # avoids windows min/max to collide (see https://stackoverflow.com/questions/11544073/how-do-i-deal-with-the-max-macro-in-windows-h-colliding-with-max-in-std)
                "__PRFCHWINTRIN_H", # issues with clang (SDL_endian.h:41:1: error: definition of builtin function '_m_prefetch')
            ]

            target.linker_flags |= [                
                "-Xlinker /SUBSYSTEM:CONSOLE", # We compile a console app, windows needs to know that main() is the entry point instead of WinMain
                "-Xlinker /ENTRY:mainCRTStartup", # make sure entry point is main() (not wmain)
                # Add libraries and deps using pkg-config / pkgconf
                pkg_config("--libs --static sdl2 freetype2 opengl"),
            ] # NativeFileDialog

            if RELEASE
                
                target.compiler_flags |= [
                    "-D_MT",
                    # "-D_DLL"
                ]

                target.linker_flags |= [
                    # We don't need to link those since "-D_DLL" is commented out
                    # "-lmsvcrt",
                    # "-lucrt",
                    # "-lvcruntime",
                    # "-lmsvcprt",
                ]
            else

                target.compiler_flags |= [
                    "-D_MT",
                    # "-D_DLL"
                ]

                target.linker_flags |= [
                    # "-lmsvcrtd",
                    # "-lucrtd",
                    # "-lvcruntimed",
                    # "-lmsvcprtd",
                ]
            end
        end
    end

    # ---- BUILD_TYPE_XXX specific --------
    if RELEASE
        target.compiler_flags |= [
            "-O2"
        ]
    elsif DEBUG
        target.compiler_flags |= [
            "-g", # generates symbols
            "-O0", # no optim
            #"-pedantic"
        ]
        target.defines |= [
            "TOOLS_DEBUG",
            "NDBL_DEBUG"
        ]
    end

    target
end
