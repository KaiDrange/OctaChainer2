cmake_minimum_required(VERSION 3.22)

project(OctaChainer2 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static MSVC runtime — no VC++ Redistributable required on end-user machines
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(OCTA_FETCH_JUCE "Download JUCE with FetchContent" ON)
option(OCTA_BUILD_VST3 "Build the VST3 plug-in target" ON)
option(OCTA_BUILD_AUDIO_PLUGIN_HOST "Build JUCE's AudioPluginHost example" OFF)
set(OCTA_JUCE_SOURCE_DIR "" CACHE PATH "Path to a local JUCE source checkout")

if (OCTA_JUCE_SOURCE_DIR)
    if (NOT EXISTS "${OCTA_JUCE_SOURCE_DIR}/extras/AudioPluginHost/CMakeLists.txt")
        message(FATAL_ERROR "OCTA_JUCE_SOURCE_DIR must point to a JUCE source tree with extras/AudioPluginHost")
    endif ()

    set(JUCE_SOURCE_DIR "${OCTA_JUCE_SOURCE_DIR}")
    add_subdirectory(
            "${JUCE_SOURCE_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}/_deps/juce-build")
elseif (OCTA_FETCH_JUCE)
    include(FetchContent)
    FetchContent_Declare(
            juce
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.14
    )
    FetchContent_MakeAvailable(juce)
else ()
    find_package(JUCE CONFIG REQUIRED)
endif ()

if (OCTA_BUILD_AUDIO_PLUGIN_HOST)
    if (NOT EXISTS "${JUCE_SOURCE_DIR}/extras/AudioPluginHost/CMakeLists.txt")
        message(FATAL_ERROR "OCTA_BUILD_AUDIO_PLUGIN_HOST requires a JUCE source tree with extras/AudioPluginHost")
    endif ()

    add_subdirectory(
            "${JUCE_SOURCE_DIR}/extras/AudioPluginHost"
            "${CMAKE_CURRENT_BINARY_DIR}/AudioPluginHost")
endif ()

file(GLOB_RECURSE SOURCE_FILES src/UI/*.cpp src/UI/*.h)
set(OCTA_SHARED_SOURCES ${SOURCE_FILES})

set(OCTA_COMMON_COMPILE_DEFINITIONS
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

set(OCTA_COMMON_PUBLIC_LIBS
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

set(OCTA_PLUGIN_FORMATS VST3)

juce_add_gui_app(OctaChainer2_App
        PRODUCT_NAME "OctaChainer 2"
        COMPANY_NAME "Tic Tech"
        BUNDLE_ID "com.octa.octachainer2.app"
)

juce_generate_juce_header(OctaChainer2_App)

target_sources(OctaChainer2_App
        PRIVATE
        src/StandaloneApp.cpp
        src/StandaloneApp.h
        ${OCTA_SHARED_SOURCES}
)

target_compile_definitions(OctaChainer2_App
        PRIVATE
        ${OCTA_COMMON_COMPILE_DEFINITIONS}
)

target_link_libraries(OctaChainer2_App
        PRIVATE
        juce::juce_gui_extra
        PUBLIC
        ${OCTA_COMMON_PUBLIC_LIBS}
)

if (OCTA_PLUGIN_FORMATS)
    juce_add_plugin(OctaChainer2
            COMPANY_NAME "Tic Tech"
            BUNDLE_ID "com.octa.octachainer2"
            IS_SYNTH FALSE
            NEEDS_MIDI_INPUT FALSE
            NEEDS_MIDI_OUTPUT FALSE
            IS_MIDI_EFFECT FALSE
            EDITOR_WANTS_KEYBOARD_FOCUS FALSE
            COPY_PLUGIN_AFTER_BUILD FALSE
            PLUGIN_MANUFACTURER_CODE Octa
            PLUGIN_CODE Oc2a
            FORMATS ${OCTA_PLUGIN_FORMATS}
            PRODUCT_NAME "OctaChainer2"
    )

    juce_generate_juce_header(OctaChainer2)

    target_sources(OctaChainer2
            PRIVATE
            src/PluginEditor.cpp
            src/PluginEditor.h
            src/PluginProcessor.cpp
            src/PluginProcessor.h
            ${OCTA_SHARED_SOURCES}
    )

    target_compile_definitions(OctaChainer2
            PRIVATE
            ${OCTA_COMMON_COMPILE_DEFINITIONS}
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(OctaChainer2
            PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
            PUBLIC
            ${OCTA_COMMON_PUBLIC_LIBS}
    )
endif ()

if (TARGET AudioPluginHost AND TARGET OctaChainer2_VST3)
    add_dependencies(AudioPluginHost OctaChainer2_VST3)
endif ()
