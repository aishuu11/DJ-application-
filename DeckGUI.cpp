/**
 * DeckGUI.cpp - DJ Deck Interface Implementation
 * 
 * PURPOSE: Provides complete GUI and control interface for a single DJ deck
 * 
 * OOP TECHNIQUES USED:
 * - Polymorphism: ITrackFeature base class for HotCue, EQ features
 * - State Pattern: DeckStateMachine for managing playback states
 * - Encapsulation: Private implementation details with public interface
 * - Inheritance: Extends multiple JUCE component interfaces
 * 
 * CODE QUALITY (C1-C7):
 * C1: Header/implementation separation
 * C2: Comprehensive function documentation
 * C3: Consistent indentation and formatting
 * C4: Single-responsibility principle (each function has one clear purpose)
 * C5: Stateless design where possible (parameters over globals)
 * C6: Meaningful, consistent naming conventions
 * C7: Explicit setters that clearly modify state
 */

#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include <fstream>

//==============================================================================
// C2 & C4: Constructor with clear single purpose - initialize deck GUI
// Parameters: player, formatManager, deckNumber (C5: passed as parameters)
DeckGUI::DeckGUI(DJAudioPlayer* _player, juce::AudioFormatManager& formatManagerToUse, int _deckNumber)
    : player(_player),
      formatManager(formatManagerToUse),
      deckNumber(_deckNumber)
{
    // Enable keyboard focus for shortcuts
    setWantsKeyboardFocus(true);
    
    stateMachine.setState(DeckState::Empty);
    
    eqFeature = std::make_unique<EQFeature>();
    for (int i = 0; i < 8; ++i)
    {
        hotCueFeatures[i] = std::make_unique<HotCueFeature>(-1.0);
    }
    
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loopInButton);
    addAndMakeVisible(loopOutButton);
    addAndMakeVisible(pitchUpButton);
    addAndMakeVisible(pitchDownButton);
    addAndMakeVisible(zoomInButton);
    addAndMakeVisible(zoomOutButton);
    addAndMakeVisible(forward10sButton);
    addAndMakeVisible(backward10sButton);
    addAndMakeVisible(zoomLabel);

    playButton.addListener(this);
    stopButton.addListener(this);
    loopInButton.addListener(this);
    loopOutButton.addListener(this);
    pitchUpButton.addListener(this);
    pitchDownButton.addListener(this);
    zoomInButton.addListener(this);
    zoomOutButton.addListener(this);
    forward10sButton.addListener(this);
    backward10sButton.addListener(this);
    
    auto specialPink = juce::Colour(0xffff69b4);
    playButton.setColour(juce::TextButton::buttonColourId, specialPink);
    playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    stopButton.setColour(juce::TextButton::buttonColourId, specialPink);
    stopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loopInButton.setColour(juce::TextButton::buttonColourId, specialPink);
    loopInButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loopOutButton.setColour(juce::TextButton::buttonColourId, specialPink);
    loopOutButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    pitchUpButton.setColour(juce::TextButton::buttonColourId, specialPink);
    pitchUpButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    pitchDownButton.setColour(juce::TextButton::buttonColourId, specialPink);
    pitchDownButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    // Zoom and skip buttons styling
    auto zoomColor = juce::Colour(0xff00d4ff); // Cyan
    auto skipColor = juce::Colour(0xff9b59b6); // Purple
    
    zoomInButton.setColour(juce::TextButton::buttonColourId, zoomColor);
    zoomInButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    zoomOutButton.setColour(juce::TextButton::buttonColourId, zoomColor);
    zoomOutButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    forward10sButton.setColour(juce::TextButton::buttonColourId, skipColor);
    forward10sButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    backward10sButton.setColour(juce::TextButton::buttonColourId, skipColor);
    backward10sButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    zoomLabel.setText("ZOOM 1x", juce::dontSendNotification); // Show initial zoom level
    zoomLabel.setColour(juce::Label::textColourId, zoomColor);
    zoomLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    zoomLabel.setJustificationType(juce::Justification::centred);
    
    playButton.setLookAndFeel(&glossyLookAndFeel);
    stopButton.setLookAndFeel(&glossyLookAndFeel);
    loopInButton.setLookAndFeel(&glossyLookAndFeel);
    loopOutButton.setLookAndFeel(&glossyLookAndFeel);
    pitchUpButton.setLookAndFeel(&glossyLookAndFeel);
    pitchDownButton.setLookAndFeel(&glossyLookAndFeel);
    zoomInButton.setLookAndFeel(&glossyLookAndFeel);
    zoomOutButton.setLookAndFeel(&glossyLookAndFeel);
    forward10sButton.setLookAndFeel(&glossyLookAndFeel);
    backward10sButton.setLookAndFeel(&glossyLookAndFeel);
    
    addAndMakeVisible(trackInfoLabel);
    trackInfoLabel.setText("No track loaded", juce::dontSendNotification);
    trackInfoLabel.setJustificationType(juce::Justification::centred);
    trackInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff1493));
    trackInfoLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff0a0e1a));
    trackInfoLabel.setFont(juce::Font(13.0f, juce::Font::bold));

    addAndMakeVisible(volSlider);
    addAndMakeVisible(volLabel);
    volSlider.setRange(0.0, 1.0);
    volSlider.setSliderStyle(juce::Slider::LinearVertical);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 40, 18);
    volSlider.setNumDecimalPlacesToDisplay(2);
    volSlider.addListener(this);
    volSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00ff41));
    volSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00ff41).withAlpha(0.5f));
    volSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1f35));
    volLabel.setText("VOLUME", juce::dontSendNotification);
    volLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ff41));
    volLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    volLabel.setJustificationType(juce::Justification::centred);
    // Set initial volume - do this AFTER adding listener to ensure proper initialization
    player->setGain(0.5);
    volSlider.setValue(0.5, juce::dontSendNotification);

    addAndMakeVisible(toneSlider);
    addAndMakeVisible(toneLabel);
    toneSlider.setRange(0.0, 1.0);
    toneSlider.setValue(0.5);
    toneSlider.setSliderStyle(juce::Slider::LinearVertical);
    toneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 40, 18);
    toneSlider.setNumDecimalPlacesToDisplay(2);
    toneSlider.addListener(this);
    toneSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffaa00));
    toneSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xffffaa00).withAlpha(0.5f));
    toneSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1f35));
    toneLabel.setText("TONE", juce::dontSendNotification);
    toneLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffaa00));
    toneLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    toneLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(speedSlider);
    addAndMakeVisible(speedLabel);
    addAndMakeVisible(resetSpeedButton);
    addAndMakeVisible(resetEQButton);
    speedSlider.setRange(0.5, 2.0);
    speedSlider.setValue(1.0);
    speedSlider.setSliderStyle(juce::Slider::LinearVertical);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 40, 18);
    speedSlider.setNumDecimalPlacesToDisplay(2);
    speedSlider.addListener(this);
    speedSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffff3b3b));
    speedSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xffff3b3b).withAlpha(0.5f));
    speedSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1f35));
    resetSpeedButton.addListener(this);
    resetSpeedButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff9b59b6));
    resetSpeedButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    resetSpeedButton.setLookAndFeel(&glossyLookAndFeel);
    resetEQButton.addListener(this);
    resetEQButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3498db));
    resetEQButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    resetEQButton.setLookAndFeel(&glossyLookAndFeel);
    speedLabel.setText("SPEED", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff3b3b));
    speedLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    speedLabel.setJustificationType(juce::Justification::centred);
    player->setSpeed(1.0);
    
    addAndMakeVisible(posSlider);
    addAndMakeVisible(posLabel);
    posSlider.setRange(0.0, 1.0);
    posSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    posSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    posSlider.setNumDecimalPlacesToDisplay(2);
    posSlider.addListener(this);
    posSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d9ff));
    posSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00d9ff).withAlpha(0.7f));
    posSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1f35));
    posLabel.setText("POSITION", juce::dontSendNotification);
    posLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d9ff));
    posLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    posLabel.attachToComponent(&posSlider, false);
    
    addAndMakeVisible(hotCueLabel);
    hotCueLabel.setText("HOT CUES", juce::dontSendNotification);
    hotCueLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffa500));
    hotCueLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    hotCueLabel.setJustificationType(juce::Justification::centred);
    
    for (int i = 0; i < 8; ++i)
    {
        addAndMakeVisible(hotCueButtons[i]);
        hotCueButtons[i].setButtonText(juce::String(i + 1));
        hotCueButtons[i].addListener(this);
        hotCueButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
        hotCueButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        hotCueButtons[i].setLookAndFeel(&glossyLookAndFeel);
        currentCuePoints[i] = -1.0;
    }
    
    addAndMakeVisible(loopOffButton);
    loopOffButton.addListener(this);
    loopOffButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff6600)); // Bright orange
    loopOffButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loopOffButton.setLookAndFeel(&glossyLookAndFeel);
    loopOffButton.setTooltip("Turn off looping");
    juce::Logger::writeToLog("Loop Off button initialized");
    
    addAndMakeVisible(resetCuesButton);
    resetCuesButton.addListener(this);
    resetCuesButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff3b3b)); // Red
    resetCuesButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    resetCuesButton.setLookAndFeel(&glossyLookAndFeel);
    resetCuesButton.setTooltip("Clear all hot cue points");
    
    addAndMakeVisible(resetEQButtonBottom);
    resetEQButtonBottom.addListener(this);
    resetEQButtonBottom.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3498db)); // Blue
    resetEQButtonBottom.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    resetEQButtonBottom.setLookAndFeel(&glossyLookAndFeel);
    resetEQButtonBottom.setTooltip("Reset all EQ to 0 dB");
    
    // C4 & DRY: Use helper function to configure EQ sliders (reduces code duplication)
    configureEQSlider(lowEQSlider, lowEQLabel, "LOW", juce::Colour(0xff00d9b4));    // Teal
    configureEQSlider(midEQSlider, midEQLabel, "MID", juce::Colour(0xffffa500));    // Orange
    configureEQSlider(highEQSlider, highEQLabel, "HIGH", juce::Colour(0xffff1493)); // Pink
    
    loadEQSettings();

    startTimer(50);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
    
    playButton.setLookAndFeel(nullptr);
    stopButton.setLookAndFeel(nullptr);
    loopInButton.setLookAndFeel(nullptr);
    loopOutButton.setLookAndFeel(nullptr);
    loopOffButton.setLookAndFeel(nullptr);
    pitchUpButton.setLookAndFeel(nullptr);
    pitchDownButton.setLookAndFeel(nullptr);
    resetSpeedButton.setLookAndFeel(nullptr);
    resetEQButton.setLookAndFeel(nullptr);
    resetCuesButton.setLookAndFeel(nullptr);
    resetEQButtonBottom.setLookAndFeel(nullptr);
    zoomInButton.setLookAndFeel(nullptr);
    zoomOutButton.setLookAndFeel(nullptr);
    forward10sButton.setLookAndFeel(nullptr);
    backward10sButton.setLookAndFeel(nullptr);
    
    for (int i = 0; i < 8; ++i)
    {
        hotCueButtons[i].setLookAndFeel(nullptr);
    }
    
    lowEQSlider.setLookAndFeel(nullptr);
    midEQSlider.setLookAndFeel(nullptr);
    highEQSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void DeckGUI::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Modern gradient background (dark blue-black)
    juce::ColourGradient gradient(juce::Colour(0xff0a0e1a), 0, 0,
                                  juce::Colour(0xff1a1f35), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.3f));
    g.drawRect(bounds, 2);
    
    // Inner shadow effect
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(bounds.removeFromTop(1));
    g.fillRect(bounds.removeFromLeft(1));

    // Modern title with glow
    auto titleArea = getLocalBounds().removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("DJ DECK " + juce::String(deckNumber), titleArea, juce::Justification::centred, true);
    
    // Draw jog wheel in the center-left area
    auto jogWheelBounds = juce::Rectangle<int>(20, 80, 200, 200);
    drawJogWheel(g, jogWheelBounds);
    
    // Draw Downbeat Detector on the right side
    auto downbeatBounds = juce::Rectangle<int>(getWidth() - 25, 60, 10, getHeight() - 150);
    float audioLevel = player->getAudioLevel();
    drawDownbeatDetector(g, downbeatBounds, audioLevel);
    
    // R5: Draw visual cue markers on position bar
    paintCueMarkers(g);
}

