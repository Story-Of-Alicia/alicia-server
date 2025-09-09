function(get_semver)
    set(ServerSemver "0.0.0-unknown (rev: XXXXXXX)" PARENT_SCOPE)

    find_package(Git QUIET)
    if (NOT Git_FOUND)
        message("Git is not available, can't determine the version")
        return()
    endif()

    execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --always
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE SEMVER
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE RETURN_CODE
    )

    if (NOT RETURN_CODE EQUAL 0)
        message("Failed retrieving the tag")
        return()
    endif ()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE REV
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE RETURN_CODE)

    if (NOT RETURN_CODE EQUAL 0)
        message("Failed retrieving the revision")
        return()
    endif ()

    set(ServerSemver "${SEMVER} (rev: ${REV})" PARENT_SCOPE)
endfunction()