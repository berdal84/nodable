include_guard(GLOBAL)

# log with nodable prefix
function(nodable_message arg)
    message("[MSG|Nodable] ${arg}")
endfunction()

# warning with nodable prefix
function(nodable_warn arg)
    message(WARNING "[WRN|Nodable] ${arg}")
endfunction()

# warning with nodable prefix
function(nodable_err arg)
    message(FATAL_ERROR "[WRN|Nodable] ${arg}")
endfunction()

# log a nice title
function(nodable_log_title_header)
    nodable_message("------------<==[ ${CMAKE_CURRENT_LIST_FILE} ]==>------------")
endfunction()