void DeckGUI::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(25); // Reduced from 30 - smaller space for title
    
    // Track info label at top
    trackInfoLabel.setBounds(area.removeFromTop(22).reduced(5)); // Reduced from 25
    
    // Main layout divided into left and right sections
    auto mainArea = area;
    
    // Left section: jog wheel (240x240) with buttons below it
    auto leftSection = mainArea.removeFromLeft(260);
    
    // Jog wheel at top of left section (reserve space)
    leftSection.removeFromTop(240);
    
    // Buttons below jog wheel - 3 rows: PLAY/STOP, LOOP IN/OUT, PITCH +/-
    leftSection.removeFromTop(10); // Small gap
    auto buttonArea = leftSection.removeFromTop(165); // Taller to fit 3 rows
    auto topRowButtons = buttonArea.removeFromTop(50);
    auto midRowButtons = buttonArea.removeFromTop(50);
    auto bottomRowButtons = buttonArea.removeFromTop(50);
    
    // Top row: PLAY and STOP
    playButton.setBounds(topRowButtons.removeFromLeft(topRowButtons.getWidth() / 2).reduced(3));
    stopButton.setBounds(topRowButtons.reduced(3));
    
    // Middle row: LOOP IN and LOOP OUT
    loopInButton.setBounds(midRowButtons.removeFromLeft(midRowButtons.getWidth() / 2).reduced(3));
    loopOutButton.setBounds(midRowButtons.reduced(3));
    
    // Bottom row: PITCH + and PITCH -
    pitchUpButton.setBounds(bottomRowButtons.removeFromLeft(bottomRowButtons.getWidth() / 2).reduced(3));
    pitchDownButton.setBounds(bottomRowButtons.reduced(3));
    
    // Right section: sliders and EQ at top, hot cues and reset at bottom
    auto rightSection = mainArea.reduced(10);
    
    // Zoom controls and position slider area
    auto positionControlArea = rightSection.removeFromTop(65);
    
    // Zoom label and buttons at the top - shift to the right
    auto zoomArea = positionControlArea.removeFromTop(20);
    zoomArea.removeFromLeft(120); // Add space on the left to shift controls right
    zoomLabel.setBounds(zoomArea.removeFromLeft(50));
    zoomOutButton.setBounds(zoomArea.removeFromLeft(60).reduced(2));
    zoomInButton.setBounds(zoomArea.removeFromLeft(60).reduced(2));
    
    // 10s skip buttons on the right of zoom controls
    backward10sButton.setBounds(zoomArea.removeFromLeft(70).reduced(2));
    forward10sButton.setBounds(zoomArea.removeFromLeft(70).reduced(2));
    
    positionControlArea.removeFromTop(3); // Small gap
    
    // Position slider below zoom controls
    auto posArea = positionControlArea.removeFromTop(40).reduced(5);
    posSlider.setBounds(posArea);
    
    rightSection.removeFromTop(5); // Minimal gap
    
    auto sliderArea = rightSection.removeFromTop(150);
    auto volArea = sliderArea.removeFromLeft(sliderArea.getWidth() / 3).reduced(2);
    auto toneArea = sliderArea.removeFromLeft(sliderArea.getWidth() / 2).reduced(2);
    auto speedArea = sliderArea.reduced(2);
    
    auto volLabelArea = volArea.removeFromBottom(18);
    volLabel.setBounds(volLabelArea);
    volSlider.setBounds(volArea);
    
    auto toneLabelArea = toneArea.removeFromBottom(18);
    toneLabel.setBounds(toneLabelArea);
    toneSlider.setBounds(toneArea);
    
    auto speedLabelArea = speedArea.removeFromBottom(18);
    speedLabel.setBounds(speedLabelArea);
    speedSlider.setBounds(speedArea.reduced(0, 5));
    
    // R4: EQ knobs BELOW volume/speed sliders (on right side)
    rightSection.removeFromTop(8); // Reduced gap
    auto eqArea = rightSection.removeFromTop(75); // Reduced from 90
    
    // Divide into 3 columns for LOW, MID, HIGH
    auto lowEQArea = eqArea.removeFromLeft(eqArea.getWidth() / 3).reduced(3);
    auto midEQArea = eqArea.removeFromLeft(eqArea.getWidth() / 2).reduced(3);
    auto highEQArea = eqArea.reduced(3);
    
    // Low EQ
    lowEQLabel.setBounds(lowEQArea.removeFromTop(12)); // Reduced label height
    lowEQSlider.setBounds(lowEQArea);
    
    // Mid EQ
    midEQLabel.setBounds(midEQArea.removeFromTop(12)); // Reduced label height
    midEQSlider.setBounds(midEQArea);
    
    // High EQ
    highEQLabel.setBounds(highEQArea.removeFromTop(12)); // Reduced label height
    highEQSlider.setBounds(highEQArea);
    
    // R3: Hot cue buttons at BOTTOM RIGHT of deck (2 rows of 4)
    rightSection.removeFromTop(8); // Reduced gap
    auto hotCueLabelArea = rightSection.removeFromTop(15); // Reduced from 18
    hotCueLabel.setBounds(hotCueLabelArea);
    
    auto hotCueArea = rightSection.removeFromTop(52); // Reduced from 60
    auto hotCueTopRow = hotCueArea.removeFromTop(28);
    auto hotCueBottomRow = hotCueArea;
    
    // Top row: Buttons 1-4
    for (int i = 0; i < 4; ++i)
    {
        hotCueButtons[i].setBounds(hotCueTopRow.removeFromLeft(hotCueTopRow.getWidth() / (4 - i)).reduced(2));
    }
    
    // Bottom row: Buttons 5-8
    for (int i = 4; i < 8; ++i)
    {
        hotCueButtons[i].setBounds(hotCueBottomRow.removeFromLeft(hotCueBottomRow.getWidth() / (8 - i)).reduced(2));
    }
    
    // Reset buttons at VERY BOTTOM RIGHT - two rows with LARGER sizes
    rightSection.removeFromTop(5); // Small gap
    
    // Bottom row: Loop Off, Reset Cues, and Reset Equalisers (3 buttons) - MUCH LARGER
    auto bottomButtonsArea = rightSection.removeFromBottom(35); // Increased from 28 to 35
    int buttonWidth = bottomButtonsArea.getWidth() / 3;
    loopOffButton.setBounds(bottomButtonsArea.removeFromLeft(buttonWidth).reduced(2));
    resetCuesButton.setBounds(bottomButtonsArea.removeFromLeft(buttonWidth).reduced(2));
    resetEQButtonBottom.setBounds(bottomButtonsArea.reduced(2));
    
    rightSection.removeFromTop(5); // Small gap
    
    // Top row: Reset Speed button (keep existing)
    auto resetSpeedArea = rightSection.removeFromBottom(35); // Increased from 28 to 35
    resetSpeedButton.setBounds(resetSpeedArea.reduced(2));
}

