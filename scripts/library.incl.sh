#!/bin/bash

# get absolute script directory
get_abs_script_dir(){
  DIR=$(dirname -- "$1")
  CLEAN_DIR=$(cd "${DIR}" >/dev/null; pwd -P)
  return "${CLEAN_DIR}/$(basename -- "$1")"
}