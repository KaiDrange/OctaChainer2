if (NOT DEFINED copy_config)
    message(FATAL_ERROR "copy_config is required")
endif ()

if (NOT copy_config STREQUAL "Release")
    return()
endif ()

if (NOT DEFINED copy_source OR NOT DEFINED copy_destination)
    message(FATAL_ERROR "copy_source and copy_destination are required")
endif ()

if (DEFINED copy_legacy_cleanup AND NOT copy_legacy_cleanup STREQUAL "")
    foreach(legacy_path IN LISTS copy_legacy_cleanup)
        if (EXISTS "${legacy_path}" OR IS_DIRECTORY "${legacy_path}")
            file(REMOVE_RECURSE "${legacy_path}")
        endif ()
    endforeach()
endif ()

get_filename_component(source_name "${copy_source}" NAME)
set(final_destination "${copy_destination}/${source_name}")

file(MAKE_DIRECTORY "${copy_destination}")

if (IS_DIRECTORY "${copy_source}")
    file(REMOVE_RECURSE "${final_destination}")
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_directory "${copy_source}" "${final_destination}"
            RESULT_VARIABLE copy_result
    )
else ()
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${copy_source}" "${final_destination}"
            RESULT_VARIABLE copy_result
    )
endif ()

if (NOT copy_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy release artifact from ${copy_source} to ${final_destination}")
endif ()

if (APPLE AND IS_DIRECTORY "${copy_source}" AND source_name MATCHES "\\.app$")
    set(info_plist "${final_destination}/Contents/Info.plist")
    if (EXISTS "${info_plist}")
        execute_process(
                COMMAND /usr/libexec/PlistBuddy -c "Print :CFBundleExecutable" "${info_plist}"
                OUTPUT_VARIABLE bundle_executable
                OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE plist_result
                ERROR_QUIET
        )

        if (plist_result EQUAL 0 AND bundle_executable STREQUAL "")
            message(FATAL_ERROR "CFBundleExecutable is empty in ${info_plist}")
        endif ()

        if (plist_result EQUAL 0)
            file(GLOB macos_binaries "${final_destination}/Contents/MacOS/*")
            list(LENGTH macos_binaries macos_binary_count)

            if (macos_binary_count EQUAL 1)
                list(GET macos_binaries 0 macos_binary)
                get_filename_component(current_executable "${macos_binary}" NAME)

                if (NOT current_executable STREQUAL bundle_executable)
                    set(expected_executable "${final_destination}/Contents/MacOS/${bundle_executable}")
                    if (EXISTS "${expected_executable}")
                        file(REMOVE "${expected_executable}")
                    endif ()

                    file(RENAME "${macos_binary}" "${expected_executable}")
                endif ()
            endif ()
        endif ()
    endif ()
endif ()