//==============================================================================
void DeckGUI::buttonClicked(juce::Button* button)
{
    juce::Logger::writeToLog("Button clicked: " + button->getButtonText());
    
    if (button == &playButton)
    {
        handlePlayButton();
    }
    else if (button == &stopButton)
    {
        handleStopButton();
    }
    else if (button == &loopInButton)
    {
        handleLoopInButton();
    }
    else if (button == &loopOutButton)
    {
        handleLoopOutButton();
    }
    else if (button == &pitchUpButton)
    {
        handlePitchUp();
    }
    else if (button == &pitchDownButton)
    {
        handlePitchDown();
    }
    else if (button == &resetSpeedButton)
    {
        handleResetSpeed();
    }
    else if (button == &resetEQButton || button == &resetEQButtonBottom)
    {
        handleResetEQ();
    }
    else if (button == &resetCuesButton)
    {
        clearAllCuePoints();
    }
    else if (button == &loopOffButton)
    {
        handleLoopOffButton();
    }
    else if (button == &zoomInButton)
    {
        handleZoomIn();
    }
    else if (button == &zoomOutButton)
    {
        handleZoomOut();
    }
    else if (button == &forward10sButton)
    {
        handleForward10s();
    }
    else if (button == &backward10sButton)
    {
        handleBackward10s();
    }
    else
    {
        // Check if it's a hot cue button
        for (int i = 0; i < 8; ++i)
        {
            if (button == &hotCueButtons[i])
            {
                handleHotCueButton(i);
                break;
            }
        }
    }
}

void DeckGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volSlider)
    {
        player->setGain(slider->getValue());
    }
    else if (slider == &toneSlider)
    {
        player->setMidEQ((slider->getValue() - 0.5) * 24.0);
    }
    else if (slider == &speedSlider)
    {
        player->setSpeed(slider->getValue());
    }
    else if (slider == &posSlider)
    {
        player->setPositionRelative(slider->getValue());
    }
    else if (slider == &lowEQSlider)
    {
        player->setLowEQ(slider->getValue());
        eqFeature->setLow(slider->getValue());
        saveEQSettings();
    }
    else if (slider == &midEQSlider)
    {
        player->setMidEQ(slider->getValue());
        eqFeature->setMid(slider->getValue());
        saveEQSettings();
    }
    else if (slider == &highEQSlider)
    {
        player->setHighEQ(slider->getValue());
        eqFeature->setHigh(slider->getValue());
        saveEQSettings();
    }
}

//==============================================================================
// R1A: Check if dragged files are audio files
bool DeckGUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    return true; // Accept file drags
}

// R1A: Handle dropped audio files
void DeckGUI::filesDropped(const juce::StringArray& files, int x, int y)
{
    if (files.size() > 0)
    {
        loadTrackWithInfo(juce::File{files[0]});
    }
}

