include_guard(GLOBAL)

option(
    PYRAMID_BUNDLE_MINGW_RUNTIME
    "Copy the MinGW compiler runtime DLLs beside Pyramid executables"
    ON
)

function(pyramid_configure_mingw_runtime)
    if(TARGET PyramidMinGWRuntime)
        return()
    endif()

    if(NOT WIN32 OR MSVC OR NOT PYRAMID_BUNDLE_MINGW_RUNTIME)
        add_custom_target(PyramidMinGWRuntime)
        return()
    endif()

    get_filename_component(_pyramid_compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(_pyramid_runtime_names
        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
        libunwind.dll
        libssp-0.dll
    )
    set(_pyramid_runtime_files)
    foreach(_pyramid_runtime_name IN LISTS _pyramid_runtime_names)
        string(MAKE_C_IDENTIFIER "${_pyramid_runtime_name}" _pyramid_runtime_identifier)
        unset(PYRAMID_RUNTIME_${_pyramid_runtime_identifier} CACHE)
        find_file(
            PYRAMID_RUNTIME_${_pyramid_runtime_identifier}
            NAMES "${_pyramid_runtime_name}"
            PATHS "${_pyramid_compiler_directory}"
            NO_DEFAULT_PATH
        )
        if(PYRAMID_RUNTIME_${_pyramid_runtime_identifier})
            list(APPEND _pyramid_runtime_files
                "${PYRAMID_RUNTIME_${_pyramid_runtime_identifier}}")
        endif()
    endforeach()

    if(NOT _pyramid_runtime_files)
        message(FATAL_ERROR
            "No MinGW runtime DLLs were found beside ${CMAKE_CXX_COMPILER}. "
            "Use the MSYS2 UCRT64 compiler or disable PYRAMID_BUNDLE_MINGW_RUNTIME explicitly.")
    endif()

    set(_pyramid_commands
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
    foreach(_pyramid_runtime_file IN LISTS _pyramid_runtime_files)
        list(APPEND _pyramid_commands
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_pyramid_runtime_file}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )
    endforeach()

    add_custom_target(PyramidMinGWRuntime ALL ${_pyramid_commands})
    set_target_properties(PyramidMinGWRuntime PROPERTIES FOLDER "Build")
    set_property(GLOBAL PROPERTY PYRAMID_MINGW_RUNTIME_FILES "${_pyramid_runtime_files}")

    install(
        FILES ${_pyramid_runtime_files}
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT PyramidRuntime
    )

    list(JOIN _pyramid_runtime_files ", " _pyramid_runtime_summary)
    message(STATUS "Pyramid MinGW runtime bundle: ${_pyramid_runtime_summary}")
endfunction()

function(pyramid_require_mingw_runtime target)
    if(TARGET "${target}" AND TARGET PyramidMinGWRuntime)
        add_dependencies("${target}" PyramidMinGWRuntime)
    endif()
endfunction()
