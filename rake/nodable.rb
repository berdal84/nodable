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
        "-Wno-nontrivial-memcall",   # imgui/imgui_internal.h:1933:38: warning: first argument in call to 'memset' is a pointer to non-trivially copyable type 'ImGuiStackTool' [-Wnontrivial-memcall]
        "-Wno-nontrivial-memaccess", # ImGui has several warnings like this one: "warning: first argument in call to 'memset' is a pointer to non-trivially copyable type 'ImGuiListClipperData' [-Wnontrivial-memcall]"
        "-Wno-unused-function",      # imstb_rectpack.h:233:16: error: unused function 'stbrp_setup_heuristic' [-Werror,-Wunused-function]  233 | STBRP_DEF void stbrp_setup_heuristic(stbrp_context *context, int heuristic)
    ]

    if EMSCRIPTEN

        target.defines = [
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

        target.defines = [
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

#---------------------------------------------------------------------------
# TARGETS
#---------------------------------------------------------------------------

$tools_core = target("tools_core", TARGET_TYPE_OBJECTS)

if !EMSCRIPTEN
    $tools_core.sources |= [
        # whereami - to be aware of the binary's path at runtime
        "extern/whereami/src/whereami.c"
    ]
end

$tools_core.sources |= FileList[
    "src/tools/core/reflection/qword.cpp",
    "src/tools/core/reflection/Type.cpp",
    "src/tools/core/reflection/TypeRegister.cpp",
    "src/tools/core/reflection/variant.cpp",
    #"src/tools/core/memory/pointers.cpp",
    "src/tools/core/EventManager.cpp",
    "src/tools/core/FileSystem.cpp",
    "src/tools/core/format.cpp",
    "src/tools/core/log.cpp",
    "src/tools/core/StateMachine.cpp",
    "src/tools/core/System.cpp",
    "src/tools/core/TaskManager.cpp"
]

#---------------------------------------------------------------------------
$tools_gui = target("tools_gui", TARGET_TYPE_OBJECTS)


$tools_gui.sources |= FileList[

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

    # tools
    "src/tools/gui/geometry/BezierCurveSegment2D.cpp",
    "src/tools/gui/geometry/BoxShape2D.cpp",
    "src/tools/gui/geometry/Rect.cpp",
    "src/tools/gui/geometry/SpatialNode.cpp",
    "src/tools/gui/geometry/TRSTransform2D.cpp",
    "src/tools/gui/Action.cpp",
    "src/tools/gui/ActionManager.cpp",
    "src/tools/gui/ActionManagerView.cpp",
    "src/tools/gui/App.cpp",
    "src/tools/gui/AppView.cpp",
    "src/tools/gui/Config.cpp",
    "src/tools/gui/FontManager.cpp",
    "src/tools/gui/ImGuiEx.cpp",
    "src/tools/gui/ViewState.cpp",
    "src/tools/gui/TextureManager.cpp",
]

#---------------------------------------------------------------------------

app = target("tools-gui-example", TARGET_TYPE_EXECUTABLE)

app.sources |= FileList[
    "src/tools/gui-example/AppExample.cpp",
    "src/tools/gui-example/AppExampleView.cpp",
    "src/tools/gui-example/main.cpp"
]
app.assets = FileList[
    # Fonts
    "fonts/CenturyGothic.ttf",
    "fonts/fa-solid-900.ttf",
    "fonts/JetBrainsMono-*.ttf", # 4 variants
]
app.depends_on_target |= [
    $tools_core,
    $tools_gui
]

#---------------------------------------------------------------------------

tools_test = target("tools-tests", TARGET_TYPE_EXECUTABLE)
tools_test.sources |= FileList[
    "src/tools/core/Delegate.specs.cpp",
    "src/tools/core/string.specs.cpp",
    "src/tools/core/reflection/reflection.specs.cpp",
    "src/tools/gui/geometry/SpatialNode.specs.cpp",
    "src/tools/gui/geometry/Rect.specs.cpp",
    "src/tools/test/main.cpp",
]

tools_test.vcpkg |= [
    "gtest"
]

tools_test.depends_on_target |= [
    $tools_core,
    $tools_gui
]

#---------------------------------------------------------------------------
ndbl_core = target("ndbl_core", TARGET_TYPE_OBJECTS)
ndbl_core.sources |= FileList[
    "src/ndbl/core/language/Nodlang.cpp",
    "src/ndbl/core/ASTForLoop.cpp",
    "src/ndbl/core/ASTFunctionCall.cpp",
    "src/ndbl/core/ASTIf.cpp",
    "src/ndbl/core/ASTLiteral.cpp",
    "src/ndbl/core/ASTNode.cpp",
    "src/ndbl/core/ASTNodeProperty.cpp",
    "src/ndbl/core/ASTNodeSlot.cpp",
    "src/ndbl/core/ASTScope.cpp",
    "src/ndbl/core/ASTSlotLink.cpp",
    "src/ndbl/core/ASTSwitchBehavior.cpp",
    "src/ndbl/core/ASTToken.cpp",
    "src/ndbl/core/ASTTokenRibbon.cpp",
    "src/ndbl/core/ASTUtils.cpp",
    "src/ndbl/core/ASTVariable.cpp",
    "src/ndbl/core/ASTWhileLoop.cpp",
    "src/ndbl/core/Graph.cpp",    
    "src/ndbl/core/NodableHeadless.cpp",
]

#---------------------------------------------------------------------------
ndbl_gui = target("ndbl_gui", TARGET_TYPE_OBJECTS)
ndbl_gui.sources |= FileList[
    "src/ndbl/gui/ASTNodeSlotView.cpp", 
    "src/ndbl/gui/ASTNodeView.cpp", 
    "src/ndbl/gui/ASTNodeViewContextualMenu.cpp",
    "src/ndbl/gui/ASTNodePropertyView.cpp",  
    "src/ndbl/gui/ASTScopeView.cpp", 
    "src/ndbl/gui/Config.cpp",
    "src/ndbl/gui/File.cpp",
    "src/ndbl/gui/FileView.cpp",
    "src/ndbl/gui/GraphView.cpp",
    "src/ndbl/gui/History.cpp",
    "src/ndbl/gui/Nodable.cpp",
    "src/ndbl/gui/NodableView.cpp",
    "src/ndbl/gui/PhysicsComponent.cpp",       
]
#---------------------------------------------------------------------------
ndbl_app = target("nodable", TARGET_TYPE_EXECUTABLE)

ndbl_app.distribute = true # will copy binary and assets into DIST_DIR

ndbl_app.sources |= FileList[
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
        "http/.htaccess:.htaccess"
    ]

else
    ndbl_app.assets = ndbl_assets
end

ndbl_app.depends_on_target |= [
    $tools_gui,
    $tools_core,
    ndbl_core,
    ndbl_gui
]
#---------------------------------------------------------------------------
ndbl_test = target("nodable-tests", TARGET_TYPE_EXECUTABLE)
ndbl_test.sources |= FileList[
    "src/ndbl/core/**/*.specs.cpp",
    "src/ndbl/test/main.cpp",
]

ndbl_test.vcpkg |= [
    "gtest"
]

ndbl_test.depends_on_target |= [
    $tools_core,
    $tools_gui,
    ndbl_core
]

if OPTIONS.ignore_gui_tests
    puts "ignore_gui_tests is TRUE, skip GUI specs."
else
    ndbl_test.assets |= ndbl_app.assets;

    ndbl_test.sources |= [
        "src/ndbl/gui/Nodable.specs.cpp"
    ]
    ndbl_test.depends_on_target |= [
        ndbl_gui
    ]
end

#---------------------------------------------------------------------------
# TASKS
#---------------------------------------------------------------------------

namespace :tools do

    task :clean => [
        'core:clean',
        'gui:clean',
        'app:clean'
    ]

    task :rebuild => [
        'clean',
        'build'
    ]

    task :build => [
        'core:build',
        'gui:build',
        'app:build',
    ]

    if DESKTOP
        task :clean => 'test:clean'
        task :build => 'test:build'
        task :test  => 'test:run'
    end

    namespace :core do
        bt_tasks_for_target( $tools_core )
    end

    task :gui
    namespace :gui do
        bt_tasks_for_target( $tools_gui )
    end

    task :app
    namespace :app do
        bt_tasks_for_target( app )
    end

    namespace :test do
        bt_tasks_for_target( tools_test )
    end

end

#---------------------------------------------------------------------------

namespace :ndbl do

    task :clean => [
        'core:clean',
        'gui:clean',
        'app:clean'
    ]

    task :rebuild => [
        'clean',
        'build'
    ]

    task :build => [
        'core:build',
        'gui:build',
        'app:build'
    ]

    if DESKTOP
        task :clean => 'test:clean'
        task :build => 'test:build'
        task :test  => 'test:run'
    end

    namespace :core do
        bt_tasks_for_target( ndbl_core )
    end
    
    namespace :gui do        
        bt_tasks_for_target( ndbl_gui )
    end

    namespace :app do
        bt_tasks_for_target( ndbl_app )
    end

    namespace :test do
        bt_tasks_for_target( ndbl_test )
    end
end
#---------------------------------------------------------------------------