void DeckGUI::timerCallback()
{
    // Update position slider without triggering the listener
    double pos = player->getPositionRelative();
    // Clamp to valid range to prevent assertion errors
    pos = juce::jlimit(0.0, 1.0, pos);
    posSlider.setValue(pos, juce::dontSendNotification);
    
    // Sync position slider thumb color with active cue
    if (activeCueIndex >= 0 && activeCueIndex < 8)
    {
        juce::Colour cueColor = getCueColour(activeCueIndex);
        posSlider.setColour(juce::Slider::thumbColourId, cueColor);
        posSlider.setColour(juce::Slider::trackColourId, cueColor.withAlpha(0.7f));
    }
    else
    {
        // Default cyan color
        posSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d9ff));
        posSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00d9ff).withAlpha(0.7f));
    }
    
    // Update jog wheel rotation when playing
    if (player->isPlaying())
    {
        // Rotate based on speed (faster = more rotation)
        jogWheelRotation += player->getSpeed() * 0.05f; // Adjust rotation speed
        if (jogWheelRotation > 360.0f)
            jogWheelRotation -= 360.0f;
    }
    
    // Repaint to update audio-reactive disc glow and rotation
    repaint();
}

void DeckGUI::loadTrackWithInfo(const juce::File& file)
{
    juce::Logger::writeToLog("=== loadTrackWithInfo called for: " + file.getFullPathName());
    
    if (!file.existsAsFile())
    {
        juce::Logger::writeToLog("ERROR: File does not exist!");
        trackInfoLabel.setText("Error: File not found", juce::dontSendNotification);
        return;
    }
    
    juce::Logger::writeToLog("File exists, checking format...");
    
    // Check if format is supported
    auto extension = file.getFileExtension().toLowerCase();
    if (extension != ".mp3" && extension != ".wav" && extension != ".aiff" && 
        extension != ".flac" && extension != ".ogg")
    {
        trackInfoLabel.setText("Only mp3/wav/aiff/flac/ogg supported", juce::dontSendNotification);
        trackInfoLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        juce::Logger::writeToLog("Unsupported format: " + extension);
        return;
    }
    
    // Try to load the file
    juce::Logger::writeToLog("Calling player->loadURL...");
    player->loadURL(juce::URL{file});
    
    // Store the loaded file
    currentLoadedFile = file;
    juce::Logger::writeToLog("File stored, updating state machine to Loaded");
    
    // Update state machine
    stateMachine.setState(DeckState::Loaded);
    juce::Logger::writeToLog("State machine set to Loaded. canPlay = " + juce::String(stateMachine.canPlay() ? "true" : "false"));
    
    // Display track info
    displayTrackInfo(file);
    
    // R3: Load hot cue points for this track
    loadCuePoints();
}

/**
 * displayTrackInfo - Show track name and duration
 * 
 * Purpose: Extract and display track metadata
 * Parameters: file - Audio file to analyze
 */
void DeckGUI::displayTrackInfo(const juce::File& file)
{
    // Get track duration
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        double lengthInSeconds = reader->lengthInSamples / reader->sampleRate;
        int minutes = static_cast<int>(lengthInSeconds / 60);
        int seconds = static_cast<int>(lengthInSeconds) % 60;
        
        juce::String duration = juce::String(minutes) + ":" + 
                               juce::String(seconds).paddedLeft('0', 2);
        
        juce::String trackInfo = "Loaded: " + file.getFileNameWithoutExtension() + 
                                " [" + duration + "]";
        trackInfoLabel.setText(trackInfo, juce::dontSendNotification);
        trackInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff1493)); // Dark neon pink
        
        delete reader;
    }
    else
    {
        trackInfoLabel.setText("Loaded: " + file.getFileNameWithoutExtension(), 
                              juce::dontSendNotification);
        trackInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff1493)); // Dark neon pink
    }
}

void DeckGUI::unloadTrack()
{
    // Stop playback
    player->stop();
    
    // Update state machine
    stateMachine.setState(DeckState::Empty);
    
    // Clear the current track
    currentLoadedFile = juce::File();
    
    // R3: Clear hot cue points
    for (int i = 0; i < 8; ++i)
    {
        currentCuePoints[i] = -1.0;
        hotCueFeatures[i]->setPosition(-1.0);
    }
    updateCueButtonColors();
    
    // Update the label
    trackInfoLabel.setText("No track loaded", juce::dontSendNotification);
    trackInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff1493));
    
    // Notify playlist component
    if (playlistComponent != nullptr)
    {
        if (deckNumber == 1)
            playlistComponent->setDeck1Playing(false);
        else
            playlistComponent->setDeck2Playing(false);
    }
    
    juce::Logger::writeToLog("Track unloaded from Deck " + juce::String(deckNumber));
}

void DeckGUI::setPlaylistComponent(PlaylistComponent* playlist)
{
    playlistComponent = playlist;
}

//==============================================================================
// Draw circular vinyl-style jog wheel with audio-reactive glow
void DeckGUI::drawJogWheel(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto centre = bounds.getCentre().toFloat();
    float radius = bounds.getWidth() / 2.0f - 5;
    float innerRadius = radius * 0.7f;
    
    // NOTE: Disc does NOT rotate - only the needle rotates based on track position
    
    // Outer metallic ring with gradient (darker/sleeker)
    {
        juce::ColourGradient ringGradient(
            juce::Colour(0xff505050), centre.x, centre.y - radius,
            juce::Colour(0xff202020), centre.x, centre.y + radius, false);
        g.setGradientFill(ringGradient);
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2);
    }
    
    // Black vinyl surface (STATIC - does not rotate)
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, 
                  innerRadius * 2, innerRadius * 2);
    
    // Vinyl grooves effect (STATIC - does not rotate)
    g.setColour(juce::Colour(0xff1a1a1a));
    for (float r = innerRadius * 0.3f; r < innerRadius; r += 6.0f)
    {
        g.drawEllipse(centre.x - r, centre.y - r, r * 2, r * 2, 0.5f);
    }
    
    // Center label area with DJ Music text (STATIC - does not rotate)
    float labelRadius = innerRadius * 0.45f;
    juce::ColourGradient labelGradient(
        juce::Colour(0xff2a2a2a), centre.x, centre.y - labelRadius,
        juce::Colour(0xff1a1a1a), centre.x, centre.y + labelRadius, false);
    g.setGradientFill(labelGradient);
    g.fillEllipse(centre.x - labelRadius, centre.y - labelRadius, 
                  labelRadius * 2, labelRadius * 2);
    
    // DJ Music text in center (STATIC - does not rotate)
    g.setColour(juce::Colour(0xffc49a1e));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("DJ", bounds.withSizeKeepingCentre(80, 30).translated(0, -10), 
               juce::Justification::centred, true);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("MUSIC", bounds.withSizeKeepingCentre(80, 30).translated(0, 5), 
               juce::Justification::centred, true);
    
    // Draw needle that rotates based on track position (0 = top, 1 = full circle)
    double trackPosition = player->getPositionRelative(); // 0.0 to 1.0
    float needleAngle = -juce::MathConstants<float>::halfPi + (trackPosition * juce::MathConstants<float>::twoPi);
    float markerLength = radius * 0.85f;
    float markerX = centre.x + std::cos(needleAngle) * markerLength;
    float markerY = centre.y + std::sin(needleAngle) * markerLength;
    
    // Draw rotating needle line
    g.setColour(juce::Colour(0xffff1493).withAlpha(0.8f));
    g.drawLine(centre.x, centre.y, markerX, markerY, 3.0f);
    
    // Draw dot at end of needle
    g.fillEllipse(markerX - 4, markerY - 4, 8, 8);
    
    // Blue-only audio-reactive glow - intensity pulses with beat (NOT rotated)
    float audioLevel = player->getAudioLevel();
    
    // Boost audio level for better visual effect (amplify quiet signals)
    audioLevel = std::min(1.0f, audioLevel * 5.0f);
    
    // Beat intensity driven by actual audio amplitude (slow beats = dim, fast beats = bright)
    float beatIntensity = audioLevel * 0.8f + 0.2f; // Scale between 0.2 (dim) and 1.0 (bright)
    
    // Blue color only - stays same, only intensity changes
    juce::Colour blueGlow = juce::Colour(0xff00aaff); // Bright blue
    
    // Draw multiple concentric glow rings that pulse with beat (thicker when louder)
    for (int i = 5; i >= 0; --i)
    {
        float glowRadius = radius + (i * 5.0f) * beatIntensity; // Gets thicker with louder audio
        float alpha = (0.7f - i * 0.1f) * beatIntensity;
        g.setColour(blueGlow.withAlpha(alpha));
        g.drawEllipse(centre.x - glowRadius, centre.y - glowRadius, 
                      glowRadius * 2, glowRadius * 2, 3.0f + (beatIntensity * 2.0f)); // Thicker lines when loud
    }
    
    // Main outer glow ring - gets brighter and thicker with beat
    g.setColour(blueGlow.withAlpha(0.9f * beatIntensity));
    g.drawEllipse(centre.x - radius, centre.y - radius, 
                  radius * 2, radius * 2, 3.0f + (beatIntensity * 3.0f));
}

