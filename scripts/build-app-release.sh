#!/bin/bash
source "library.incl.sh"

# Configure build
BUILD_DIR="${get_abs_script_dir}../cmake-build-release"
cmake "${get_abs_script_dir}.." -B cmake-build-release || exit 1

# Build and install (to ../out folder)
cmake --build "${BUILD_DIR}" --config Release --target install || exit 1

echo "Build succeed. Output files are in folder ${get_abs_script_dir}../out"
exit 0