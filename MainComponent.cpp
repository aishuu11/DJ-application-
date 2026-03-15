#include "MainComponent.h"

//==============================================================================
// Constructor - Sets up the main application window with two decks
MainComponent::MainComponent()
{
    // Make sure audio is set up before the component is made visible
    setSize(800, 600);

    // Add and make visible all child components
    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    addAndMakeVisible(playlistComponent);
    
    // R1C: Configure crossfader (left = Deck1, right = Deck2)
    addAndMakeVisible(crossfader);
    addAndMakeVisible(crossfaderLabel);
    crossfader.setRange(0.0, 1.0);
    crossfader.setValue(0.5); // Center position (both decks equal)
    crossfader.setSliderStyle(juce::Slider::LinearHorizontal);
    crossfader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    crossfader.addListener(this);
    crossfader.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffa500));
    crossfader.setColour(juce::Slider::trackColourId, juce::Colour(0xff00d9ff));
    crossfader.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1f35));
    
    // Initialize crossfader multipliers (equal power at center position)
    double position = 0.5;
    crossfadeMultiplier1 = std::cos(position * juce::MathConstants<double>::halfPi);
    crossfadeMultiplier2 = std::sin(position * juce::MathConstants<double>::halfPi);
    crossfaderLabel.setText("CROSSFADER", juce::dontSendNotification);
    crossfaderLabel.setJustificationType(juce::Justification::centred);
    crossfaderLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    crossfaderLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    
    // Add SYNC button to match deck speeds
    addAndMakeVisible(syncButton);
    syncButton.setButtonText("SYNC");
    syncButton.addListener(this);
    syncButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff1493));
    syncButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    syncButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00ff41));
    syncButton.setTooltip("Sync Deck 2 speed to Deck 1");
    syncButton.setClickingTogglesState(true);

    // Register basic audio formats (WAV, AIFF, MP3, etc.)
    formatManager.registerBasicFormats();
    
    // Set playlist component reference in decks for status updates
    deckGUI1.setPlaylistComponent(&playlistComponent);
    deckGUI2.setPlaylistComponent(&playlistComponent);
    
    // Start timer for waveform animation (30fps)
    startTimer(33);

    // Some platforms require permissions to open input channels
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                          [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels
        setAudioChannels(0, 2);
    }
}

MainComponent::~MainComponent()
{
    // Shut down audio properly
    shutdownAudio();
}

//==============================================================================
// R1: Prepare audio system for playback
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Prepare both audio players
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

// R1B & R1C: Mix audio from both decks with crossfader and individual volumes
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Clear the buffer first
    bufferToFill.clearActiveBufferRegion();
    
    // Create temporary buffers for each player
    juce::AudioBuffer<float> tempBuffer1(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
    juce::AudioBuffer<float> tempBuffer2(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
    
    // Get audio from player 1
    juce::AudioSourceChannelInfo info1;
    info1.buffer = &tempBuffer1;
    info1.startSample = 0;
    info1.numSamples = bufferToFill.numSamples;
    tempBuffer1.clear();
    player1.getNextAudioBlock(info1);
    
    // Get audio from player 2
    juce::AudioSourceChannelInfo info2;
    info2.buffer = &tempBuffer2;
    info2.startSample = 0;
    info2.numSamples = bufferToFill.numSamples;
    tempBuffer2.clear();
    player2.getNextAudioBlock(info2);
    
    // Mix both players into output buffer with crossfader multipliers and master volume
    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        auto* outputData = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        const auto* input1Data = tempBuffer1.getReadPointer(channel);
        const auto* input2Data = tempBuffer2.getReadPointer(channel);
        
        for (int i = 0; i < bufferToFill.numSamples; ++i)
        {
            // Mix with crossfader multipliers (player volumes are already applied by their gain settings)
            float mixedSample = (input1Data[i] * static_cast<float>(crossfadeMultiplier1)) + 
                               (input2Data[i] * static_cast<float>(crossfadeMultiplier2));
            
            // Write mixed output
            outputData[i] = mixedSample;
        }
    }
}

