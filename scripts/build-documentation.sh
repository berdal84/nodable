#!/bin/bash

source "library.incl.sh"

# move to docs parent subfolder
cd ${get_abs_script_dir}../docs || (echo "Unable to change directory to ${DIR}../docs" && exit 1)

# generate the documentation from the doxyfile present in that folder
doxygen || (echo "Unable to run doxygen command. Try to install it." && exit 1)

echo "Documentation built. Browse ${DIR}../docs"
exit 0
