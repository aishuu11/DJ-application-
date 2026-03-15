#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DeckGUI.h"
#include "DJAudioPlayer.h"
#include "PlaylistComponent.h"

/**
 * MainComponent - The main application window containing all DJ components
 * This class manages the overall layout and audio system setup
 */
class MainComponent : public juce::AudioAppComponent,
                      public juce::Slider::Listener,
                      public juce::Button::Listener,
                      public juce::Timer
{
public:
    /**
     * Constructor - Initializes the main component with two decks
     */
    MainComponent();
    
    /**
     * Destructor - Cleans up audio resources
     */
    ~MainComponent() override;

    /**
     * prepareToPlay - Sets up audio processing
     * @param samplesPerBlockExpected Number of samples in each processing block
     * @param sampleRate Sample rate in Hz
     */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    
    /**
     * getNextAudioBlock - Processes audio for mixing two decks
     * @param bufferToFill Audio buffer to fill with mixed output
     */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    
    /**
     * releaseResources - Releases audio resources when stopping
     */
    void releaseResources() override;

    /**
     * paint - Draws the component
     * @param g Graphics context for drawing
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * resized - Called when component is resized, arranges child components
     */
    void resized() override;
    
    /**
     * sliderValueChanged - Handles crossfader movement (R1C: Mixing)
     * @param slider The slider that changed
     */
    void sliderValueChanged(juce::Slider* slider) override;
    
    /**
     * buttonClicked - Handles SYNC button clicks
     * @param button The button that was clicked
     */
    void buttonClicked(juce::Button* button) override;
    
    /**
     * timerCallback - Updates waveform animation
     */
    void timerCallback() override;

private:
    /**
     * drawWaveform - Draws colorful waveform visualization
     * @param g Graphics context
     * @param bounds Area to draw in
     */
    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    // Audio format manager to handle different file types
    juce::AudioFormatManager formatManager;
    
    // Audio players for each deck (R1A: Load audio files)
    DJAudioPlayer player1{formatManager};
    DJAudioPlayer player2{formatManager};
    
    // GUI components for each deck
    DeckGUI deckGUI1{&player1, formatManager, 1};
    DeckGUI deckGUI2{&player2, formatManager, 2};
    
    // Playlist/Music library component
    PlaylistComponent playlistComponent{&player1, &player2, &deckGUI1, &deckGUI2, formatManager};
    
    // Crossfader slider (R1C: Mix between decks)
    juce::Slider crossfader;
    juce::Label crossfaderLabel;
    
    // SYNC button to match deck speeds
    juce::TextButton syncButton{"SYNC"};
    
    // Crossfader multipliers (preserved to not override deck volume sliders)
    double crossfadeMultiplier1 = 1.0;
    double crossfadeMultiplier2 = 1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
