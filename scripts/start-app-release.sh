#!/bin/bash
source "library.incl.sh"

${get_abs_script_dir}../out/app/nodable || ( echo "nodable command returned an error code" && exit 1 )

exit 0