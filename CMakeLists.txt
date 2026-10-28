cmake_minimum_required(VERSION 3.22)
project(YMYVerb VERSION 1.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Statically link the MSVC runtime so hosts do not need a separately installed
# Visual C++ runtime DLL just to load YMYVerb.
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(YMYVerbData SOURCES Source/Presets/factory_presets.json)

juce_add_plugin(YMYVerb
    COMPANY_NAME "YMY Records"
    PLUGIN_MANUFACTURER_CODE Ymyr
    PLUGIN_CODE Yvrb
    FORMATS VST3 Standalone
    PRODUCT_NAME "YMYVerb"
    BUNDLE_ID "com.ymyrecords.ymyverb"
    VST3_CATEGORIES Fx Reverb
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(YMYVerb PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(YMYVerb PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(YMYVerb PRIVATE YMYVerbData juce::juce_audio_utils juce::juce_audio_processors
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Standalone DSP unit tests (no JUCE needed to run them)
enable_testing()
add_executable(test_dsp Tests/test_dsp.cpp)
add_test(NAME dsp COMMAND test_dsp)