// Draw Downbeat Detector (replaces VU meter)
void DeckGUI::drawDownbeatDetector(juce::Graphics& g, juce::Rectangle<int> bounds, float level)
{
    // Clamp level
    level = juce::jlimit(0.0f, 1.0f, level);
    
    // Background
    g.setColour(juce::Colour(0xff1a1f35));
    g.fillRoundedRectangle(bounds.toFloat(), 3.0f);
    
    // Border - RGB neon (cycling color)
    auto borderCurrentTime = juce::Time::getMillisecondCounterHiRes();
    float borderHue = fmodf((borderCurrentTime / 1000.0f) * 0.1f, 1.0f); // Slow color cycle
    g.setColour(juce::Colour::fromHSV(borderHue, 0.9f, 1.0f, 0.5f));
    g.drawRoundedRectangle(bounds.toFloat(), 3.0f, 1.5f);
    
    // === IMPROVED DOWNBEAT DETECTION ALGORITHM ===
    
    // Get current time
    double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    
    // 1. Short-time energy window (detect rapid increases in energy)
    float energyDelta = level - previousAudioLevel;
    previousAudioLevel = level;
    
    // 2. Threshold-based peak detection
    const float ENERGY_THRESHOLD = 0.15f; // Detect significant energy increases
    const float LEVEL_THRESHOLD = 0.5f;   // Minimum level to consider as beat
    bool isPeakDetected = (energyDelta > ENERGY_THRESHOLD && level > LEVEL_THRESHOLD);
    
    // 3. Cooldown period (avoid double triggering)
    const double COOLDOWN_PERIOD = 0.15; // 150ms cooldown
    bool isInCooldown = (currentTime < cooldownEndTime);
    
    bool isBeat = false;
    bool isDownbeat = false;
    
    if (isPeakDetected && !isInCooldown)
    {
        isBeat = true;
        
        // 4. Calculate average beat interval
        double beatInterval = currentTime - lastBeatTime;
        
        // Only track reasonable beat intervals (40-240 BPM range)
        if (beatInterval > 0.25 && beatInterval < 1.5)
        {
            recentBeatIntervals.push_back(beatInterval);
            
            // Keep only last 8 intervals for averaging
            if (recentBeatIntervals.size() > 8)
                recentBeatIntervals.erase(recentBeatIntervals.begin());
            
            // Calculate average
            double sum = 0.0;
            for (auto interval : recentBeatIntervals)
                sum += interval;
            averageBeatInterval = sum / recentBeatIntervals.size();
        }
        
        lastBeatTime = currentTime;
        cooldownEndTime = currentTime + COOLDOWN_PERIOD;
        
        // 5. Mark first beat of 4-beat cycle as downbeat
        beatCounter = (beatCounter + 1) % 4;
        isDownbeat = (beatCounter == 0);
    }
    
    // Calculate fill height
    float fillHeight = bounds.getHeight() * level;
    auto fillBounds = bounds.removeFromBottom(static_cast<int>(fillHeight)).toFloat();
    
    // RGB gradient fill with beat detection
    // Create multi-point RGB gradient (Red -> Green -> Blue from bottom to top)
    juce::ColourGradient rgbGradient(
        juce::Colours::red,      // Bottom: Red
        fillBounds.getCentreX(), fillBounds.getBottom(),
        juce::Colours::blue,     // Top: Blue
        fillBounds.getCentreX(), fillBounds.getY(), 
        false);
    
    // Add green in the middle
    rgbGradient.addColour(0.5, juce::Colours::lime); // Middle: Green
    
    // Brighten colors on beat detection
    if (isBeat && isDownbeat)
    {
        // Brighten all colors for downbeat
        rgbGradient.multiplyOpacity(1.5f);
    }
    else if (isBeat)
    {
        // Slight brighten for regular beats
        rgbGradient.multiplyOpacity(1.2f);
    }
    
    g.setGradientFill(rgbGradient);
    g.fillRoundedRectangle(fillBounds, 5.0f);
    
    // Pulse effect - BIGGER for downbeat, smaller for normal beats
    if (isBeat)
    {
        if (isDownbeat)
        {
            // Larger pulse for downbeat (first beat of 4)
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.fillEllipse(bounds.getCentreX() - 12, bounds.getY() + 8, 24, 24);
            
            // Additional glow ring
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.drawEllipse(bounds.getCentreX() - 15, bounds.getY() + 5, 30, 30, 2.0f);
        }
        else
        {
            // Smaller pulse for normal beats (beats 2, 3, 4)
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.fillEllipse(bounds.getCentreX() - 6, bounds.getY() + 12, 12, 12);
        }
    }
    
    // LED segments effect
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    for (int i = 0; i < 10; ++i)
    {
        float y = bounds.getY() + (i * bounds.getHeight() / 10.0f);
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 2.0f);
    }
}

