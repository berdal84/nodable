require_relative '_utils'

# Provide a base target
def new_target_from_base(name, type)

    target = new_empty_target(name, type)
    
    if OPTIONS.verbose
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
        "NDBL_BUILD_REF=\\\"#{`git describe --tags HEAD`.chomp}-#{OPTIONS.build_config}\\\"",
        "NDBL_#{OPTIONS.target.upcase}",
        "NDBL_#{OS.upcase}",
        "CPPTRACE_STATIC_DEFINE" #  error LNK2019: unresolved external symbol "__declspec(dllimport) public: void __cdecl cpptrace::stacktrace::print_with_snippets...
    ]

    target.cxx_flags |= [
        "-x c++",       # we use clang, not clang++ (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-x)
        "-std=c++20",   # see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-std       
        "-fno-char8_t", # related to ImGui and font awesome (const char* and concatenation would fail without it)
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
            "-lstdc++", # note: -llibstdc++ was not working, it requires to be installed.
        ]

        if LINUX

            target.vcpkg = [
                "sdl2",
                "freetype2",
                "gl", # equivqlent of opengl for linux
                "nfd", "dbus-1", # required by nfd (that has no .pc file)
                "gl3w",
                "lodepng",
            ]

        elsif WINDOWS      

            target.vcpkg = [
                "sdl2",
                "freetype2",
                "opengl",
                "nfd",
                "gl3w",
                "lodepng",
            ]

            target.defines |= [
                "SDL_MAIN_HANDLED",
                "NOMINMAX", # avoids windows min/max to collide (see https://stackoverflow.com/questions/11544073/how-do-i-deal-with-the-max-macro-in-windows-h-colliding-with-max-in-std)
                "__PRFCHWINTRIN_H", # issues with clang (SDL_endian.h:41:1: error: definition of builtin function '_m_prefetch')
            ]

            target.linker_flags |= [                
                "-Xlinker /SUBSYSTEM:CONSOLE", # We compile a console app, windows needs to know that main() is the entry point instead of WinMain
                "-Xlinker /ENTRY:mainCRTStartup", # make sure entry point is main() (not wmain)
            ]

            target.compiler_flags |= [
                "-D_MT",
                # "-D_DLL"
            ]
        end
    end

    # ---- BUILD_CONFIG_XXX specific --------

    if RELEASE

        target.compiler_flags |= [
            "-Oz", # O2 + extra reduced size (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption)
            # "-pedantic", # https://clang.llvm.org/docs/UsersManual.html#cmdoption-pedantic
            # "-Werror", # It's too much!
        ]

    elsif OPTIMIZED

        target.compiler_flags |= [
            "-g",  # Generate debug information (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-g)
            "-O2", # Moderate level of optimization which enables most optimizations. (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-O2)
        ]

    elsif DEBUG

        target.compiler_flags |= [
            "-g",  # Generate debug information (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-g)
            "-O0", # No optimizations (see https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-O0)
        ]

        target.defines |= [
            "TOOLS_DEBUG",
            "NDBL_DEBUG"
        ]

    end

    target
end
