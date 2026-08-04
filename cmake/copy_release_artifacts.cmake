if (NOT DEFINED copy_config)
    message(FATAL_ERROR "copy_config is required")
endif ()

if (NOT copy_config STREQUAL "Release")
    return()
endif ()

if (NOT DEFINED copy_source OR NOT DEFINED copy_destination)
    message(FATAL_ERROR "copy_source and copy_destination are required")
endif ()

set(octa_macos_codesign_identity "$ENV{OCTA_MACOS_CODESIGN_IDENTITY}")

function(octa_is_mach_o_file path result_var)
    execute_process(
            COMMAND /usr/bin/file -b "${path}"
            OUTPUT_VARIABLE file_output
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE file_result
            ERROR_QUIET
    )

    if (file_result EQUAL 0 AND file_output MATCHES "Mach-O")
        set(${result_var} TRUE PARENT_SCOPE)
    else ()
        set(${result_var} FALSE PARENT_SCOPE)
    endif ()
endfunction()

function(octa_codesign_item path identity)
    if (NOT EXISTS "${path}")
        message(FATAL_ERROR "Cannot codesign missing path: ${path}")
    endif ()

    if (IS_DIRECTORY "${path}")
        file(GLOB entries LIST_DIRECTORIES true "${path}/*")
        foreach(entry IN LISTS entries)
            if (IS_DIRECTORY "${entry}")
                octa_codesign_item("${entry}" "${identity}")
            else ()
                octa_is_mach_o_file("${entry}" is_macho)
                if (is_macho)
                    execute_process(
                            COMMAND codesign --force --timestamp --options runtime --sign "${identity}" "${entry}"
                            RESULT_VARIABLE sign_result
                            OUTPUT_VARIABLE sign_output
                            ERROR_VARIABLE sign_error
                    )
                    if (NOT sign_result EQUAL 0)
                        message(FATAL_ERROR "Failed to codesign ${entry}\n${sign_output}${sign_error}")
                    endif ()
                endif ()
            endif ()
        endforeach()

        get_filename_component(directory_name "${path}" NAME)
        if (directory_name MATCHES "\\.(app|bundle|framework|xpc|appex|plugin|vst3)$")
            execute_process(
                    COMMAND codesign --force --timestamp --options runtime --sign "${identity}" "${path}"
                    RESULT_VARIABLE sign_result
                    OUTPUT_VARIABLE sign_output
                    ERROR_VARIABLE sign_error
            )
            if (NOT sign_result EQUAL 0)
                message(FATAL_ERROR "Failed to codesign bundle ${path}\n${sign_output}${sign_error}")
            endif ()
        endif ()
    else ()
        octa_is_mach_o_file("${path}" is_macho)
        if (is_macho)
            execute_process(
                    COMMAND codesign --force --timestamp --options runtime --sign "${identity}" "${path}"
                    RESULT_VARIABLE sign_result
                    OUTPUT_VARIABLE sign_output
                    ERROR_VARIABLE sign_error
            )
            if (NOT sign_result EQUAL 0)
                message(FATAL_ERROR "Failed to codesign ${path}\n${sign_output}${sign_error}")
            endif ()
        endif ()
    endif ()
endfunction()

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

if (APPLE AND IS_DIRECTORY "${copy_source}" AND source_name MATCHES "\\.(app|bundle|framework|xpc|appex|plugin|vst3)$")
    if (NOT DEFINED octa_macos_codesign_identity OR octa_macos_codesign_identity STREQUAL "")
        message(FATAL_ERROR "OCTA_MACOS_CODESIGN_IDENTITY is required to codesign ${source_name}")
    endif ()
endif ()

if (APPLE AND DEFINED octa_macos_codesign_identity AND NOT octa_macos_codesign_identity STREQUAL "")
    message(STATUS "Codesigning ${final_destination} with Developer ID identity")
    octa_codesign_item("${final_destination}" "${octa_macos_codesign_identity}")
endif ()