// R5: Paint visual cue markers on position bar
void DeckGUI::paintCueMarkers(juce::Graphics& g)
{
    // Get position slider bounds
    auto sliderBounds = posSlider.getBounds();
    
    // Draw cue markers for each set hot cue
    for (int i = 0; i < 8; ++i)
    {
        if (currentCuePoints[i] >= 0.0 && currentCuePoints[i] <= 1.0)
        {
            // Calculate x position based on cue point position (0.0 to 1.0)
            float cuePosition = static_cast<float>(currentCuePoints[i]);
            int xPos = sliderBounds.getX() + static_cast<int>(cuePosition * sliderBounds.getWidth());
            
            // Draw small colored tick mark
            int tickHeight = 8;
            int tickWidth = 3;
            
            // Draw tick above the slider
            auto tickBounds = juce::Rectangle<int>(xPos - tickWidth / 2, 
                                                   sliderBounds.getY() - tickHeight - 2,
                                                   tickWidth, 
                                                   tickHeight);
            
            // Get color for this cue
            juce::Colour cueColor = getCueColour(i);
            
            // Draw with glow effect
            g.setColour(cueColor.withAlpha(0.3f));
            g.fillRect(tickBounds.expanded(2));
            
            // Draw solid tick
            g.setColour(cueColor);
            g.fillRect(tickBounds);
            
            // Draw tiny triangle pointer below tick
            juce::Path triangle;
            float tipX = static_cast<float>(xPos);
            float tipY = static_cast<float>(sliderBounds.getY() - 2);
            triangle.addTriangle(tipX, tipY, 
                               tipX - 3, tipY - 4, 
                               tipX + 3, tipY - 4);
            g.fillPath(triangle);
        }
    }
}

//==============================================================================
// R3: Hot Cue Functions

void DeckGUI::setCuePoint(int cueIndex)
{
    if (cueIndex < 0 || cueIndex >= 8)
        return;
    
    // Store current playback position
    double position = player->getPositionRelative();
    currentCuePoints[cueIndex] = position;
    
    // Update polymorphic feature
    hotCueFeatures[cueIndex]->setPosition(position);
    
    // Update button color to indicate cue is set
    updateCueButtonColors();
    
    // Save cue points for this track
    saveCuePoints();
    
    juce::Logger::writeToLog("Hot cue " + juce::String(cueIndex + 1) + " set at " + 
                             juce::String(currentCuePoints[cueIndex], 3));
}

void DeckGUI::jumpToCuePoint(int cueIndex)
{
    if (cueIndex < 0 || cueIndex >= 8)
        return;
    
    if (currentCuePoints[cueIndex] >= 0.0)
    {
        // Use polymorphic feature to apply cue
        hotCueFeatures[cueIndex]->apply(player);
        juce::Logger::writeToLog("Jumped to hot cue " + juce::String(cueIndex + 1));
    }
}

void DeckGUI::clearCuePoint(int cueIndex)
{
    if (cueIndex < 0 || cueIndex >= 8)
        return;
    
    
    // Reset polymorphic feature
    hotCueFeatures[cueIndex]->reset();
    currentCuePoints[cueIndex] = -1.0; // Mark as not set
    
    // Update button color
    updateCueButtonColors();
    
    // Save updated cue points
    saveCuePoints();
    
    juce::Logger::writeToLog("Hot cue " + juce::String(cueIndex + 1) + " cleared");
}

void DeckGUI::clearAllCuePoints()
{
    // R3C: Clear all 8 hot cue points
    for (int i = 0; i < 8; ++i)
    {
        currentCuePoints[i] = -1.0;
        hotCueFeatures[i]->reset();
    }
    
    // Update button colors
    updateCueButtonColors();
    
    // Save updated cue points
    saveCuePoints();
    
    juce::Logger::writeToLog("All hot cues cleared");
}

void DeckGUI::updateCueButtonColors()
{
    for (int i = 0; i < 8; ++i)
    {
        if (currentCuePoints[i] >= 0.0)
        {
            // Cue is set - use color from palette
            hotCueButtons[i].setColour(juce::TextButton::buttonColourId, getCueColour(i));
        }
        else
        {
            // Cue is not set - use dark gray
            hotCueButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
        }
    }
}

void DeckGUI::saveCuePoints()
{
    if (currentLoadedFile == juce::File())
        return; // No track loaded
    
    // Store cue points for this track
    juce::String trackPath = currentLoadedFile.getFullPathName();
    cuePointsMap[trackPath] = currentCuePoints;
    
    // Save to file
    juce::File cueFile = getCueFilePath();
    cueFile.getParentDirectory().createDirectory();
    
    std::ofstream outFile(cueFile.getFullPathName().toStdString());
    if (outFile.is_open())
    {
        // Write all cue points for all tracks
        outFile << cuePointsMap.size() << "\\n";
        
        for (const auto& entry : cuePointsMap)
        {
            outFile << entry.first.toStdString() << "\\n";
            for (int i = 0; i < 8; ++i)
            {
                outFile << entry.second[i] << " ";
            }
            outFile << "\\n";
        }
        
        outFile.close();
        juce::Logger::writeToLog("Cue points saved for deck " + juce::String(deckNumber));
    }
}

void DeckGUI::loadCuePoints()
{
    if (currentLoadedFile == juce::File())
        return; // No track loaded
    
    juce::File cueFile = getCueFilePath();
    if (!cueFile.existsAsFile())
    {
        // No saved cue points - initialize to empty
        for (int i = 0; i < 8; ++i)
        {
            currentCuePoints[i] = -1.0;
        }
        updateCueButtonColors();
        return;
    }
    
    // Load all cue points from file
    std::ifstream inFile(cueFile.getFullPathName().toStdString());
    if (inFile.is_open())
    {
        int numTracks = 0;
        inFile >> numTracks;
        inFile.ignore();
        
        cuePointsMap.clear();
        
        for (int t = 0; t < numTracks; ++t)
        {
            std::string trackPath;
            std::getline(inFile, trackPath);
            
            std::array<double, 8> cues;
            for (int i = 0; i < 8; ++i)
            {
                inFile >> cues[i];
            }
            inFile.ignore();
            
            cuePointsMap[juce::String(trackPath)] = cues;
        }
        
        inFile.close();
        
        // Load cue points for current track
        juce::String trackPath = currentLoadedFile.getFullPathName();
        if (cuePointsMap.find(trackPath) != cuePointsMap.end())
        {
            currentCuePoints = cuePointsMap[trackPath];
            juce::Logger::writeToLog("Loaded cue points for track");
        }
        else
        {
            // No cue points for this track
            for (int i = 0; i < 8; ++i)
            {
                currentCuePoints[i] = -1.0;
            }
        }
        
        updateCueButtonColors();
    }
}

juce::File DeckGUI::getCueFilePath()
{
    juce::File appDataDir = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    
    return appDataDir.getChildFile("Otodecks")
                     .getChildFile("cue_points_deck" + juce::String(deckNumber) + ".dat");
}

//==============================================================================
// R4: EQ Settings Persistence

void DeckGUI::saveEQSettings()
{
    juce::File eqFile = getEQFilePath();
    eqFile.getParentDirectory().createDirectory();
    
    std::ofstream outFile(eqFile.getFullPathName().toStdString());
    if (outFile.is_open())
    {
        outFile << player->getLowEQ() << "\\n";
        outFile << player->getMidEQ() << "\\n";
        outFile << player->getHighEQ() << "\\n";
        outFile.close();
        
        juce::Logger::writeToLog("EQ settings saved for deck " + juce::String(deckNumber));
    }
}

void DeckGUI::loadEQSettings()
{
    juce::File eqFile = getEQFilePath();
    if (!eqFile.existsAsFile())
    {
        // No saved settings - use defaults (flat EQ)
        return;
    }
    
    std::ifstream inFile(eqFile.getFullPathName().toStdString());
    if (inFile.is_open())
    {
        double lowEQ, midEQ, highEQ;
        inFile >> lowEQ >> midEQ >> highEQ;
        inFile.close();
        
        // Apply loaded settings
        lowEQSlider.setValue(lowEQ, juce::dontSendNotification);
        midEQSlider.setValue(midEQ, juce::dontSendNotification);
        highEQSlider.setValue(highEQ, juce::dontSendNotification);
        
        player->setLowEQ(lowEQ);
        player->setMidEQ(midEQ);
        player->setHighEQ(highEQ);
        
        juce::Logger::writeToLog("EQ settings loaded for deck " + juce::String(deckNumber));
    }
}

