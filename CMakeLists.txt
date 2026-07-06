cmake_minimum_required(VERSION 3.22)

project(OctaChainer2 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static MSVC runtime — no VC++ Redistributable required on end-user machines
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(OCTA_FETCH_JUCE "Download JUCE with FetchContent" ON)
option(OCTA_BUILD_VST3 "Build the VST3 target in addition to standalone" OFF)

if (OCTA_FETCH_JUCE)
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.0
    )
    FetchContent_MakeAvailable(juce)
else ()
    find_package(JUCE CONFIG REQUIRED)
endif ()

set(OCTA_PLUGIN_FORMATS Standalone)
if (OCTA_BUILD_VST3)
    list(APPEND OCTA_PLUGIN_FORMATS VST3)
endif ()

juce_add_plugin(OctaChainer2
    COMPANY_NAME "Tic Tech"
    BUNDLE_ID "com.octa.octachainer2"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
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
)

target_compile_definitions(OctaChainer2
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(OctaChainer2
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

