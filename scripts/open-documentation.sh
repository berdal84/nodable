#!/bin/bash

# get script directory
DIR="$(cd "$(dirname -- "$1")" >/dev/null; pwd -P)/$(basename -- "$1")"

# open doc
open "${DIR}../docs/doxygen/html/index.html" || (echo "unable to open documentation, index.html not found" && exit 1)
exit 0