juce::File DeckGUI::getEQFilePath()
{
    juce::File appDataDir = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    
    return appDataDir.getChildFile("Otodecks")
                     .getChildFile("eq_settings_deck" + juce::String(deckNumber) + ".dat");
}

//==============================================================================
// Split button handler functions (C4: Functions should do ONE job)

void DeckGUI::handlePlayButton()
{
    juce::Logger::writeToLog("=== Play button clicked - Deck " + juce::String(deckNumber));
    juce::Logger::writeToLog("Current state: " + juce::String((int)stateMachine.getState()));
    juce::Logger::writeToLog("canPlay: " + juce::String(stateMachine.canPlay() ? "true" : "false"));
    juce::Logger::writeToLog("Current volume/gain: " + juce::String(volSlider.getValue()));
    juce::Logger::writeToLog("Transport length: " + juce::String(player->getPositionRelative()));
    
    if (!stateMachine.canPlay())
    {
        juce::Logger::writeToLog("ERROR: Cannot play - state is Empty (no track loaded)");
        trackInfoLabel.setText("Load a track first!", juce::dontSendNotification);
        trackInfoLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        return;
    }
    
    // Ensure volume is set correctly before playing
    double currentVolume = volSlider.getValue();
    player->setGain(currentVolume);
    juce::Logger::writeToLog("Volume set to: " + juce::String(currentVolume));
    
    juce::Logger::writeToLog("Calling player->start()...");
    player->start();
    juce::Logger::writeToLog("player->start() called successfully");
    
    // Update state machine
    if (player->isLooping())
        stateMachine.setState(DeckState::Looping);
    else
        stateMachine.setState(DeckState::Playing);
    
    // Notify playlist component
    if (playlistComponent != nullptr)
    {
        if (deckNumber == 1)
            playlistComponent->setDeck1Playing(true);
        else
            playlistComponent->setDeck2Playing(true);
    }
}

void DeckGUI::handleStopButton()
{
    player->stop();
    
    // Update state machine
    if (!stateMachine.isEmpty())
        stateMachine.setState(DeckState::Stopped);
    
    // Notify playlist component
    if (playlistComponent != nullptr)
    {
        if (deckNumber == 1)
            playlistComponent->setDeck1Playing(false);
        else
            playlistComponent->setDeck2Playing(false);
    }
}

void DeckGUI::handleLoopInButton()
{
    double currentPos = player->getPositionRelative();
    loopInPosition = currentPos;
    
    // Increment counter and update button text
    loopInCounter++;
    loopInButton.setButtonText("LOOP IN x" + juce::String(loopInCounter));
    
    // If we have both in and out points, enable looping
    if (loopInPosition >= 0 && loopOutPosition >= 0 && loopInPosition < loopOutPosition)
    {
        player->setLooping(true, loopInPosition, loopOutPosition);
        loopInButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00ff41));
        
        // Update state if playing
        if (player->isPlaying())
            stateMachine.setState(DeckState::Looping);
        
        juce::Logger::writeToLog("Loop IN set at: " + juce::String(loopInPosition, 3));
    }
    else
    {
        loopInButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffffa500));
    }
}

void DeckGUI::handleLoopOutButton()
{
    double currentPos = player->getPositionRelative();
    loopOutPosition = currentPos;
    
    // Increment counter and update button text
    loopOutCounter++;
    loopOutButton.setButtonText("LOOP OUT x" + juce::String(loopOutCounter));
    
    // If we have both in and out points, enable looping
    if (loopInPosition >= 0 && loopOutPosition >= 0 && loopInPosition < loopOutPosition)
    {
        player->setLooping(true, loopInPosition, loopOutPosition);
        loopOutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00ff41));
        
        // Update state if playing
        if (player->isPlaying())
            stateMachine.setState(DeckState::Looping);
        
        juce::Logger::writeToLog("Loop OUT set - Looping enabled");
    }
    else
    {
        loopOutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffffa500));
    }
}

void DeckGUI::handleLoopOffButton()
{
    juce::Logger::writeToLog("=== Loop Off button clicked - Deck " + juce::String(deckNumber));
    
    player->setLooping(false, 0.0, 1.0);
    
    // Reset loop positions
    loopInPosition = -1.0;
    loopOutPosition = -1.0;
    
    // Reset loop counters and button text
    loopInCounter = 0;
    loopOutCounter = 0;
    loopInButton.setButtonText("LOOP IN");
    loopOutButton.setButtonText("LOOP OUT");
    
    // Reset button colors
    auto specialPink = juce::Colour(0xffff69b4);
    loopInButton.setColour(juce::TextButton::buttonColourId, specialPink);
    loopOutButton.setColour(juce::TextButton::buttonColourId, specialPink);
    
    juce::Logger::writeToLog("Loop disabled - positions reset");
    
    // Update state if playing
    if (player->isPlaying())
        stateMachine.setState(DeckState::Playing);
    
    juce::Logger::writeToLog("Loop disabled");
}

void DeckGUI::handlePitchUp()
{
    double currentSpeed = speedSlider.getValue();
    speedSlider.setValue(juce::jmin(2.0, currentSpeed + 0.05));
}

void DeckGUI::handlePitchDown()
{
    double currentSpeed = speedSlider.getValue();
    speedSlider.setValue(juce::jmax(0.5, currentSpeed - 0.05));
}

void DeckGUI::handleResetSpeed()
{
    speedSlider.setValue(1.0);
    player->setSpeed(1.0);
}

void DeckGUI::handleResetEQ()
{
    lowEQSlider.setValue(0.0);
    midEQSlider.setValue(0.0);
    highEQSlider.setValue(0.0);
    
    player->setLowEQ(0.0);
    player->setMidEQ(0.0);
    player->setHighEQ(0.0);
    
    eqFeature->reset();
}

void DeckGUI::handleHotCueButton(int cueIndex)
{
    // If Shift/Ctrl is held, clear the cue point
    if (juce::ModifierKeys::getCurrentModifiers().isCommandDown() ||
        juce::ModifierKeys::getCurrentModifiers().isShiftDown())
    {
        clearCuePoint(cueIndex);
    }
    else
    {
        // If cue point is set, jump to it; otherwise, set it
        if (currentCuePoints[cueIndex] >= 0.0)
        {
            jumpToCuePoint(cueIndex);
            // Set this as the active cue for color synchronization
            activeCueIndex = cueIndex;
        }
        else
        {
            setCuePoint(cueIndex);
        }
    }
}

void DeckGUI::handleForward10s()
{
    juce::Logger::writeToLog("=== SKIP FORWARD 10s - Deck " + juce::String(deckNumber));
    double oldPos = player->getPositionRelative();
    juce::Logger::writeToLog("Position before skip: " + juce::String(oldPos * 100.0) + "%");
    
    player->skipForward(10.0);
    
    double newPos = player->getPositionRelative();
    juce::Logger::writeToLog("Position after skip: " + juce::String(newPos * 100.0) + "%");
    juce::Logger::writeToLog("Skipped forward 10 seconds");
    
    // Force position slider update
    posSlider.setValue(newPos, juce::dontSendNotification);
}

