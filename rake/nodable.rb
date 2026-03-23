#---------------------------------------------------------------------------
# NODABLE's BUILD TARGETS & TASKS
#---------------------------------------------------------------------------
#
# This file contains the definition of several build targets:
# tools, tools-tests, nodable, and nodable-tests
# Each target inherits* from a base target defined in ndbl_target_new()
# (*this was a joke to trigger potential your OOP-phobia ;) )
#
#                                                            Bérenger, 2026.
#---------------------------------------------------------------------------

# Import my custom tools to build C++ desktop/web clang-based projects.
require_relative 'build-tools'

#---------------------------------------------------------------------------
# BASE TARGET - 
#---------------------------------------------------------------------------

def target(name, type)

    target = bt_target(name, type)
    
    if OPTIONS.verbose
        target.compiler_flags.tools_append("-v")
        target.linker_flags.tools_append("-v")
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
    end

    # disable some warnings
    target.cxx_flags |= [
        LINUX ?  "-Wno-nontrivial-memaccess" : "-Wno-nontrivial-memcall", # imgui/imgui_internal.h:1933:38: warning: first argument in call to 'memset' is a pointer to non-trivially copyable type 'ImGuiStackTool' [-Wnontrivial-memcall]
        "-Wno-unused-function",    # imstb_rectpack.h:233:16: error: unused function 'stbrp_setup_heuristic' [-Werror,-Wunused-function]  233 | STBRP_DEF void stbrp_setup_heuristic(stbrp_context *context, int heuristic)
    ]

    if EMSCRIPTEN

        target.defines |= [
            "NDBL_WEB"
        ]

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

        target.vcpkg = [
            # "sdl2",
            # "freetype2",
            # "opengl",
            # "nfd",
            # "gl3w",
            "lodepng",
        ]

    else

        target.defines |= [
            "NDBL_DESKTOP"
        ]

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
                "-Xlinker /SUBSYSTEM:CONSOLE", # We compile a console tools_app, windows needs to know that main() is the entry point instead of WinMain
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

#---------------------------------------------------------------------------
# TARGETS
#---------------------------------------------------------------------------

common = target("common", TARGET_TYPE_OBJECTS)

common.sources = FileList[

    # unity builds
    # TODO: automatize the generation of each **/unity_build.cpp
    "src/tools/core/unity_build.cpp",
    "src/ndbl/core/unity_build.cpp",
    "src/tools/gui/unity_build.cpp",
    "src/ndbl/gui/unity_build.cpp",
    # unity builds (end)

    # Imgui and related sources
    "extern/imgui/imgui.cpp",
    "extern/imgui/imgui_demo.cpp",
    "extern/imgui/imgui_draw.cpp",
    "extern/imgui/imgui_tables.cpp",
    "extern/imgui/imgui_widgets.cpp",
    "extern/imgui/misc/freetype/imgui_freetype.cpp",
    "extern/imgui/backends/imgui_impl_sdl2.cpp",
    "extern/imgui/backends/imgui_impl_opengl3.cpp",
    "extern/ImGuiColorTextEdit/TextEditor.cpp", # not from imgui, but related to
]

if DESKTOP
    common.sources |= [
        # whereami - to be aware of the binary's path at runtime
        "extern/whereami/src/whereami.c"
    ]
end

#---------------------------------------------------------------------------

tools_app = target("tools-gui-example", TARGET_TYPE_EXECUTABLE)

tools_app.sources |= FileList[
    "src/tools/gui-example/main.cpp"
]
tools_app.assets = FileList[
    # Fonts
    "fonts/CenturyGothic.ttf",
    "fonts/fa-solid-900.ttf",
    "fonts/JetBrainsMono-*.ttf", # 4 variants
]

tools_app.depends_on_target = [
    common
]

#---------------------------------------------------------------------------

tools_test = target("tools-tests", TARGET_TYPE_EXECUTABLE)

tools_test.sources |= FileList[
    "src/tools/test/main.cpp",
]

tools_test.vcpkg |= [
    "gtest"
]

tools_test.depends_on_target = [
    common
]

#---------------------------------------------------------------------------
ndbl_app = target("nodable", TARGET_TYPE_EXECUTABLE)

ndbl_app.distribute       = true  # will copy binary and assets into DIST_DIR

ndbl_app.sources = FileList[
    "src/ndbl/app/main.cpp",
]

ndbl_assets = FileList[
    # Examples
    "./examples/arithmetic.cpp",
    "./examples/for-loop.cpp",
    "./examples/if-else.cpp",
    "./examples/multi-instructions.cpp",
    # Fonts
    "./fonts/CenturyGothic.ttf",
    "./fonts/fa-solid-900.ttf",
    "./fonts/JetBrainsMono-*.ttf", # 4 variants
    # Images
    "./images/nodable-logo-xs.png",
]

if EMSCRIPTEN

    # Preload assets (they will be compiled in a binary .data)
    ndbl_app.linker_flags |= ndbl_assets.map{|path| "--preload-file #{path}" }

    # Provide headers for deployment (note: requires https when deployed).
    ndbl_app.assets = [
        "http/.htaccess:.htaccess",
        "http/nodable.html:nodable.html",
    ]

else
    ndbl_app.assets = ndbl_assets
end

ndbl_app.depends_on_target = [
    common
]

#---------------------------------------------------------------------------
ndbl_test = target("nodable-tests", TARGET_TYPE_EXECUTABLE)
ndbl_test.sources |= FileList[
    "src/ndbl/test/main.cpp",
]

ndbl_test.vcpkg |= [
    "gtest"
]

ndbl_test.depends_on_target = [
    common
]

if OPTIONS.ignore_gui_tests
    puts "ignore_gui_tests is TRUE, skip GUI specs."
else
    ndbl_test.assets |= ndbl_app.assets;

    ndbl_test.sources |= [
        "src/ndbl/gui/Nodable.specs.cpp"
    ]
end

#---------------------------------------------------------------------------
# TASKS
#---------------------------------------------------------------------------

namespace :common do
    bt_tasks_for_target( common )
end

#---------------------------------------------------------------------------

namespace :tools do

    task :clean => [
        'app:clean'
    ]

    task :rebuild => [
        'clean',
        'build'
    ]

    task :build => [
        'app:build',
    ]

    if DESKTOP
        task :clean => 'test:clean'
        task :build => 'test:build'
        task :test  => 'test:run'
    end

    namespace :app do
        bt_tasks_for_target( tools_app )
    end

    namespace :test do
        bt_tasks_for_target( tools_test )
    end

end

#---------------------------------------------------------------------------

namespace :ndbl do

    task :clean => [
        'app:clean'
    ]

    task :rebuild => [
        'clean',
        'build'
    ]

    task :build => [
        'app:build'
    ]

    if DESKTOP
        task :clean => 'test:clean'
        task :build => 'test:build'
        task :test  => 'test:run'
    end

    namespace :app do
        bt_tasks_for_target( ndbl_app )
    end

    namespace :test do
        bt_tasks_for_target( ndbl_test )
    end
end
#---------------------------------------------------------------------------