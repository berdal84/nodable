require_relative '_utils'

# Provide a base target
def new_target_from_base(name, type)

    target = new_empty_target(name, type)
    target.includes |= FileList[
        "src",
        "src/ndbl",
        "src/tools",
        "libs",
        "libs/whereami/src",
        "libs/imgui",
        "libs/imgui",
        "libs/glm",
        "libs/gl3w/GL",
        "libs/gl3w",
        "libs/SDL/include",
        "libs/IconFontCppHeaders",
        "libs/nativefiledialog-extended/src/include",
        "libs/cpptrace",
        "libs/freetype/include",
        "/usr/include/X11/mesa/GL",
        "#{INSTALL_DIR}/cpptrace/include",
        "#{INSTALL_DIR}/nfd/include",
        "#{INSTALL_DIR}/sdl/include",
    ]
    target.cxx_flags |= [
        "--std=c++20",
        "-fno-char8_t"
    ]
    target.linker_flags |= [
        "-L#{INSTALL_DIR}/sdl/freetype -lfreetype",
        "-L#{INSTALL_DIR}/sdl/lib -lSDL2 -lSDL2main",
        "-L#{INSTALL_DIR}/nfd/lib -lnfd", # Native File Dialog
        "-L#{INSTALL_DIR}/cpptrace/lib -lcpptrace", # https://github.com/jeremy-rifkin/cpptrace?tab=readme-ov-file#use-without-cmake
    ]
    if BUILD_OS_WINDOWS
        target.linker_flags |= [
            "-lole32 -luuid -lshell32", # for nfd
            "-Wl,/SUBSYSTEM:CONSOLE",
            "-lopengl32",
            "-lcpptrace -ldbghelp" # for cpptrace
        ]
    else
        target.linker_flags |= [
            "-lGL",
            "-ldwarf -lz -lzstd -ldl" # for cpptrace
        ]
    end

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

    target.asset_folder_path = "assets" # a single folder

    target.defines |= [
        "IMGUI_USER_CONFIG=\\\"tools/gui/ImGuiExConfig.h\\\"",
        "NDBL_APP_ASSETS_DIR=\\\"#{target.asset_folder_path}\\\"",
        "NDBL_APP_NAME=\\\"#{target.name}\\\"",
        "NDBL_BUILD_REF=\\\"local\\\"",
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