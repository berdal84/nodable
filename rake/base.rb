require_relative '_utils'

# Provide a base target
def new_target_from_base(name, type)

    target = new_empty_target(name, type)

    target.includes |= FileList[
        "src",
        "src/ndbl",
        "src/tools"
    ]

    target.includes |= FileList[
        "libs",
        "libs/whereami/src",
        "libs/imgui",
        "libs/imgui",
        "libs/glm",
        "libs/gl3w/GL",
        "libs/gl3w",
        "libs/IconFontCppHeaders",
        "/usr/include/X11/mesa/GL",
        "libs/cpptrace/include",
        "libs/freetype/include",
        "libs/googletest/googletest/include",
        "libs/nativefiledialog-extended/src/include",
        "libs/SDL/include",
    ]

    target.linker_flags |= [
        "-L#{BUILD_DIR}/cpptrace",
        "-L#{BUILD_DIR}/cpptrace/_deps/libdwarf-build/src/lib/libdwarf",
        "-L#{BUILD_DIR}/cpptrace/_deps/zstd-build/lib",
        "-L#{BUILD_DIR}/freetype",
        "-L#{BUILD_DIR}/googletest/lib",
        "-L#{BUILD_DIR}/nfd/src",
        "-L#{BUILD_DIR}/sdl",
    ]

    target.cxx_flags |= [
        "--std=c++20",
        "-fno-char8_t"
    ]

    target.linker_flags |= [
        "-lfreetype -lbz2 -lpng16 -lz -lharfbuzz -lbrotlidec",
        "-lSDL2 -lSDL2main",
    ]

    if BUILD_OS_WINDOWS
        target.linker_flags |= [
            "-Wl,/SUBSYSTEM:CONSOLE", # WinMain vs main, here we want to use main
            "-lopengl32",
            "-lnfd -lole32 -luuid -lshell32",
            "-lcpptrace -ldbghelp" # see https://github.com/jeremy-rifkin/cpptrace?tab=readme-ov-file#use-without-cmake
        ]
    else
        target.linker_flags |= [
            "-lGL", # OpenGL
            "-lcpptrace -ldwarf -lz -lzstd -ldl", # see https://github.com/jeremy-rifkin/cpptrace?tab=readme-ov-file#use-without-cmake
            "-lnfd",
        ]

        if BUILD_OS_LINUX
            target.linker_flags |= [
                "`pkg-config --cflags --libs gtk+-3.0`",
            ]
        elsif BUILD_OS_MACOS
            target.linker_flags |= [
                "-framework CoreFoundation",
                "-framework Cocoa"
            ]
        end
    end

    target.asset_folder_path = "assets" # a single folder

    target.defines |= [
        "IMGUI_USER_CONFIG=\\\"tools/gui/ImGuiExConfig.h\\\"",
        "NDBL_APP_ASSETS_DIR=\\\"#{target.asset_folder_path}\\\"",
        "NDBL_APP_NAME=\\\"#{target.name}\\\"",
        "NDBL_BUILD_REF=\\\"local\\\""
    ]
    
    if BUILD_OS_WINDOWS
        target.defines |= [
            "_DLL",
            "WIN32", # to have an MSVC-like macro 
            "NOMINMAX", # in WIN32, min and max are macros by default, it creates conflicts with std::min/std::max
        ]  
    end

    if BUILD_TYPE_RELEASE
        target.compiler_flags |= [
            "-O3"
        ] 
    elsif BUILD_TYPE_DEBUG
        target.compiler_flags |= [
            "-g", # generates symbols
            "-O0", # no optim
            "-Wfatal-errors",
            "-pedantic"
        ]
    end
    target
end


def new_target_from_test(name, type)

    if type != TargetType::EXECUTABLE
        raise "Please provide TargetType::EXECUTABLE as type" # we want the same interface, that's why type is an arg
    end

    target = new_target_from_base( name, type)
    target.includes |= [
        "#{BUILD_DIR}/googletest/include",
    ]
    target.linker_flags |= [
        "-L#{BUILD_DIR}/googletest/lib -lgtest -lgtest_main",
    ]
    target
end