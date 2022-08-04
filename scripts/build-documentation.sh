#!/bin/bash

# get script directory
DIR="$(cd "$(dirname -- "$1")" >/dev/null; pwd -P)/$(basename -- "$1")"

# move to docs parent subfolder
cd ${DIR}../docs || (echo "Unable to change directory to ${DIR}../docs" && exit 1)

# generate the documentation from the doxyfile present in that folder
doxygen || (echo "Unable to run doxygen command. Try to install it." && exit 1)

echo "Documentation built. Browse ${DIR}../docs"
exit 0
