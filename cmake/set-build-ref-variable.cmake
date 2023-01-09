include_guard(GLOBAL)
nodable_log_title_header()

# Get the git short hash
execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_SHORT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the git hash
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the git last tag hash
execute_process(
        COMMAND git rev-list --tags --max-count=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_LAST_TAG_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the git last tag
execute_process(
        COMMAND git describe --tags ${GIT_LAST_TAG_HASH}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_LAST_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# define the version number
if(GITHUB_REF_NAME)
    set(BUILD_REF "${GITHUB_REF_NAME} ${GITHUB_SHA}")
elseif(GIT_LAST_TAG_HASH EQUAL GIT_HASH)
    set(BUILD_REF "${GIT_LAST_TAG} ${GIT_HASH} (local build)")
else()
    set(BUILD_REF "v${PROJECT_VERSION}.x (local build)")
endif()