void DeckGUI::handleBackward10s()
{
    juce::Logger::writeToLog("=== SKIP BACKWARD 10s - Deck " + juce::String(deckNumber));
    double oldPos = player->getPositionRelative();
    juce::Logger::writeToLog("Position before skip: " + juce::String(oldPos * 100.0) + "%");
    
    player->skipBackward(10.0);
    
    double newPos = player->getPositionRelative();
    juce::Logger::writeToLog("Position after skip: " + juce::String(newPos * 100.0) + "%");
    juce::Logger::writeToLog("Skipped backward 10 seconds");
    
    // Force position slider update
    posSlider.setValue(newPos, juce::dontSendNotification);
}

void DeckGUI::handleZoomIn()
{
    juce::Logger::writeToLog("=== ZOOM IN clicked - Deck " + juce::String(deckNumber));
    juce::Logger::writeToLog("Current zoom level: " + juce::String(waveformZoomLevel));
    
    if (waveformZoomLevel < 3)
    {
        waveformZoomLevel++;
        if (waveformZoomLevel == 1)
            waveformTimeWindow = 30.0;
        else if (waveformZoomLevel == 2)
            waveformTimeWindow = 15.0;
        else if (waveformZoomLevel == 3)
            waveformTimeWindow = 5.0;
        
        juce::Logger::writeToLog("NEW Zoom level: " + juce::String(waveformZoomLevel) + 
                                " (" + juce::String(waveformTimeWindow) + " seconds)");
        
        // Update zoom label to show current zoom level with color feedback
        juce::Colour zoomColor;
        if (waveformZoomLevel == 1)
            zoomColor = juce::Colour(0xff00d4ff); // Cyan - Wide view
        else if (waveformZoomLevel == 2)
            zoomColor = juce::Colour(0xff00ff41); // Green - Medium view
        else if (waveformZoomLevel == 3)
            zoomColor = juce::Colour(0xffff6600); // Orange - Close view
        
        zoomLabel.setText("ZOOM " + juce::String(waveformZoomLevel) + "x", juce::dontSendNotification);
        zoomLabel.setColour(juce::Label::textColourId, zoomColor);
        
        // Update waveform display in playlist
        if (playlistComponent != nullptr)
        {
            juce::Logger::writeToLog("Updating playlist waveform zoom for deck " + juce::String(deckNumber));
            if (deckNumber == 1)
                playlistComponent->setDeck1Zoom(waveformZoomLevel);
            else
                playlistComponent->setDeck2Zoom(waveformZoomLevel);
        }
        else
        {
            juce::Logger::writeToLog("WARNING: playlistComponent is nullptr!");
        }
        
        repaint();
    }
    else
    {
        juce::Logger::writeToLog("Already at maximum zoom level (3x)");
    }
}

void DeckGUI::handleZoomOut()
{
    juce::Logger::writeToLog("=== ZOOM OUT clicked - Deck " + juce::String(deckNumber));
    juce::Logger::writeToLog("Current zoom level: " + juce::String(waveformZoomLevel));
    
    if (waveformZoomLevel > 1)
    {
        waveformZoomLevel--;
        if (waveformZoomLevel == 1)
            waveformTimeWindow = 30.0;
        else if (waveformZoomLevel == 2)
            waveformTimeWindow = 15.0;
        else if (waveformZoomLevel == 3)
            waveformTimeWindow = 5.0;
        
        juce::Logger::writeToLog("NEW Zoom level: " + juce::String(waveformZoomLevel) + 
                                " (" + juce::String(waveformTimeWindow) + " seconds)");
        
        // Update zoom label to show current zoom level with color feedback
        juce::Colour zoomColor;
        if (waveformZoomLevel == 1)
            zoomColor = juce::Colour(0xff00d4ff); // Cyan - Wide view
        else if (waveformZoomLevel == 2)
            zoomColor = juce::Colour(0xff00ff41); // Green - Medium view  
        else if (waveformZoomLevel == 3)
            zoomColor = juce::Colour(0xffff6600); // Orange - Close view
        
        zoomLabel.setText("ZOOM " + juce::String(waveformZoomLevel) + "x", juce::dontSendNotification);
        zoomLabel.setColour(juce::Label::textColourId, zoomColor);
        
        // Update waveform display in playlist
        if (playlistComponent != nullptr)
        {
            juce::Logger::writeToLog("Updating playlist waveform zoom for deck " + juce::String(deckNumber));
            if (deckNumber == 1)
                playlistComponent->setDeck1Zoom(waveformZoomLevel);
            else
                playlistComponent->setDeck2Zoom(waveformZoomLevel);
        }
        else
        {
            juce::Logger::writeToLog("WARNING: playlistComponent is nullptr!");
        }
        
        repaint();
    }
    else
    {
        juce::Logger::writeToLog("Already at minimum zoom level (1x)");
    }
}

bool DeckGUI::keyPressed(const juce::KeyPress& key)
{
    // Space bar - Play/Pause
    if (key == juce::KeyPress::spaceKey)
    {
        if (!player->isPlaying())
            handlePlayButton();
        else
            handleStopButton();
        return true;
    }
    // Left arrow - Jump back 10 seconds
    else if (key == juce::KeyPress::leftKey)
    {
        handleBackward10s();
        return true;
    }
    // Right arrow - Jump forward 10 seconds
    else if (key == juce::KeyPress::rightKey)
    {
        handleForward10s();
        return true;
    }
    // Number keys 1-8 for hot cues
    else if (key.getTextCharacter() >= '1' && key.getTextCharacter() <= '8')
    {
        int cueIndex = key.getTextCharacter() - '1';
        handleHotCueButton(cueIndex);
        return true;
    }
    
    return false;
}

//==============================================================================
// C4 & DRY: Helper function to configure EQ sliders with consistent settings
// Purpose: Reduce code duplication in constructor (follows DRY principle)
void DeckGUI::configureEQSlider(juce::Slider& slider, juce::Label& label,
                                const juce::String& labelText, const juce::Colour& colour)
{
    // Add to UI
    addAndMakeVisible(slider);
    addAndMakeVisible(label);
    
    // Configure slider properties
    slider.setRange(-24.0, 12.0);  // Professional DJ EQ range (-24dB to +12dB)
    slider.setValue(0.0);           // Flat EQ by default
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 45, 18);
    slider.setNumDecimalPlacesToDisplay(1);
    slider.addListener(this);
    
    // Apply color scheme
    slider.setColour(juce::Slider::thumbColourId, colour);
    slider.setColour(juce::Slider::rotarySliderFillColourId, colour);
    slider.setLookAndFeel(&glossyLookAndFeel);
    
    // Configure label
    label.setText(labelText, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, colour);
    label.setFont(juce::Font(10.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
}

juce::Colour DeckGUI::getCueColour(int cueIndex)
{
    // Color palette for 8 different cue points
    const juce::Colour cueColors[8] = {
        juce::Colours::red,          // Cue 1
        juce::Colours::orange,       // Cue 2
        juce::Colours::yellow,       // Cue 3
        juce::Colours::green,        // Cue 4
        juce::Colours::cyan,         // Cue 5
        juce::Colours::blue,         // Cue 6
        juce::Colours::purple,       // Cue 7
        juce::Colours::hotpink       // Cue 8
    };
    
    if (cueIndex >= 0 && cueIndex < 8)
        return cueColors[cueIndex];
    
    return juce::Colours::grey;
}

