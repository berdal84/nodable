include_guard(GLOBAL)

# log with nodable prefix
function(ndbl_log arg)
    message("[ndbl::MSG] ${arg}")
endfunction()

# warning with nodable prefix
function(ndbl_warn arg)
    message(WARNING "[ndbl::WRN] ${arg}")
endfunction()

# warning with nodable prefix
function(ndbl_err arg)
    message(FATAL_ERROR "[ndbl::ERR] ${arg}")
endfunction()

# log a nice title
function(ndbl_log_title_header)
    message("\n>>> ${CMAKE_CURRENT_LIST_FILE} ...\n")
endfunction()
