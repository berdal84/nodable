#!/bin/bash
source "library.incl.sh"

# get script directory
DIR=${get_abs_script_dir}

# open doc
open "${DIR}../docs/doxygen/html/index.html" || (echo "unable to open documentation, index.html not found" && exit 1)
exit 0