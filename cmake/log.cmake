include_guard(GLOBAL)

# log with nodable prefix
function(ndbl_log arg)
    message("[MSG|Nodable] ${arg}")
endfunction()

# warning with nodable prefix
function(ndbl_warn arg)
    message(WARNING "[WRN|Nodable] ${arg}")
endfunction()

# warning with nodable prefix
function(ndbl_err arg)
    message(FATAL_ERROR "[ERR|Nodable] ${arg}")
endfunction()

# log a nice title
function(ndbl_log_title_header)
    ndbl_log("------------<==[ ${CMAKE_CURRENT_LIST_FILE} ]==>------------")
endfunction()
