require_relative '_utils'

# Provide a base target
def new_target_from_base(name, type)

    target = new_empty_target(name, type)
    target.includes |= FileList[
        # internal
        "src",
        "src/ndbl",
        "src/tools",
        # external
        "libs",
        "libs/gl3w",
        "libs/gl3w/GL",
        "libs/glm",
        "libs/IconFontCppHeaders",
        "libs/imgui",
        "libs/imgui",
        "libs/whereami/src",
    ]

    target.defines |= [
        "IMGUI_USER_CONFIG=\\\"tools/gui/ImGuiExConfig.h\\\"",
        "NDBL_APP_NAME=\\\"nodable\\\"",
        "NDBL_BUILD_REF=\\\"local\\\"",
        "CPPTRACE_STATIC_DEFINE", #  error LNK2019: unresolved external symbol "__declspec(dllimport) public: void __cdecl cpptrace::stacktrace::print_with_snippets...
        "PLATFORM_#{PLATFORM.upcase}"
    ]

    target.cxx_flags |= [
        "-std=c++20",

        # related to ImGui
        "-fno-char8_t",
        "-Wno-nontrivial-memcall",
        "-Wno-nontrivial-memaccess", # ImGui has several warnings like this one: "warning: first argument in call to 'memset' is a pointer to non-trivially copyable type 'ImGuiListClipperData' [-Wnontrivial-memcall]"
        "-Wno-unused-function", # imstb_rectpack.h:233:16: error: unused function 'stbrp_setup_heuristic' [-Werror,-Wunused-function]  233 | STBRP_DEF void stbrp_setup_heuristic(stbrp_context *context, int heuristic)
    ]

    # ---- PLATFORM_XXX specific --------
    if PLATFORM_WEB
        target.compiler_flags |= [
            "-v",
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

    elsif PLATFORM_DESKTOP
        
        target.includes |= [
            "libs/nativefiledialog-extended/src/include",
            `pkg-config --cflags-only-I sdl2 freetype2 gl`
        ]

        if BUILD_OS_LINUX
            target.linker_flags |= [
                "-lnfd", `pkg-config --libs gtk+-3.0`,
                `pkg-config --libs --static sdl2 freetype2 gl`,
            ] # NativeFileDialog

        elsif BUILD_OS_WINDOWS      
            
            target.includes |= []   

            target.defines |= [
                "SDL_MAIN_HANDLED",
                "NOMINMAX", # avoids windows min/max to collide (see https://stackoverflow.com/questions/11544073/how-do-i-deal-with-the-max-macro-in-windows-h-colliding-with-max-in-std)
                "__PRFCHWINTRIN_H", # issues with clang (SDL_endian.h:41:1: error: definition of builtin function '_m_prefetch')
            ]

            target.linker_flags |= [
                "-Xlinker /NODEFAULTLIB:libcmt",
                "-Xlinker /SUBSYSTEM:CONSOLE", # We compile a console app, windows needs to know that main() is the entry point instead of WinMain
                "-Xlinker /ENTRY:mainCRTStartup", # make sure entry point is main() (not wmain)
                "-L#{BUILD_DIR}/libs/nativefiledialog-extended/lib", "-lnfd",

            ] # NativeFileDialog

            target.vcpkg = [
                'sdl2',
                'freetype2',
                'opengl'
            ]

            if BUILD_TYPE_RELEASE
                
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
    if BUILD_TYPE_RELEASE
        target.compiler_flags |= [
            "-O2"
        ]
    elsif BUILD_TYPE_DEBUG
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