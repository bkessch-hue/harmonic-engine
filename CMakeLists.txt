cmake_minimum_required(VERSION 3.24)
project(HarmonicEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── JUCE ──────────────────────────────────────────────────────
add_subdirectory(libs/JUCE)

# ── Source files ──────────────────────────────────────────────
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# ── Main application target ───────────────────────────────────
juce_add_gui_app(HarmonicEngineApp
    PRODUCT_NAME "Harmonic Engine"
    COMPANY_NAME "Harmonic Engine"
    BUNDLE_ID "com.harmonicengine.daw"
    VERSION "0.1.0"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Harmonic Engine needs microphone access for audio recording"
)

target_sources(HarmonicEngineApp PRIVATE ${SOURCES} ${HEADERS})
target_include_directories(HarmonicEngineApp PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(HarmonicEngineApp
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
)

target_compile_definitions(HarmonicEngineApp
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_AU=1
)

# ── Tests ─────────────────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
