cmake_minimum_required(VERSION 3.15)

project(OTODECKS VERSION 1.0.0)

# Add JUCE
add_subdirectory(JUCE)

# Define the application
juce_add_gui_app(Otodecks
    PRODUCT_NAME "Otodecks DJ Application"
    COMPANY_NAME "DJ Software Inc"
    VERSION 1.0.0)

# Add source files
target_sources(Otodecks PRIVATE
    Main.cpp
    MainComponent.cpp
    DJAudioPlayer.cpp
    DeckGUI.cpp
    PlaylistComponent.cpp
    GlossyButtonLookAndFeel.cpp)

# Add header files
target_sources(Otodecks PRIVATE
    MainComponent.h
    DJAudioPlayer.h
    DeckGUI.h
    PlaylistComponent.h
    GlossyButtonLookAndFeel.h
    DeckState.h
    ITrackFeature.h)

# Link JUCE modules
target_link_libraries(Otodecks PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp)

# Compiler definitions
target_compile_definitions(Otodecks PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Otodecks,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Otodecks,JUCE_VERSION>")

# Set C++ standard
target_compile_features(Otodecks PRIVATE cxx_std_17)
