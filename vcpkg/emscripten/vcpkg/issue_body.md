Package: dbus:wasm32-emscripten@1.16.2#2

**Host Environment**

- Host: x64-windows
- Compiler: Clang 23.0.0
- CMake Version: 4.2.3
-    vcpkg-tool version: 2025-12-16-44bb3ce006467fc13ba37ca099f64077b8bbf84d
    vcpkg-scripts version: 05442024c3 2026-02-20 (4 weeks ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Note: dbus only supports dynamic library linkage. Building dynamic library.
-- Using cached dbus-dbus-dbus-1.16.2.tar.gz
-- Cleaning sources at C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean. Use --editable to skip cleaning for the packages you specify.
-- Extracting source C:/vcpkg/downloads/dbus-dbus-dbus-1.16.2.tar.gz
-- Applying patch cmake.dep.patch
-- Applying patch pkgconfig.patch
-- Applying patch getpeereid.patch
-- Applying patch libsystemd.patch
-- Applying patch remove-path.patch
-- Applying patch remove-var-lib-dbus-creation.patch
-- Using source at C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean
-- Configuring wasm32-emscripten
CMake Error at scripts/cmake/vcpkg_execute_required_process.cmake:127 (message):
    Command failed: C:\\vcpkg\\downloads\\tools\\ninja-1.13.2-windows\\ninja.exe -v
    Working Directory: C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/vcpkg-parallel-configure
    Error code: 1
    See logs for more information:
      C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-dbg-CMakeCache.txt.log
      C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-rel-CMakeCache.txt.log
      C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-dbg-CMakeConfigureLog.yaml.log
      C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-rel-CMakeConfigureLog.yaml.log
      C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-out.log

Call Stack (most recent call first):
  C:/Users/beren/nodable/vcpkg/emscripten/x64-windows/share/vcpkg-cmake/vcpkg_cmake_configure.cmake:269 (vcpkg_execute_required_process)
  C:/Users/beren/AppData/Local/vcpkg/registries/git-trees/a29fd7f168a1d2a3de4267941c2e28965ea1125e/portfile.cmake:28 (vcpkg_cmake_configure)
  scripts/ports.cmake:206 (include)



```

<details><summary>C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-out.log</summary>

```
[1/2] "C:/Program Files/CMake/bin/cmake.exe" -E chdir "../../wasm32-emscripten-dbg" "C:/Program Files/CMake/bin/cmake.exe" "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean" "-G" "Ninja" "-DCMAKE_BUILD_TYPE=Debug" "-DCMAKE_INSTALL_PREFIX=C:/vcpkg/packages/dbus_wasm32-emscripten/debug" "-DFETCHCONTENT_FULLY_DISCONNECTED=ON" "-DDBUS_BUILD_TESTS=OFF" "-DDBUS_ENABLE_DOXYGEN_DOCS=OFF" "-DDBUS_ENABLE_XML_DOCS=OFF" "-DDBUS_INSTALL_SYSTEM_LIBS=OFF" "-DDBUS_WITH_GLIB=OFF" "-DTHREADS_PREFER_PTHREAD_FLAG=ON" "-DXSLTPROC_EXECUTABLE=FALSE" "-DCMAKE_INSTALL_SYSCONFDIR=C:/vcpkg/packages/dbus_wasm32-emscripten/etc/dbus" "-DWITH_SYSTEMD_SYSTEMUNITDIR=lib/systemd/system" "-DWITH_SYSTEMD_USERUNITDIR=lib/systemd/user" "-DENABLE_SYSTEMD=OFF" "-DDBUS_BUILD_X11=OFF" "-DCMAKE_REQUIRE_FIND_PACKAGE_X11=OFF" "-DCMAKE_MAKE_PROGRAM=C:\vcpkg\downloads\tools\ninja-1.13.2-windows\ninja.exe" "-DCMAKE_SYSTEM_NAME=Emscripten" "-DBUILD_SHARED_LIBS=ON" "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=C:\Users\beren\nodable\extern\emsdk\upstream\emscripten/cmake/Modules/Platform/Emscripten.cmake" "-DVCPKG_TARGET_TRIPLET=wasm32-emscripten" "-DVCPKG_SET_CHARSET_FLAG=ON" "-DVCPKG_PLATFORM_TOOLSET=external" "-DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON" "-DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON" "-DCMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY=ON" "-DCMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP=TRUE" "-DCMAKE_VERBOSE_MAKEFILE=ON" "-DVCPKG_APPLOCAL_DEPS=OFF" "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DCMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION=ON" "-DVCPKG_CXX_FLAGS=" "-DVCPKG_CXX_FLAGS_RELEASE=" "-DVCPKG_CXX_FLAGS_DEBUG=" "-DVCPKG_C_FLAGS=" "-DVCPKG_C_FLAGS_RELEASE=" "-DVCPKG_C_FLAGS_DEBUG=" "-DVCPKG_CRT_LINKAGE=dynamic" "-DVCPKG_LINKER_FLAGS=" "-DVCPKG_LINKER_FLAGS_RELEASE=" "-DVCPKG_LINKER_FLAGS_DEBUG=" "-DVCPKG_TARGET_ARCHITECTURE=wasm32" "-DCMAKE_INSTALL_LIBDIR:STRING=lib" "-DCMAKE_INSTALL_BINDIR:STRING=bin" "-D_VCPKG_ROOT_DIR=C:/vcpkg" "-D_VCPKG_INSTALLED_DIR=C:/Users/beren/nodable/vcpkg/emscripten" "-DVCPKG_MANIFEST_INSTALL=OFF"
FAILED: [code=1] ../../wasm32-emscripten-dbg/CMakeCache.txt 
"C:/Program Files/CMake/bin/cmake.exe" -E chdir "../../wasm32-emscripten-dbg" "C:/Program Files/CMake/bin/cmake.exe" "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean" "-G" "Ninja" "-DCMAKE_BUILD_TYPE=Debug" "-DCMAKE_INSTALL_PREFIX=C:/vcpkg/packages/dbus_wasm32-emscripten/debug" "-DFETCHCONTENT_FULLY_DISCONNECTED=ON" "-DDBUS_BUILD_TESTS=OFF" "-DDBUS_ENABLE_DOXYGEN_DOCS=OFF" "-DDBUS_ENABLE_XML_DOCS=OFF" "-DDBUS_INSTALL_SYSTEM_LIBS=OFF" "-DDBUS_WITH_GLIB=OFF" "-DTHREADS_PREFER_PTHREAD_FLAG=ON" "-DXSLTPROC_EXECUTABLE=FALSE" "-DCMAKE_INSTALL_SYSCONFDIR=C:/vcpkg/packages/dbus_wasm32-emscripten/etc/dbus" "-DWITH_SYSTEMD_SYSTEMUNITDIR=lib/systemd/system" "-DWITH_SYSTEMD_USERUNITDIR=lib/systemd/user" "-DENABLE_SYSTEMD=OFF" "-DDBUS_BUILD_X11=OFF" "-DCMAKE_REQUIRE_FIND_PACKAGE_X11=OFF" "-DCMAKE_MAKE_PROGRAM=C:\vcpkg\downloads\tools\ninja-1.13.2-windows\ninja.exe" "-DCMAKE_SYSTEM_NAME=Emscripten" "-DBUILD_SHARED_LIBS=ON" "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=C:\Users\beren\nodable\extern\emsdk\upstream\emscripten/cmake/Modules/Platform/Emscripten.cmake" "-DVCPKG_TARGET_TRIPLET=wasm32-emscripten" "-DVCPKG_SET_CHARSET_FLAG=ON" "-DVCPKG_PLATFORM_TOOLSET=external" "-DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON" "-DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON" "-DCMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY=ON" "-DCMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP=TRUE" "-DCMAKE_VERBOSE_MAKEFILE=ON" "-DVCPKG_APPLOCAL_DEPS=OFF" "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DCMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION=ON" "-DVCPKG_CXX_FLAGS=" "-DVCPKG_CXX_FLAGS_RELEASE=" "-DVCPKG_CXX_FLAGS_DEBUG=" "-DVCPKG_C_FLAGS=" "-DVCPKG_C_FLAGS_RELEASE=" "-DVCPKG_C_FLAGS_DEBUG=" "-DVCPKG_CRT_LINKAGE=dynamic" "-DVCPKG_LINKER_FLAGS=" "-DVCPKG_LINKER_FLAGS_RELEASE=" "-DVCPKG_LINKER_FLAGS_DEBUG=" "-DVCPKG_TARGET_ARCHITECTURE=wasm32" "-DCMAKE_INSTALL_LIBDIR:STRING=lib" "-DCMAKE_INSTALL_BINDIR:STRING=bin" "-D_VCPKG_ROOT_DIR=C:/vcpkg" "-D_VCPKG_INSTALLED_DIR=C:/Users/beren/nodable/vcpkg/emscripten" "-DVCPKG_MANIFEST_INSTALL=OFF"
...
Skipped 682 lines
...
-- effectively used disabled warnings for 'WARNINGS_CFLAGS': error=inline;error=overloaded-virtual;error=missing-field-initializers;error=null-dereference;error=strict-aliasing;error=unused-parameter;unused-parameter
-- unsupported warnings for 'WARNINGS_CFLAGS': duplicated-branches;duplicated-cond;logical-op;restrict
-- Performing Test HAVE_CXX_FLAG_Wall
-- Performing Test HAVE_CXX_FLAG_Wall - Success
-- Performing Test HAVE_CXX_FLAG_Warray_bounds
-- Performing Test HAVE_CXX_FLAG_Warray_bounds - Success
-- Performing Test HAVE_CXX_FLAG_Wcast_align
-- Performing Test HAVE_CXX_FLAG_Wcast_align - Success
-- Performing Test HAVE_CXX_FLAG_Wchar_subscripts
-- Performing Test HAVE_CXX_FLAG_Wchar_subscripts - Success
-- Performing Test HAVE_CXX_FLAG_Wdeclaration_after_statement
-- Performing Test HAVE_CXX_FLAG_Wdeclaration_after_statement - Success
-- Performing Test HAVE_CXX_FLAG_Wdouble_promotion
-- Performing Test HAVE_CXX_FLAG_Wdouble_promotion - Success
-- Performing Test HAVE_CXX_FLAG_Wduplicated_branches
-- Performing Test HAVE_CXX_FLAG_Wduplicated_branches - Failed
-- Performing Test HAVE_CXX_FLAG_Wduplicated_cond
-- Performing Test HAVE_CXX_FLAG_Wduplicated_cond - Failed
-- Performing Test HAVE_CXX_FLAG_Wextra
-- Performing Test HAVE_CXX_FLAG_Wextra - Success
-- Performing Test HAVE_CXX_FLAG_Wfloat_equal
-- Performing Test HAVE_CXX_FLAG_Wfloat_equal - Success
-- Performing Test HAVE_CXX_FLAG_Wformat_nonliteral
-- Performing Test HAVE_CXX_FLAG_Wformat_nonliteral - Success
-- Performing Test HAVE_CXX_FLAG_Wformat_security
-- Performing Test HAVE_CXX_FLAG_Wformat_security - Success
-- Performing Test HAVE_CXX_FLAG_Wformat_2
-- Performing Test HAVE_CXX_FLAG_Wformat_2 - Success
-- Performing Test HAVE_CXX_FLAG_Wimplicit_function_declaration
-- Performing Test HAVE_CXX_FLAG_Wimplicit_function_declaration - Success
-- Performing Test HAVE_CXX_FLAG_Winit_self
-- Performing Test HAVE_CXX_FLAG_Winit_self - Success
CMake Warning at cmake/modules/Macros.cmake:229 (message):
  warning 'inline' already specified as disabled, ignored
Call Stack (most recent call first):
  CMakeLists.txt:349 (generate_compiler_warning_flags)


-- Performing Test HAVE_CXX_FLAG_Wjump_misses_init
-- Performing Test HAVE_CXX_FLAG_Wjump_misses_init - Success
-- Performing Test HAVE_CXX_FLAG_Wlogical_op
-- Performing Test HAVE_CXX_FLAG_Wlogical_op - Failed
-- Performing Test HAVE_CXX_FLAG_Wmissing_declarations
-- Performing Test HAVE_CXX_FLAG_Wmissing_declarations - Success
-- Performing Test HAVE_CXX_FLAG_Wmissing_format_attribute
-- Performing Test HAVE_CXX_FLAG_Wmissing_format_attribute - Success
-- Performing Test HAVE_CXX_FLAG_Wmissing_include_dirs
-- Performing Test HAVE_CXX_FLAG_Wmissing_include_dirs - Success
-- Performing Test HAVE_CXX_FLAG_Wmissing_noreturn
-- Performing Test HAVE_CXX_FLAG_Wmissing_noreturn - Success
-- Performing Test HAVE_CXX_FLAG_Wmissing_prototypes
-- Performing Test HAVE_CXX_FLAG_Wmissing_prototypes - Success
-- Performing Test HAVE_CXX_FLAG_Wnested_externs
-- Performing Test HAVE_CXX_FLAG_Wnested_externs - Success
-- Performing Test HAVE_CXX_FLAG_Wno_error_missing_field_initializers
-- Performing Test HAVE_CXX_FLAG_Wno_error_missing_field_initializers - Success
-- Performing Test HAVE_CXX_FLAG_Wno_error_unused_label
-- Performing Test HAVE_CXX_FLAG_Wno_error_unused_label - Success
-- Performing Test HAVE_CXX_FLAG_Wno_error_unused_parameter
-- Performing Test HAVE_CXX_FLAG_Wno_error_unused_parameter - Success
-- Performing Test HAVE_CXX_FLAG_Wno_missing_field_initializers
-- Performing Test HAVE_CXX_FLAG_Wno_missing_field_initializers - Success
-- Performing Test HAVE_CXX_FLAG_Wno_unused_label
-- Performing Test HAVE_CXX_FLAG_Wno_unused_label - Success
-- Performing Test HAVE_CXX_FLAG_Wno_unused_parameter
-- Performing Test HAVE_CXX_FLAG_Wno_unused_parameter - Success
-- Performing Test HAVE_CXX_FLAG_Wnull_dereference
-- Performing Test HAVE_CXX_FLAG_Wnull_dereference - Success
-- Performing Test HAVE_CXX_FLAG_Wold_style_definition
-- Performing Test HAVE_CXX_FLAG_Wold_style_definition - Success
-- Performing Test HAVE_CXX_FLAG_Wpacked
-- Performing Test HAVE_CXX_FLAG_Wpacked - Success
-- Performing Test HAVE_CXX_FLAG_Wpointer_arith
-- Performing Test HAVE_CXX_FLAG_Wpointer_arith - Success
-- Performing Test HAVE_CXX_FLAG_Wpointer_sign
-- Performing Test HAVE_CXX_FLAG_Wpointer_sign - Success
-- Performing Test HAVE_CXX_FLAG_Wredundant_decls
-- Performing Test HAVE_CXX_FLAG_Wredundant_decls - Success
-- Performing Test HAVE_CXX_FLAG_Wrestrict
-- Performing Test HAVE_CXX_FLAG_Wrestrict - Failed
-- Performing Test HAVE_CXX_FLAG_Wreturn_type
-- Performing Test HAVE_CXX_FLAG_Wreturn_type - Success
-- Performing Test HAVE_CXX_FLAG_Wshadow
-- Performing Test HAVE_CXX_FLAG_Wshadow - Success
-- Performing Test HAVE_CXX_FLAG_Wsign_compare
-- Performing Test HAVE_CXX_FLAG_Wsign_compare - Success
-- Performing Test HAVE_CXX_FLAG_Wstrict_aliasing
-- Performing Test HAVE_CXX_FLAG_Wstrict_aliasing - Success
-- Performing Test HAVE_CXX_FLAG_Wstrict_prototypes
-- Performing Test HAVE_CXX_FLAG_Wstrict_prototypes - Success
-- Performing Test HAVE_CXX_FLAG_Wswitch_default
-- Performing Test HAVE_CXX_FLAG_Wswitch_default - Success
-- Performing Test HAVE_CXX_FLAG_Wswitch_enum
-- Performing Test HAVE_CXX_FLAG_Wswitch_enum - Success
-- Performing Test HAVE_CXX_FLAG_Wundef
-- Performing Test HAVE_CXX_FLAG_Wundef - Success
-- Performing Test HAVE_CXX_FLAG_Wunused_but_set_variable
-- Performing Test HAVE_CXX_FLAG_Wunused_but_set_variable - Success
-- Performing Test HAVE_CXX_FLAG_Wwrite_strings
-- Performing Test HAVE_CXX_FLAG_Wwrite_strings - Success
-- Performing Test HAVE_CXX_FLAG_Wno_error_inline
-- Performing Test HAVE_CXX_FLAG_Wno_error_inline - Success
-- Performing Test HAVE_CXX_FLAG_Wno_error_overloaded_virtual
-- Performing Test HAVE_CXX_FLAG_Wno_error_overloaded_virtual - Success
-- Performing Test HAVE_CXX_FLAG_Wno_error_null_dereference
-- Performing Test HAVE_CXX_FLAG_Wno_error_null_dereference - Success
-- Performing Test HAVE_CXX_FLAG_Wno_error_strict_aliasing
-- Performing Test HAVE_CXX_FLAG_Wno_error_strict_aliasing - Success
CMake Warning at cmake/modules/Macros.cmake:246 (message):
  disabled warning 'inline' already specified as warning, ignored
Call Stack (most recent call first):
  CMakeLists.txt:349 (generate_compiler_warning_flags)


-- effectively used warnings for 'WARNINGS_CXXFLAGS': all;array-bounds;cast-align;char-subscripts;declaration-after-statement;double-promotion;extra;float-equal;format-nonliteral;format-security;format=2;implicit-function-declaration;init-self;jump-misses-init;missing-declarations;missing-format-attribute;missing-include-dirs;missing-noreturn;missing-prototypes;nested-externs;no-error=missing-field-initializers;no-error=unused-label;no-error=unused-parameter;no-missing-field-initializers;no-unused-label;no-unused-parameter;null-dereference;old-style-definition;packed;pointer-arith;pointer-sign;redundant-decls;return-type;shadow;sign-compare;strict-aliasing;strict-prototypes;switch-default;switch-enum;undef;unused-but-set-variable;write-strings
-- effectively used disabled warnings for 'WARNINGS_CXXFLAGS': error=inline;error=overloaded-virtual;error=missing-field-initializers;error=null-dereference;error=strict-aliasing;error=unused-parameter;unused-parameter
-- unsupported warnings for 'WARNINGS_CXXFLAGS': duplicated-branches;duplicated-cond;logical-op;restrict
CMake Error at CMakeLists.txt:574 (message):
  cannot autodetect session socket directory when crosscompiling, pass
  -DDBUS_SESSION_SOCKET_DIR=...


-- Configuring incomplete, errors occurred!
ninja: build stopped: subcommand failed.
```
</details>

<details><summary>C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-rel-CMakeCache.txt.log</summary>

```
# This is the CMakeCache file.
# For build in directory: c:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel
# It was generated by CMake: C:/Program Files/CMake/bin/cmake.exe
# You can edit this file to change values found and used by cmake.
# If you do not want to change any of the values, simply exit the editor.
# If you do want to change a value, simply edit, save, and exit the editor.
# The syntax for the file is as follows:
# KEY:TYPE=VALUE
# KEY is the name of a variable in the cache.
# TYPE is a hint to GUIs for the type of VALUE, DO NOT EDIT TYPE!.
# VALUE is the current value for the KEY.

########################
# EXTERNAL cache entries
########################

//No help, variable specified on the command line.
BUILD_SHARED_LIBS:UNINITIALIZED=ON

//Header providing backtrace(3) facility
Backtrace_HEADER:STRING=backtrace.h

//Path to a file.
Backtrace_INCLUDE_DIR:PATH=Backtrace_INCLUDE_DIR-NOTFOUND

//Path to a library.
Backtrace_LIBRARY:FILEPATH=Backtrace_LIBRARY-NOTFOUND

//Path to a program.
CMAKE_ADDR2LINE:FILEPATH=CMAKE_ADDR2LINE-NOTFOUND

//Choose the type of build, options are: None Debug Release RelWithDebInfo
// MinSizeRel ...
CMAKE_BUILD_TYPE:STRING=Release

//Path to the emulator for the target system.
CMAKE_CROSSCOMPILING_EMULATOR:FILEPATH=C:/Program Files/nodejs/node.exe

//Flags used by the CXX compiler during all build types.
CMAKE_CXX_FLAGS:STRING=

//Flags used by the CXX compiler during DEBUG builds.
CMAKE_CXX_FLAGS_DEBUG:STRING=-g

//Flags used by the CXX compiler during MINSIZEREL builds.
CMAKE_CXX_FLAGS_MINSIZEREL:STRING=-Os -DNDEBUG

//Flags used by the CXX compiler during RELEASE builds.
CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG

//Flags used by the CXX compiler during RELWITHDEBINFO builds.
CMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=-O2 -g -DNDEBUG

//`clang-scan-deps` dependency scanner
CMAKE_C_COMPILER_CLANG_SCAN_DEPS:FILEPATH=C:/Program Files/LLVM/bin/clang-scan-deps.exe

//Flags used by the C compiler during all build types.
CMAKE_C_FLAGS:STRING=

//Flags used by the C compiler during DEBUG builds.
CMAKE_C_FLAGS_DEBUG:STRING=-g

//Flags used by the C compiler during MINSIZEREL builds.
CMAKE_C_FLAGS_MINSIZEREL:STRING=-Os -DNDEBUG

//Flags used by the C compiler during RELEASE builds.
CMAKE_C_FLAGS_RELEASE:STRING=-O3 -DNDEBUG

//Flags used by the C compiler during RELWITHDEBINFO builds.
CMAKE_C_FLAGS_RELWITHDEBINFO:STRING=-O2 -g -DNDEBUG

//Path to a program.
CMAKE_DLLTOOL:FILEPATH=C:/Program Files/LLVM/bin/llvm-dlltool.exe

//No help, variable specified on the command line.
CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION:UNINITIALIZED=ON

//Flags used by the linker during all build types.
CMAKE_EXE_LINKER_FLAGS:STRING=

//Flags used by the linker during DEBUG builds.
CMAKE_EXE_LINKER_FLAGS_DEBUG:STRING=

//Flags used by the linker during MINSIZEREL builds.
CMAKE_EXE_LINKER_FLAGS_MINSIZEREL:STRING=

//Flags used by the linker during RELEASE builds.
CMAKE_EXE_LINKER_FLAGS_RELEASE:STRING=

//Flags used by the linker during RELWITHDEBINFO builds.
CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO:STRING=

//Enable/Disable output of build database during the build.
CMAKE_EXPORT_BUILD_DATABASE:BOOL=

//Enable/Disable output of compile commands during generation.
CMAKE_EXPORT_COMPILE_COMMANDS:BOOL=

//No help, variable specified on the command line.
CMAKE_EXPORT_NO_PACKAGE_REGISTRY:UNINITIALIZED=ON

//No help, variable specified on the command line.
CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY:UNINITIALIZED=ON

//No help, variable specified on the command line.
CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY:UNINITIALIZED=ON

//Value Computed by CMake.
CMAKE_FIND_PACKAGE_REDIRECTS_DIR:STATIC=C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/pkgRedirects

//No help, variable specified on the command line.
CMAKE_INSTALL_BINDIR:STRING=bin

//Read-only architecture-independent data (DATAROOTDIR)
CMAKE_INSTALL_DATADIR:PATH=

//Read-only architecture-independent data root (share)
CMAKE_INSTALL_DATAROOTDIR:PATH=share

//Documentation root (DATAROOTDIR/doc/PROJECT_NAME)
CMAKE_INSTALL_DOCDIR:PATH=

//C header files (include)
CMAKE_INSTALL_INCLUDEDIR:PATH=include

//Info documentation (DATAROOTDIR/info)
CMAKE_INSTALL_INFODIR:PATH=

...
Skipped 1496 lines
...
X11_Xss_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xt_INCLUDE_PATH
X11_Xt_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xt_LIB
X11_Xt_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xtst_INCLUDE_PATH
X11_Xtst_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xtst_LIB
X11_Xtst_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xutil_INCLUDE_PATH
X11_Xutil_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xv_INCLUDE_PATH
X11_Xv_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xv_LIB
X11_Xv_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86misc_INCLUDE_PATH
X11_Xxf86misc_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86misc_LIB
X11_Xxf86misc_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86vm_INCLUDE_PATH
X11_Xxf86vm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86vm_LIB
X11_Xxf86vm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_dpms_INCLUDE_PATH
X11_dpms_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_INCLUDE_PATH
X11_xcb_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_LIB
X11_xcb_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_composite_INCLUDE_PATH
X11_xcb_composite_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_composite_LIB
X11_xcb_composite_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_cursor_INCLUDE_PATH
X11_xcb_cursor_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_cursor_LIB
X11_xcb_cursor_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_damage_INCLUDE_PATH
X11_xcb_damage_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_damage_LIB
X11_xcb_damage_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dpms_INCLUDE_PATH
X11_xcb_dpms_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dpms_LIB
X11_xcb_dpms_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri2_INCLUDE_PATH
X11_xcb_dri2_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri2_LIB
X11_xcb_dri2_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri3_INCLUDE_PATH
X11_xcb_dri3_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri3_LIB
X11_xcb_dri3_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_errors_INCLUDE_PATH
X11_xcb_errors_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_errors_LIB
X11_xcb_errors_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_ewmh_INCLUDE_PATH
X11_xcb_ewmh_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_ewmh_LIB
X11_xcb_ewmh_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_glx_INCLUDE_PATH
X11_xcb_glx_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_glx_LIB
X11_xcb_glx_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_icccm_INCLUDE_PATH
X11_xcb_icccm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_icccm_LIB
X11_xcb_icccm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_image_INCLUDE_PATH
X11_xcb_image_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_image_LIB
X11_xcb_image_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_keysyms_INCLUDE_PATH
X11_xcb_keysyms_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_keysyms_LIB
X11_xcb_keysyms_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_present_INCLUDE_PATH
X11_xcb_present_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_present_LIB
X11_xcb_present_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_randr_INCLUDE_PATH
X11_xcb_randr_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_randr_LIB
X11_xcb_randr_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_record_INCLUDE_PATH
X11_xcb_record_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_record_LIB
X11_xcb_record_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_INCLUDE_PATH
X11_xcb_render_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_LIB
X11_xcb_render_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_util_INCLUDE_PATH
X11_xcb_render_util_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_util_LIB
X11_xcb_render_util_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_res_INCLUDE_PATH
X11_xcb_res_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_res_LIB
X11_xcb_res_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_screensaver_INCLUDE_PATH
X11_xcb_screensaver_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_screensaver_LIB
X11_xcb_screensaver_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shape_INCLUDE_PATH
X11_xcb_shape_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shape_LIB
X11_xcb_shape_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shm_INCLUDE_PATH
X11_xcb_shm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shm_LIB
X11_xcb_shm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_sync_INCLUDE_PATH
X11_xcb_sync_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_sync_LIB
X11_xcb_sync_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_util_INCLUDE_PATH
X11_xcb_util_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_util_LIB
X11_xcb_util_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xf86dri_INCLUDE_PATH
X11_xcb_xf86dri_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xf86dri_LIB
X11_xcb_xf86dri_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xfixes_INCLUDE_PATH
X11_xcb_xfixes_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xfixes_LIB
X11_xcb_xfixes_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinerama_INCLUDE_PATH
X11_xcb_xinerama_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinerama_LIB
X11_xcb_xinerama_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinput_INCLUDE_PATH
X11_xcb_xinput_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinput_LIB
X11_xcb_xinput_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xkb_LIB
X11_xcb_xkb_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xrm_INCLUDE_PATH
X11_xcb_xrm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xrm_LIB
X11_xcb_xrm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xtest_INCLUDE_PATH
X11_xcb_xtest_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xtest_LIB
X11_xcb_xtest_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xv_INCLUDE_PATH
X11_xcb_xv_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xv_LIB
X11_xcb_xv_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xvmc_INCLUDE_PATH
X11_xcb_xvmc_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xvmc_LIB
X11_xcb_xvmc_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_INCLUDE_PATH
X11_xkbcommon_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_LIB
X11_xkbcommon_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_X11_INCLUDE_PATH
X11_xkbcommon_X11_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_X11_LIB
X11_xkbcommon_X11_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbfile_INCLUDE_PATH
X11_xkbfile_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbfile_LIB
X11_xkbfile_LIB-ADVANCED:INTERNAL=1
//Making sure VCPKG_MANIFEST_MODE doesn't change
Z_VCPKG_CHECK_MANIFEST_MODE:INTERNAL=OFF
//Vcpkg root directory
Z_VCPKG_ROOT_DIR:INTERNAL=C:/vcpkg
//Have symbol backtrace
_Backtrace_SYM_FOUND:INTERNAL=
//CMAKE_INSTALL_PREFIX during last run
_GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX:INTERNAL=C:/vcpkg/packages/dbus_wasm32-emscripten
__pkg_config_checked_PC_LibGLIB2:INTERNAL=1

```
</details>

<details><summary>C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-dbg-CMakeCache.txt.log</summary>

```
# This is the CMakeCache file.
# For build in directory: c:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg
# It was generated by CMake: C:/Program Files/CMake/bin/cmake.exe
# You can edit this file to change values found and used by cmake.
# If you do not want to change any of the values, simply exit the editor.
# If you do want to change a value, simply edit, save, and exit the editor.
# The syntax for the file is as follows:
# KEY:TYPE=VALUE
# KEY is the name of a variable in the cache.
# TYPE is a hint to GUIs for the type of VALUE, DO NOT EDIT TYPE!.
# VALUE is the current value for the KEY.

########################
# EXTERNAL cache entries
########################

//No help, variable specified on the command line.
BUILD_SHARED_LIBS:UNINITIALIZED=ON

//Header providing backtrace(3) facility
Backtrace_HEADER:STRING=backtrace.h

//Path to a file.
Backtrace_INCLUDE_DIR:PATH=Backtrace_INCLUDE_DIR-NOTFOUND

//Path to a library.
Backtrace_LIBRARY:FILEPATH=Backtrace_LIBRARY-NOTFOUND

//Path to a program.
CMAKE_ADDR2LINE:FILEPATH=CMAKE_ADDR2LINE-NOTFOUND

//Choose the type of build, options are: None Debug Release RelWithDebInfo
// MinSizeRel ...
CMAKE_BUILD_TYPE:STRING=Debug

//Path to the emulator for the target system.
CMAKE_CROSSCOMPILING_EMULATOR:FILEPATH=C:/Program Files/nodejs/node.exe

//Flags used by the CXX compiler during all build types.
CMAKE_CXX_FLAGS:STRING=

//Flags used by the CXX compiler during DEBUG builds.
CMAKE_CXX_FLAGS_DEBUG:STRING=-g

//Flags used by the CXX compiler during MINSIZEREL builds.
CMAKE_CXX_FLAGS_MINSIZEREL:STRING=-Os -DNDEBUG

//Flags used by the CXX compiler during RELEASE builds.
CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG

//Flags used by the CXX compiler during RELWITHDEBINFO builds.
CMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=-O2 -g -DNDEBUG

//`clang-scan-deps` dependency scanner
CMAKE_C_COMPILER_CLANG_SCAN_DEPS:FILEPATH=C:/Program Files/LLVM/bin/clang-scan-deps.exe

//Flags used by the C compiler during all build types.
CMAKE_C_FLAGS:STRING=

//Flags used by the C compiler during DEBUG builds.
CMAKE_C_FLAGS_DEBUG:STRING=-g

//Flags used by the C compiler during MINSIZEREL builds.
CMAKE_C_FLAGS_MINSIZEREL:STRING=-Os -DNDEBUG

//Flags used by the C compiler during RELEASE builds.
CMAKE_C_FLAGS_RELEASE:STRING=-O3 -DNDEBUG

//Flags used by the C compiler during RELWITHDEBINFO builds.
CMAKE_C_FLAGS_RELWITHDEBINFO:STRING=-O2 -g -DNDEBUG

//Path to a program.
CMAKE_DLLTOOL:FILEPATH=C:/Program Files/LLVM/bin/llvm-dlltool.exe

//No help, variable specified on the command line.
CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION:UNINITIALIZED=ON

//Flags used by the linker during all build types.
CMAKE_EXE_LINKER_FLAGS:STRING=

//Flags used by the linker during DEBUG builds.
CMAKE_EXE_LINKER_FLAGS_DEBUG:STRING=

//Flags used by the linker during MINSIZEREL builds.
CMAKE_EXE_LINKER_FLAGS_MINSIZEREL:STRING=

//Flags used by the linker during RELEASE builds.
CMAKE_EXE_LINKER_FLAGS_RELEASE:STRING=

//Flags used by the linker during RELWITHDEBINFO builds.
CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO:STRING=

//Enable/Disable output of build database during the build.
CMAKE_EXPORT_BUILD_DATABASE:BOOL=

//Enable/Disable output of compile commands during generation.
CMAKE_EXPORT_COMPILE_COMMANDS:BOOL=

//No help, variable specified on the command line.
CMAKE_EXPORT_NO_PACKAGE_REGISTRY:UNINITIALIZED=ON

//No help, variable specified on the command line.
CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY:UNINITIALIZED=ON

//No help, variable specified on the command line.
CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY:UNINITIALIZED=ON

//Value Computed by CMake.
CMAKE_FIND_PACKAGE_REDIRECTS_DIR:STATIC=C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/pkgRedirects

//No help, variable specified on the command line.
CMAKE_INSTALL_BINDIR:STRING=bin

//Read-only architecture-independent data (DATAROOTDIR)
CMAKE_INSTALL_DATADIR:PATH=

//Read-only architecture-independent data root (share)
CMAKE_INSTALL_DATAROOTDIR:PATH=share

//Documentation root (DATAROOTDIR/doc/PROJECT_NAME)
...
Skipped 1514 lines
...
X11_Xutil_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xv_INCLUDE_PATH
X11_Xv_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xv_LIB
X11_Xv_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86misc_INCLUDE_PATH
X11_Xxf86misc_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86misc_LIB
X11_Xxf86misc_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86vm_INCLUDE_PATH
X11_Xxf86vm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_Xxf86vm_LIB
X11_Xxf86vm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_dpms_INCLUDE_PATH
X11_dpms_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_INCLUDE_PATH
X11_xcb_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_LIB
X11_xcb_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_composite_INCLUDE_PATH
X11_xcb_composite_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_composite_LIB
X11_xcb_composite_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_cursor_INCLUDE_PATH
X11_xcb_cursor_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_cursor_LIB
X11_xcb_cursor_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_damage_INCLUDE_PATH
X11_xcb_damage_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_damage_LIB
X11_xcb_damage_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dpms_INCLUDE_PATH
X11_xcb_dpms_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dpms_LIB
X11_xcb_dpms_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri2_INCLUDE_PATH
X11_xcb_dri2_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri2_LIB
X11_xcb_dri2_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri3_INCLUDE_PATH
X11_xcb_dri3_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_dri3_LIB
X11_xcb_dri3_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_errors_INCLUDE_PATH
X11_xcb_errors_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_errors_LIB
X11_xcb_errors_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_ewmh_INCLUDE_PATH
X11_xcb_ewmh_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_ewmh_LIB
X11_xcb_ewmh_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_glx_INCLUDE_PATH
X11_xcb_glx_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_glx_LIB
X11_xcb_glx_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_icccm_INCLUDE_PATH
X11_xcb_icccm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_icccm_LIB
X11_xcb_icccm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_image_INCLUDE_PATH
X11_xcb_image_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_image_LIB
X11_xcb_image_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_keysyms_INCLUDE_PATH
X11_xcb_keysyms_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_keysyms_LIB
X11_xcb_keysyms_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_present_INCLUDE_PATH
X11_xcb_present_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_present_LIB
X11_xcb_present_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_randr_INCLUDE_PATH
X11_xcb_randr_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_randr_LIB
X11_xcb_randr_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_record_INCLUDE_PATH
X11_xcb_record_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_record_LIB
X11_xcb_record_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_INCLUDE_PATH
X11_xcb_render_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_LIB
X11_xcb_render_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_util_INCLUDE_PATH
X11_xcb_render_util_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_render_util_LIB
X11_xcb_render_util_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_res_INCLUDE_PATH
X11_xcb_res_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_res_LIB
X11_xcb_res_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_screensaver_INCLUDE_PATH
X11_xcb_screensaver_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_screensaver_LIB
X11_xcb_screensaver_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shape_INCLUDE_PATH
X11_xcb_shape_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shape_LIB
X11_xcb_shape_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shm_INCLUDE_PATH
X11_xcb_shm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_shm_LIB
X11_xcb_shm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_sync_INCLUDE_PATH
X11_xcb_sync_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_sync_LIB
X11_xcb_sync_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_util_INCLUDE_PATH
X11_xcb_util_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_util_LIB
X11_xcb_util_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xf86dri_INCLUDE_PATH
X11_xcb_xf86dri_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xf86dri_LIB
X11_xcb_xf86dri_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xfixes_INCLUDE_PATH
X11_xcb_xfixes_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xfixes_LIB
X11_xcb_xfixes_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinerama_INCLUDE_PATH
X11_xcb_xinerama_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinerama_LIB
X11_xcb_xinerama_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinput_INCLUDE_PATH
X11_xcb_xinput_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xinput_LIB
X11_xcb_xinput_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xkb_LIB
X11_xcb_xkb_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xrm_INCLUDE_PATH
X11_xcb_xrm_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xrm_LIB
X11_xcb_xrm_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xtest_INCLUDE_PATH
X11_xcb_xtest_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xtest_LIB
X11_xcb_xtest_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xv_INCLUDE_PATH
X11_xcb_xv_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xv_LIB
X11_xcb_xv_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xvmc_INCLUDE_PATH
X11_xcb_xvmc_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xcb_xvmc_LIB
X11_xcb_xvmc_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_INCLUDE_PATH
X11_xkbcommon_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_LIB
X11_xkbcommon_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_X11_INCLUDE_PATH
X11_xkbcommon_X11_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbcommon_X11_LIB
X11_xkbcommon_X11_LIB-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbfile_INCLUDE_PATH
X11_xkbfile_INCLUDE_PATH-ADVANCED:INTERNAL=1
//ADVANCED property for variable: X11_xkbfile_LIB
X11_xkbfile_LIB-ADVANCED:INTERNAL=1
//Making sure VCPKG_MANIFEST_MODE doesn't change
Z_VCPKG_CHECK_MANIFEST_MODE:INTERNAL=OFF
//Vcpkg root directory
Z_VCPKG_ROOT_DIR:INTERNAL=C:/vcpkg
//Have symbol backtrace
_Backtrace_SYM_FOUND:INTERNAL=
//CMAKE_INSTALL_PREFIX during last run
_GNUInstallDirs_LAST_CMAKE_INSTALL_PREFIX:INTERNAL=C:/vcpkg/packages/dbus_wasm32-emscripten/debug
__pkg_config_checked_PC_LibGLIB2:INTERNAL=1

```
</details>

<details><summary>C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-rel-CMakeConfigureLog.yaml.log</summary>

```

---
events:
  -
    kind: "find-v1"
    backtrace:
      - "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake:357 (find_program)"
      - "C:/vcpkg/scripts/buildsystems/vcpkg.cmake:209 (include)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/CMakeDetermineSystem.cmake:146 (include)"
      - "CMakeLists.txt:22 (project)"
    mode: "program"
    variable: "NODE_JS_EXECUTABLE"
    description: "Path to a program."
    settings:
      SearchFramework: "NEVER"
      SearchAppBundle: "NEVER"
      CMAKE_FIND_USE_CMAKE_PATH: true
      CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH: true
      CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH: true
      CMAKE_FIND_USE_CMAKE_SYSTEM_PATH: true
      CMAKE_FIND_USE_INSTALL_PREFIX: true
    names:
      - "nodejs"
      - "node"
    candidate_directories:
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/"
      - "C:/Windows/System32/"
      - "C:/Windows/"
      - "C:/Windows/System32/wbem/"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/"
      - "C:/Windows/System32/OpenSSH/"
      - "C:/Users/beren/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Git/mingw64/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/local/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/bin/"
      - "C:/Users/beren/.vscode/extensions/vadimcn.vscode-lldb-1.12.1/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Microsoft VS Code/"
      - "C:/Python314/Scripts/"
      - "C:/Python314/"
      - "C:/Users/beren/Jai/bin/"
      - "C:/Program Files/nodejs/"
      - "C:/ProgramData/chocolatey/bin/"
      - "C:/Users/beren/subread/bin/"
      - "C:/Users/beren/bowtie/"
      - "C:/Program Files/CMake/bin/"
      - "C:/Program Files (x86)/Windows Kits/10/Windows Performance Toolkit/"
      - "C:/Program Files/LLVM/bin/"
      - "C:/Program Files/Docker/Docker/resources/bin/"
      - "C:/Ruby40-x64/bin/"
      - "C:/Users/beren/AppData/Local/Microsoft/WindowsApps/"
      - "C:/Users/beren/AppData/Local/Programs/Microsoft VS Code/bin/"
      - "C:/Users/beren/Jai/jai/bin/"
      - "C:/Users/beren/AppData/Local/Programs/MiKTeX/miktex/bin/x64/"
      - "C:/Users/beren/AppData/Local/Pandoc/"
      - "C:/Users/beren/AppData/Roaming/npm/"
      - "C:/Users/beren/AppData/Local/PowerToys/DSCModules/"
      - "C:/Users/beren/AppData/Local/Programs/Git/cmd/"
      - "C:/vcpkg/"
      - "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64/"
      - "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/"
      - "C:/Users/beren/.vscode/extensions/ms-python.debugpy-2025.18.0-win32-x64/bundled/scripts/noConfigScripts/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/bin/vendor_perl/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/bin/core_perl/"
      - "C:/vcpkg/downloads/tools/ninja-1.13.2-windows/"
      - "/bin/"
      - "/sbin/"
    searched_directories:
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/nodejs.com"
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/nodejs.exe"
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/nodejs"
      - "C:/Windows/System32/nodejs.com"
      - "C:/Windows/System32/nodejs.exe"
      - "C:/Windows/System32/nodejs"
      - "C:/Windows/nodejs.com"
      - "C:/Windows/nodejs.exe"
      - "C:/Windows/nodejs"
      - "C:/Windows/System32/wbem/nodejs.com"
      - "C:/Windows/System32/wbem/nodejs.exe"
      - "C:/Windows/System32/wbem/nodejs"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/nodejs.com"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/nodejs.exe"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/nodejs"
      - "C:/Windows/System32/OpenSSH/nodejs.com"
      - "C:/Windows/System32/OpenSSH/nodejs.exe"
      - "C:/Windows/System32/OpenSSH/nodejs"
      - "C:/Users/beren/bin/nodejs.com"
      - "C:/Users/beren/bin/nodejs.exe"
      - "C:/Users/beren/bin/nodejs"
      - "C:/Users/beren/AppData/Local/Programs/Git/mingw64/bin/nodejs.com"
...
Skipped 100984 lines
...
      - "cmake/modules/Macros.cmake:174 (check_cxx_compiler_flag)"
      - "cmake/modules/Macros.cmake:248 (check_compiler_warning_flag)"
      - "CMakeLists.txt:349 (generate_compiler_warning_flags)"
    checks:
      - "Performing Test HAVE_CXX_FLAG_Wno_error_overloaded_virtual"
    directories:
      source: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-s087a0"
      binary: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-s087a0"
    cmakeVariables:
      CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS: "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/emscan-deps.bat"
      CMAKE_CXX_FLAGS: " -fno-common"
      CMAKE_CXX_FLAGS_DEBUG: "-g"
      CMAKE_EXE_LINKER_FLAGS: ""
      CMAKE_MODULE_PATH: "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake;C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake/modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules"
      VCPKG_CHAINLOAD_TOOLCHAIN_FILE: "C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten/cmake/Modules/Platform/Emscripten.cmake"
      VCPKG_INSTALLED_DIR: "C:/Users/beren/nodable/vcpkg/emscripten"
      VCPKG_PREFER_SYSTEM_LIBS: "OFF"
      VCPKG_TARGET_ARCHITECTURE: "wasm32"
      VCPKG_TARGET_TRIPLET: "wasm32-emscripten"
      Z_VCPKG_ROOT_DIR: "C:/vcpkg"
    buildResult:
      variable: "HAVE_CXX_FLAG_Wno_error_overloaded_virtual"
      cached: true
      stdout: |
        Change Dir: 'C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-s087a0'
        
        Run Build Command(s): C:\\vcpkg\\downloads\\tools\\ninja-1.13.2-windows\\ninja.exe -v cmTC_d541c
        [1/2] C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -DHAVE_CXX_FLAG_Wno_error_overloaded_virtual  -fno-common    -Wno-error=overloaded-virtual -Werror -MD -MT CMakeFiles/cmTC_d541c.dir/src.cxx.o -MF CMakeFiles\\cmTC_d541c.dir\\src.cxx.o.d -o CMakeFiles/cmTC_d541c.dir/src.cxx.o -c C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-s087a0/src.cxx
        [2/2] C:\\Windows\\system32\\cmd.exe /C "cd . && C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -fno-common  CMakeFiles/cmTC_d541c.dir/src.cxx.o -o cmTC_d541c.js   && cd ."
        
      exitCode: 0
  -
    kind: "try_compile-v1"
    backtrace:
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckSourceCompiles.cmake:104 (try_compile)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckCompilerFlag.cmake:18 (cmake_check_source_compiles)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/CheckCXXCompilerFlag.cmake:103 (cmake_check_compiler_flag)"
      - "cmake/modules/Macros.cmake:174 (check_cxx_compiler_flag)"
      - "cmake/modules/Macros.cmake:248 (check_compiler_warning_flag)"
      - "CMakeLists.txt:349 (generate_compiler_warning_flags)"
    checks:
      - "Performing Test HAVE_CXX_FLAG_Wno_error_null_dereference"
    directories:
      source: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1h40n6"
      binary: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1h40n6"
    cmakeVariables:
      CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS: "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/emscan-deps.bat"
      CMAKE_CXX_FLAGS: " -fno-common"
      CMAKE_CXX_FLAGS_DEBUG: "-g"
      CMAKE_EXE_LINKER_FLAGS: ""
      CMAKE_MODULE_PATH: "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake;C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake/modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules"
      VCPKG_CHAINLOAD_TOOLCHAIN_FILE: "C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten/cmake/Modules/Platform/Emscripten.cmake"
      VCPKG_INSTALLED_DIR: "C:/Users/beren/nodable/vcpkg/emscripten"
      VCPKG_PREFER_SYSTEM_LIBS: "OFF"
      VCPKG_TARGET_ARCHITECTURE: "wasm32"
      VCPKG_TARGET_TRIPLET: "wasm32-emscripten"
      Z_VCPKG_ROOT_DIR: "C:/vcpkg"
    buildResult:
      variable: "HAVE_CXX_FLAG_Wno_error_null_dereference"
      cached: true
      stdout: |
        Change Dir: 'C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1h40n6'
        
        Run Build Command(s): C:\\vcpkg\\downloads\\tools\\ninja-1.13.2-windows\\ninja.exe -v cmTC_86ca7
        [1/2] C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -DHAVE_CXX_FLAG_Wno_error_null_dereference  -fno-common    -Wno-error=null-dereference -Werror -MD -MT CMakeFiles/cmTC_86ca7.dir/src.cxx.o -MF CMakeFiles\\cmTC_86ca7.dir\\src.cxx.o.d -o CMakeFiles/cmTC_86ca7.dir/src.cxx.o -c C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1h40n6/src.cxx
        [2/2] C:\\Windows\\system32\\cmd.exe /C "cd . && C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -fno-common  CMakeFiles/cmTC_86ca7.dir/src.cxx.o -o cmTC_86ca7.js   && cd ."
        
      exitCode: 0
  -
    kind: "try_compile-v1"
    backtrace:
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckSourceCompiles.cmake:104 (try_compile)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckCompilerFlag.cmake:18 (cmake_check_source_compiles)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/CheckCXXCompilerFlag.cmake:103 (cmake_check_compiler_flag)"
      - "cmake/modules/Macros.cmake:174 (check_cxx_compiler_flag)"
      - "cmake/modules/Macros.cmake:248 (check_compiler_warning_flag)"
      - "CMakeLists.txt:349 (generate_compiler_warning_flags)"
    checks:
      - "Performing Test HAVE_CXX_FLAG_Wno_error_strict_aliasing"
    directories:
      source: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1vp8qg"
      binary: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1vp8qg"
    cmakeVariables:
      CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS: "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/emscan-deps.bat"
      CMAKE_CXX_FLAGS: " -fno-common"
      CMAKE_CXX_FLAGS_DEBUG: "-g"
      CMAKE_EXE_LINKER_FLAGS: ""
      CMAKE_MODULE_PATH: "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake;C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake/modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules"
      VCPKG_CHAINLOAD_TOOLCHAIN_FILE: "C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten/cmake/Modules/Platform/Emscripten.cmake"
      VCPKG_INSTALLED_DIR: "C:/Users/beren/nodable/vcpkg/emscripten"
      VCPKG_PREFER_SYSTEM_LIBS: "OFF"
      VCPKG_TARGET_ARCHITECTURE: "wasm32"
      VCPKG_TARGET_TRIPLET: "wasm32-emscripten"
      Z_VCPKG_ROOT_DIR: "C:/vcpkg"
    buildResult:
      variable: "HAVE_CXX_FLAG_Wno_error_strict_aliasing"
      cached: true
      stdout: |
        Change Dir: 'C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1vp8qg'
        
        Run Build Command(s): C:\\vcpkg\\downloads\\tools\\ninja-1.13.2-windows\\ninja.exe -v cmTC_7599b
        [1/2] C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -DHAVE_CXX_FLAG_Wno_error_strict_aliasing  -fno-common    -Wno-error=strict-aliasing -Werror -MD -MT CMakeFiles/cmTC_7599b.dir/src.cxx.o -MF CMakeFiles\\cmTC_7599b.dir\\src.cxx.o.d -o CMakeFiles/cmTC_7599b.dir/src.cxx.o -c C:/vcpkg/buildtrees/dbus/wasm32-emscripten-rel/CMakeFiles/CMakeScratch/TryCompile-1vp8qg/src.cxx
        [2/2] C:\\Windows\\system32\\cmd.exe /C "cd . && C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -fno-common  CMakeFiles/cmTC_7599b.dir/src.cxx.o -o cmTC_7599b.js   && cd ."
        
      exitCode: 0
...
```
</details>

<details><summary>C:\vcpkg\buildtrees\dbus\config-wasm32-emscripten-dbg-CMakeConfigureLog.yaml.log</summary>

```

---
events:
  -
    kind: "find-v1"
    backtrace:
      - "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake:357 (find_program)"
      - "C:/vcpkg/scripts/buildsystems/vcpkg.cmake:209 (include)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/CMakeDetermineSystem.cmake:146 (include)"
      - "CMakeLists.txt:22 (project)"
    mode: "program"
    variable: "NODE_JS_EXECUTABLE"
    description: "Path to a program."
    settings:
      SearchFramework: "NEVER"
      SearchAppBundle: "NEVER"
      CMAKE_FIND_USE_CMAKE_PATH: true
      CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH: true
      CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH: true
      CMAKE_FIND_USE_CMAKE_SYSTEM_PATH: true
      CMAKE_FIND_USE_INSTALL_PREFIX: true
    names:
      - "nodejs"
      - "node"
    candidate_directories:
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/"
      - "C:/Windows/System32/"
      - "C:/Windows/"
      - "C:/Windows/System32/wbem/"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/"
      - "C:/Windows/System32/OpenSSH/"
      - "C:/Users/beren/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Git/mingw64/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/local/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/bin/"
      - "C:/Users/beren/.vscode/extensions/vadimcn.vscode-lldb-1.12.1/bin/"
      - "C:/Users/beren/AppData/Local/Programs/Microsoft VS Code/"
      - "C:/Python314/Scripts/"
      - "C:/Python314/"
      - "C:/Users/beren/Jai/bin/"
      - "C:/Program Files/nodejs/"
      - "C:/ProgramData/chocolatey/bin/"
      - "C:/Users/beren/subread/bin/"
      - "C:/Users/beren/bowtie/"
      - "C:/Program Files/CMake/bin/"
      - "C:/Program Files (x86)/Windows Kits/10/Windows Performance Toolkit/"
      - "C:/Program Files/LLVM/bin/"
      - "C:/Program Files/Docker/Docker/resources/bin/"
      - "C:/Ruby40-x64/bin/"
      - "C:/Users/beren/AppData/Local/Microsoft/WindowsApps/"
      - "C:/Users/beren/AppData/Local/Programs/Microsoft VS Code/bin/"
      - "C:/Users/beren/Jai/jai/bin/"
      - "C:/Users/beren/AppData/Local/Programs/MiKTeX/miktex/bin/x64/"
      - "C:/Users/beren/AppData/Local/Pandoc/"
      - "C:/Users/beren/AppData/Roaming/npm/"
      - "C:/Users/beren/AppData/Local/PowerToys/DSCModules/"
      - "C:/Users/beren/AppData/Local/Programs/Git/cmd/"
      - "C:/vcpkg/"
      - "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64/"
      - "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/"
      - "C:/Users/beren/.vscode/extensions/ms-python.debugpy-2025.18.0-win32-x64/bundled/scripts/noConfigScripts/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/bin/vendor_perl/"
      - "C:/Users/beren/AppData/Local/Programs/Git/usr/bin/core_perl/"
      - "C:/vcpkg/downloads/tools/ninja-1.13.2-windows/"
      - "/bin/"
      - "/sbin/"
    searched_directories:
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/nodejs.com"
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/nodejs.exe"
      - "C:/vcpkg/downloads/tools/powershell-core-7.5.4-windows/nodejs"
      - "C:/Windows/System32/nodejs.com"
      - "C:/Windows/System32/nodejs.exe"
      - "C:/Windows/System32/nodejs"
      - "C:/Windows/nodejs.com"
      - "C:/Windows/nodejs.exe"
      - "C:/Windows/nodejs"
      - "C:/Windows/System32/wbem/nodejs.com"
      - "C:/Windows/System32/wbem/nodejs.exe"
      - "C:/Windows/System32/wbem/nodejs"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/nodejs.com"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/nodejs.exe"
      - "C:/Windows/System32/WindowsPowerShell/v1.0/nodejs"
      - "C:/Windows/System32/OpenSSH/nodejs.com"
      - "C:/Windows/System32/OpenSSH/nodejs.exe"
      - "C:/Windows/System32/OpenSSH/nodejs"
      - "C:/Users/beren/bin/nodejs.com"
      - "C:/Users/beren/bin/nodejs.exe"
      - "C:/Users/beren/bin/nodejs"
      - "C:/Users/beren/AppData/Local/Programs/Git/mingw64/bin/nodejs.com"
...
Skipped 101099 lines
...
      - "cmake/modules/Macros.cmake:174 (check_cxx_compiler_flag)"
      - "cmake/modules/Macros.cmake:248 (check_compiler_warning_flag)"
      - "CMakeLists.txt:349 (generate_compiler_warning_flags)"
    checks:
      - "Performing Test HAVE_CXX_FLAG_Wno_error_overloaded_virtual"
    directories:
      source: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-1uzfvr"
      binary: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-1uzfvr"
    cmakeVariables:
      CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS: "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/emscan-deps.bat"
      CMAKE_CXX_FLAGS: " -fno-common"
      CMAKE_CXX_FLAGS_DEBUG: "-g"
      CMAKE_EXE_LINKER_FLAGS: ""
      CMAKE_MODULE_PATH: "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake;C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake/modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules"
      VCPKG_CHAINLOAD_TOOLCHAIN_FILE: "C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten/cmake/Modules/Platform/Emscripten.cmake"
      VCPKG_INSTALLED_DIR: "C:/Users/beren/nodable/vcpkg/emscripten"
      VCPKG_PREFER_SYSTEM_LIBS: "OFF"
      VCPKG_TARGET_ARCHITECTURE: "wasm32"
      VCPKG_TARGET_TRIPLET: "wasm32-emscripten"
      Z_VCPKG_ROOT_DIR: "C:/vcpkg"
    buildResult:
      variable: "HAVE_CXX_FLAG_Wno_error_overloaded_virtual"
      cached: true
      stdout: |
        Change Dir: 'C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-1uzfvr'
        
        Run Build Command(s): C:\\vcpkg\\downloads\\tools\\ninja-1.13.2-windows\\ninja.exe -v cmTC_b7f68
        [1/2] C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -DHAVE_CXX_FLAG_Wno_error_overloaded_virtual  -fno-common    -Wno-error=overloaded-virtual -Werror -MD -MT CMakeFiles/cmTC_b7f68.dir/src.cxx.o -MF CMakeFiles\\cmTC_b7f68.dir\\src.cxx.o.d -o CMakeFiles/cmTC_b7f68.dir/src.cxx.o -c C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-1uzfvr/src.cxx
        [2/2] C:\\Windows\\system32\\cmd.exe /C "cd . && C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -fno-common  CMakeFiles/cmTC_b7f68.dir/src.cxx.o -o cmTC_b7f68.js   && cd ."
        
      exitCode: 0
  -
    kind: "try_compile-v1"
    backtrace:
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckSourceCompiles.cmake:104 (try_compile)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckCompilerFlag.cmake:18 (cmake_check_source_compiles)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/CheckCXXCompilerFlag.cmake:103 (cmake_check_compiler_flag)"
      - "cmake/modules/Macros.cmake:174 (check_cxx_compiler_flag)"
      - "cmake/modules/Macros.cmake:248 (check_compiler_warning_flag)"
      - "CMakeLists.txt:349 (generate_compiler_warning_flags)"
    checks:
      - "Performing Test HAVE_CXX_FLAG_Wno_error_null_dereference"
    directories:
      source: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-5gilqz"
      binary: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-5gilqz"
    cmakeVariables:
      CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS: "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/emscan-deps.bat"
      CMAKE_CXX_FLAGS: " -fno-common"
      CMAKE_CXX_FLAGS_DEBUG: "-g"
      CMAKE_EXE_LINKER_FLAGS: ""
      CMAKE_MODULE_PATH: "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake;C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake/modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules"
      VCPKG_CHAINLOAD_TOOLCHAIN_FILE: "C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten/cmake/Modules/Platform/Emscripten.cmake"
      VCPKG_INSTALLED_DIR: "C:/Users/beren/nodable/vcpkg/emscripten"
      VCPKG_PREFER_SYSTEM_LIBS: "OFF"
      VCPKG_TARGET_ARCHITECTURE: "wasm32"
      VCPKG_TARGET_TRIPLET: "wasm32-emscripten"
      Z_VCPKG_ROOT_DIR: "C:/vcpkg"
    buildResult:
      variable: "HAVE_CXX_FLAG_Wno_error_null_dereference"
      cached: true
      stdout: |
        Change Dir: 'C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-5gilqz'
        
        Run Build Command(s): C:\\vcpkg\\downloads\\tools\\ninja-1.13.2-windows\\ninja.exe -v cmTC_95519
        [1/2] C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -DHAVE_CXX_FLAG_Wno_error_null_dereference  -fno-common    -Wno-error=null-dereference -Werror -MD -MT CMakeFiles/cmTC_95519.dir/src.cxx.o -MF CMakeFiles\\cmTC_95519.dir\\src.cxx.o.d -o CMakeFiles/cmTC_95519.dir/src.cxx.o -c C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-5gilqz/src.cxx
        [2/2] C:\\Windows\\system32\\cmd.exe /C "cd . && C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -fno-common  CMakeFiles/cmTC_95519.dir/src.cxx.o -o cmTC_95519.js   && cd ."
        
      exitCode: 0
  -
    kind: "try_compile-v1"
    backtrace:
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckSourceCompiles.cmake:104 (try_compile)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/Internal/CheckCompilerFlag.cmake:18 (cmake_check_source_compiles)"
      - "C:/Program Files/CMake/share/cmake-4.2/Modules/CheckCXXCompilerFlag.cmake:103 (cmake_check_compiler_flag)"
      - "cmake/modules/Macros.cmake:174 (check_cxx_compiler_flag)"
      - "cmake/modules/Macros.cmake:248 (check_compiler_warning_flag)"
      - "CMakeLists.txt:349 (generate_compiler_warning_flags)"
    checks:
      - "Performing Test HAVE_CXX_FLAG_Wno_error_strict_aliasing"
    directories:
      source: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-s2x2by"
      binary: "C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-s2x2by"
    cmakeVariables:
      CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS: "C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/emscan-deps.bat"
      CMAKE_CXX_FLAGS: " -fno-common"
      CMAKE_CXX_FLAGS_DEBUG: "-g"
      CMAKE_EXE_LINKER_FLAGS: ""
      CMAKE_MODULE_PATH: "C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake;C:/vcpkg/buildtrees/dbus/src/bus-1.16.2-02a7b61930.clean/cmake/modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules;C:/Users/beren/nodable/extern/emsdk/upstream/emscripten/cmake/Modules"
      VCPKG_CHAINLOAD_TOOLCHAIN_FILE: "C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten/cmake/Modules/Platform/Emscripten.cmake"
      VCPKG_INSTALLED_DIR: "C:/Users/beren/nodable/vcpkg/emscripten"
      VCPKG_PREFER_SYSTEM_LIBS: "OFF"
      VCPKG_TARGET_ARCHITECTURE: "wasm32"
      VCPKG_TARGET_TRIPLET: "wasm32-emscripten"
      Z_VCPKG_ROOT_DIR: "C:/vcpkg"
    buildResult:
      variable: "HAVE_CXX_FLAG_Wno_error_strict_aliasing"
      cached: true
      stdout: |
        Change Dir: 'C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-s2x2by'
        
        Run Build Command(s): C:\\vcpkg\\downloads\\tools\\ninja-1.13.2-windows\\ninja.exe -v cmTC_02dc2
        [1/2] C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -DHAVE_CXX_FLAG_Wno_error_strict_aliasing  -fno-common    -Wno-error=strict-aliasing -Werror -MD -MT CMakeFiles/cmTC_02dc2.dir/src.cxx.o -MF CMakeFiles\\cmTC_02dc2.dir\\src.cxx.o.d -o CMakeFiles/cmTC_02dc2.dir/src.cxx.o -c C:/vcpkg/buildtrees/dbus/wasm32-emscripten-dbg/CMakeFiles/CMakeScratch/TryCompile-s2x2by/src.cxx
        [2/2] C:\\Windows\\system32\\cmd.exe /C "cd . && C:\\Users\\beren\\nodable\\extern\\emsdk\\upstream\\emscripten\\em++.bat -fno-common  CMakeFiles/cmTC_02dc2.dir/src.cxx.o -o cmTC_02dc2.js   && cd ."
        
      exitCode: 0
...
```
</details>

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "dependencies": [
    "pkgconf",
    "zlib",
    "libpng",
    "freetype",
    "sdl2",
    "opengl",
    "gtest",
    "nativefiledialog-extended",
    "gl3w",
    "lodepng",
    {
      "name": "glm",
      "version>=": "1.0.3"
    }
  ]
}

```
</details>