void MainComponent::releaseResources()
{
    // Release resources from players
    player1.releaseResources();
    player2.releaseResources();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Modern dark gradient background
    juce::ColourGradient gradient(juce::Colour(0xff000000), getWidth() / 2.0f, 0,
                                  juce::Colour(0xff0f1419), getWidth() / 2.0f, 
                                  static_cast<float>(getHeight()), false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Add subtle grid pattern for pro look
    g.setColour(juce::Colour(0xff1a1f35).withAlpha(0.3f));
    for (int i = 0; i < getWidth(); i += 50)
    {
        g.drawLine(static_cast<float>(i), 0, static_cast<float>(i), 
                   static_cast<float>(getHeight()), 0.5f);
    }
    
    // Draw waveform visualization at top center
    auto waveformBounds = juce::Rectangle<int>(getWidth() / 4, 10, 
                                                getWidth() / 2, 60);
    drawWaveform(g, waveformBounds);
}

void MainComponent::resized()
{
    // Layout: Playlist at top, two decks side by side below
    auto area = getLocalBounds();
    
    // Playlist takes top 40% of screen
    playlistComponent.setBounds(area.removeFromTop(getHeight() * 0.4));
    
    // Crossfader at bottom (R1C: Mix between decks)
    auto bottomControlsArea = area.removeFromBottom(60);
    crossfaderLabel.setBounds(bottomControlsArea.removeFromTop(20));
    
    auto crossfaderArea = bottomControlsArea.removeFromTop(30).reduced(40, 5);
    crossfader.setBounds(crossfaderArea.removeFromLeft(crossfaderArea.getWidth() * 0.85));
    
    // SYNC button to the right of crossfader - taller and narrower (like cue button)
    int syncButtonWidth = 80;
    int syncButtonHeight = 30;
    int syncButtonX = crossfaderArea.getX() + (crossfaderArea.getWidth() - syncButtonWidth) / 2;
    int syncButtonY = crossfaderArea.getY() - 5; // Shift up slightly
    syncButton.setBounds(syncButtonX, syncButtonY, syncButtonWidth, syncButtonHeight);
    
    // Split remaining area for two decks with spacing
    auto deckArea = area;
    deckGUI1.setBounds(deckArea.removeFromLeft(getWidth() / 2).reduced(5));
    deckGUI2.setBounds(deckArea.reduced(5));
}

// R1C: Crossfader controls mix between Deck 1 and Deck 2
void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &crossfader)
    {
        double position = slider->getValue(); // 0.0 = full left (Deck1), 1.0 = full right (Deck2)
        
        // Calculate crossfade multipliers using equal power crossfade curve
        double mult1 = std::cos(position * juce::MathConstants<double>::halfPi);
        double mult2 = std::sin(position * juce::MathConstants<double>::halfPi);
        
        // Store crossfader multipliers for use in getNextAudioBlock
        crossfadeMultiplier1 = mult1;
        crossfadeMultiplier2 = mult2;
        
        // Debug logging
        juce::Logger::writeToLog("Crossfader moved to: " + juce::String(position, 2) + 
                                " | Deck1: " + juce::String(mult1, 2) + 
                                " | Deck2: " + juce::String(mult2, 2));
        crossfadeMultiplier2 = mult2;
    }
}

// Handle SYNC button click - sync Deck 2 speed to Deck 1
void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &syncButton)
    {
        bool isNowSynced = syncButton.getToggleState();
        
        if (isNowSynced)
        {
            // SYNC mode: Match Deck 2 speed to Deck 1
            double deck1Speed = player1.getSpeed();
            player2.setSpeed(deck1Speed);
            
            // Change button text to SYNCED
            syncButton.setButtonText("SYNCED");
            
            // Sync waveforms in the playlist component
            playlistComponent.syncWaveforms(true);
            
            // After a short delay, un-sync the visual effect but keep speeds matched
            juce::Timer::callAfterDelay(2000, [this]()
            {
                playlistComponent.syncWaveforms(false);
            });
            
            juce::Logger::writeToLog("SYNC: Deck 2 speed matched to Deck 1 (" + 
                                    juce::String(deck1Speed, 2) + "x)");
        }
        else
        {
            // UN-SYNC mode: Restore button text
            syncButton.setButtonText("SYNC");
            juce::Logger::writeToLog("UN-SYNCED: Decks are now independent");
        }
    }
}

// Timer callback to animate waveform
void MainComponent::timerCallback()
{
    repaint(); // Trigger animation
}

// Draw colorful waveform visualization (like rainbow audio wave)
void MainComponent::drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Dark background
    g.setColour(juce::Colour(0xff0a0e1a));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    
    // Border
    g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 2.0f);
    
    // Draw colorful waveform bars
    int numBars = 60;
    float barWidth = bounds.getWidth() / static_cast<float>(numBars);
    float centerY = bounds.getCentreY();
    
    // Get current time for animation
    auto currentTime = juce::Time::getMillisecondCounterHiRes();
    
    for (int i = 0; i < numBars; ++i)
    {
        // Animated wave height using sine wave
        float phase = (currentTime / 500.0) + (i * 0.2f);
        float height = (std::sin(phase) * 0.5f + 0.5f) * 0.8f + 0.2f;
        float barHeight = height * (bounds.getHeight() / 2.0f);
        
        // Color gradient across spectrum (red -> yellow -> green -> cyan -> blue -> magenta)
        float hue = static_cast<float>(i) / static_cast<float>(numBars);
        juce::Colour barColor = juce::Colour::fromHSV(hue, 0.9f, 1.0f, 1.0f);
        
        // Draw bar (mirrored top and bottom from center)
        float x = bounds.getX() + (i * barWidth);
        
        juce::Rectangle<float> topBar(x, centerY - barHeight, barWidth - 2, barHeight);
        juce::Rectangle<float> bottomBar(x, centerY, barWidth - 2, barHeight);
        
        g.setColour(barColor);
        g.fillRect(topBar);
        g.fillRect(bottomBar);
        
        // Add glow effect
        g.setColour(barColor.withAlpha(0.3f));
        g.fillRect(topBar.expanded(1, 2));
        g.fillRect(bottomBar.expanded(1, 2));
    }
}
