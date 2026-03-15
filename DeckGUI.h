#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "GlossyButtonLookAndFeel.h"
#include "DeckState.h"
#include "ITrackFeature.h"

// DJ deck interface - buttons, sliders, waveform, and hot cues
class DeckGUI : public juce::Component,
                public juce::Button::Listener,
                public juce::Slider::Listener,
                public juce::FileDragAndDropTarget,
                public juce::Timer
{
public:
    DeckGUI(DJAudioPlayer* player, juce::AudioFormatManager& formatManagerToUse, int deckNumber);
    ~DeckGUI() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress& key) override;
    
    void loadTrackWithInfo(const juce::File& file);
    void unloadTrack();
    void setPlaylistComponent(class PlaylistComponent* playlist);
    
    // Hot cue functions
    void setCuePoint(int cueIndex);
    void jumpToCuePoint(int cueIndex);
    void clearCuePoint(int cueIndex);
    void clearAllCuePoints();
    void saveCuePoints();
    void loadCuePoints();

private:
    // C4: Button handlers - split up for single responsibility (one clear purpose each)
    void handlePlayButton();
    void handleStopButton();
    void handleLoopInButton();
    void handleLoopOutButton();
    void handleLoopOffButton();
    void handlePitchUp();
    void handlePitchDown();
    void handleResetSpeed();
    void handleResetEQ();
    void handleHotCueButton(int cueIndex);
    void handleForward10s();
    void handleBackward10s();
    void handleZoomIn();
    void handleZoomOut();
    
    // Helper functions for drawing and data management
    void displayTrackInfo(const juce::File& file);
    void drawJogWheel(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawDownbeatDetector(juce::Graphics& g, juce::Rectangle<int> bounds, float level);
    void paintCueMarkers(juce::Graphics& g);
    juce::File getCueFilePath();
    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds);
    juce::Colour getCueColour(int cueIndex);
    void updateCueButtonColors();
    void saveEQSettings();
    void loadEQSettings();
    juce::File getEQFilePath();
    
    // C4 & DRY: Helper function to reduce redundant EQ slider configuration code
    void configureEQSlider(juce::Slider& slider, juce::Label& label, 
                          const juce::String& labelText, const juce::Colour& colour);
    
    // C2: OOP patterns - Polymorphism, State Machine, Encapsulation
    DeckStateMachine stateMachine;                              // State pattern for deck states
    std::unique_ptr<EQFeature> eqFeature;                       // Polymorphic EQ feature
    std::array<std::unique_ptr<HotCueFeature>, 8> hotCueFeatures; // 8 polymorphic cue points
    
    // UI controls
    juce::TextButton playButton{"PLAY"};
    juce::TextButton stopButton{"STOP"};
    juce::TextButton loopInButton{"LOOP IN"};
    juce::TextButton loopOutButton{"LOOP OUT"};
    juce::TextButton loopOffButton{"LOOP OFF"};
    juce::TextButton pitchUpButton{"PITCH +"};
    juce::TextButton pitchDownButton{"PITCH -"};
    
    // Zoom and skip controls
    juce::TextButton zoomInButton{"ZOOM +"};
    juce::TextButton zoomOutButton{"ZOOM -"};
    juce::TextButton forward10sButton{">> 10s"};
    juce::TextButton backward10sButton{"<< 10s"};
    juce::Label zoomLabel;
    
    juce::Label trackInfoLabel;
    juce::Slider volSlider;
    juce::Label volLabel;
    juce::Slider toneSlider;
    juce::Label toneLabel;
    juce::Slider speedSlider;
    juce::Label speedLabel;
    juce::TextButton resetSpeedButton{"RESET"};
    juce::TextButton resetEQButton{"RESET EQ"};
    juce::TextButton resetCuesButton{"RESET CUES"};
    juce::TextButton resetEQButtonBottom{"RESET EQ"};
    juce::Label hotCueLabel;
    juce::Slider posSlider;
    juce::Label posLabel;
    juce::TextButton hotCueButtons[8]; // Persistent cue storage
    std::array<double, 8> currentCuePoints;                     // Current track cue points
    std::map<juce::String, std::array<double, 8>> cuePointsMap; // Track-to-cues mapping
    juce::Slider lowEQSlider;   // Low frequency equalizer (20Hz-300Hz)
    juce::Label lowEQLabel;
    juce::Slider midEQSlider;   // Mid frequency equalizer (300Hz-4kHz)
    juce::Label midEQLabel;
    juce::Slider highEQSlider;  // High frequency equalizer (4kHz-20kHz)
    juce::Label highEQLabel;

    // C5 & C6: Core dependencies with meaningful names
    DJAudioPlayer* player;                      // Audio playback engine
    juce::AudioFormatManager& formatManager;    // Audio file format handler
    int deckNumber;                             // Deck identifier (1 or 2)
    std::unique_ptr<juce::FileChooser> fileChooser;
    GlossyButtonLookAndFeel glossyLookAndFeel;  // Custom UI styling
    class PlaylistComponent* playlistComponent = nullptr;
    juce::File currentLoadedFile;               // Currently loaded audio file
    float jogWheelRotation = 0.0f;
    float previousAudioLevel = 0.0f;
    double lastBeatTime = 0.0;
    double averageBeatInterval = 0.5;
    int beatCounter = 0;
    double cooldownEndTime = 0.0;
    
    // Waveform zoom state (1 = 30s, 2 = 15s, 3 = 5s)
    int waveformZoomLevel = 1;
    double waveformTimeWindow = 30.0; // seconds
    
    // Active cue for position sync
    int activeCueIndex = -1;
    std::vector<double> recentBeatIntervals;
    double loopInPosition = -1.0;
    double loopOutPosition = -1.0;
    int loopInCounter = 0;
    int loopOutCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};
