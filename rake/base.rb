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
        "/usr/include/X11/mesa/GL"
    ]
    target.cxx_flags |= [
        # "-stdlib=platform", # ‘libc++’ (with extensions), ‘libstdc++’ (standard), or ‘platform’ (default).
        "--std=c++20",
        "-fno-char8_t"
    ]
        
    target.linker_flags |= [
        "-L#{LIB_DIR}", 
        "`pkg-config --cflags --libs --static freetype2`",
        "`sdl2-config --cflags --static-libs`",
        "-lnfd", # Native File Dialog
        "-lGL",
        "-lcpptrace -ldwarf -lz -lzstd -ldl", # https://github.com/jeremy-rifkin/cpptrace?tab=readme-ov-file#use-without-cmake
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

    target.asset_folder_path = "assets" # a single folder

    target.defines |= [
        "IMGUI_USER_CONFIG=\\\"tools/gui/ImGuiExConfig.h\\\"",
        "NDBL_APP_ASSETS_DIR=\\\"#{target.asset_folder_path}\\\"",
        "NDBL_APP_NAME=\\\"#{target.name}\\\"",
        "NDBL_BUILD_REF=\\\"local\\\"",
    ]
    
    if BUILD_OS_WINDOWS
        target.defines |= [
            "WIN32", # to have an MSVC-like macro 
            "NOMINMAX", # in WIN32, min and max are macros by default, it creates conflicts with std::min/std::max
        ]  
    end

    if BUILD_OS_MACOS
        target.compiler_flags |= [
            "-mmacosx-version-min=#{MACOSX_VERSION_MIN}",
        ]
    end

    if BUILD_TYPE_RELEASE
        target.compiler_flags |= [
            "-O2"
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