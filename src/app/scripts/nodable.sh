#!/bin/bash

function log_message { echo "[MSG|Launcher] $1"; }
function log_erro    { echo "[ERR|Launcher] $1"; }

log_message "Starting ..."

SCRIPT_DIR=`dirname "$0"`
log_message "Will run nodable from ${SCRIPT_DIR} ..."

cd ${SCRIPT_DIR}
log_message "Scaning folder ..."
ls -la
log_message "Launching ./bin/nodable ..."
./bin/nodable || echo log_error "Oups! Something went bad with nodable..."
cd -

log_message "Launcher stopped"
