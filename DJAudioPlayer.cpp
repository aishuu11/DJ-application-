#include "DJAudioPlayer.h"

//==============================================================================
// C2 & C3: Constructor implementation with clear purpose
// Purpose: Initialize the DJ audio player with audio format manager
DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& _formatManager)
    : formatManager(_formatManager)
{
}

// C2: Destructor - clean up resources
DJAudioPlayer::~DJAudioPlayer()
{
}

//==============================================================================
// C2 & C5: Audio system preparation (stateless - uses parameters, not globals)
// Purpose: Configure audio processing with sample rate and buffer size
void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // C7: Explicit setter - clearly modifies currentSampleRate state
    // Store sample rate for filter configuration (R4)
    currentSampleRate = sampleRate;
    
    // Prepare the transport source
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    
    // Prepare the resampling source for speed control (R1D)
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    
    // R4: Initialize EQ filters for each channel
    for (int channel = 0; channel < 2; ++channel)
    {
        lowShelfFilter[channel].reset();
        highShelfFilter[channel].reset();
        peakFilter[channel].reset();
    }
    
    // Set initial filter coefficients (flat EQ)
    setLowEQ(0.0);
    setMidEQ(0.0);
    setHighEQ(0.0);
}

void DJAudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Check loop boundaries if looping is enabled
    if (looping && transportSource.getLengthInSeconds() > 0)
    {
        double currentPos = getPositionRelative();
        if (currentPos >= loopEndPos)
        {
            // Jump back to loop start
            setPositionRelative(loopStartPos);
        }
    }
    
    // Get audio from the resampling source (which applies speed changes)
    resampleSource.getNextAudioBlock(bufferToFill);
    
    // R4: Apply 3-band EQ filters to each channel separately
    int numChannels = juce::jmin(2, bufferToFill.buffer->getNumChannels());
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        
        // Apply filters in series: low shelf -> peak (mid) -> high shelf
        lowShelfFilter[channel].processSamples(channelData, bufferToFill.numSamples);
        peakFilter[channel].processSamples(channelData, bufferToFill.numSamples);
        highShelfFilter[channel].processSamples(channelData, bufferToFill.numSamples);
    }
    
    // Calculate RMS level for visualization
    float level = 0.0f;
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        level = bufferToFill.buffer->getRMSLevel(0, bufferToFill.startSample, bufferToFill.numSamples);
        
        // If stereo, average with right channel
        if (bufferToFill.buffer->getNumChannels() > 1)
        {
            float rightLevel = bufferToFill.buffer->getRMSLevel(1, bufferToFill.startSample, bufferToFill.numSamples);
            level = (level + rightLevel) * 0.5f;
        }
    }
    
    // Store level (atomic for thread safety)
    currentAudioLevel.store(level);
}

void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

//==============================================================================
// R1A: Load audio file into the player
void DJAudioPlayer::loadURL(juce::URL audioURL)
{
    juce::Logger::writeToLog("Attempting to load: " + audioURL.toString(false));
    
    // Create an input stream for the audio file
    auto inputStream = audioURL.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress));
    
    if (inputStream == nullptr)
    {
        juce::Logger::writeToLog("ERROR: Could not create input stream for " + audioURL.toString(false));
        return;
    }
    
    // Create a reader for the audio file
    auto* reader = formatManager.createReaderFor(std::move(inputStream));

    if (reader != nullptr) // Valid file
    {
        juce::Logger::writeToLog("Successfully created reader for: " + audioURL.toString(false));
        juce::Logger::writeToLog("Sample rate: " + juce::String(reader->sampleRate) + " Hz");
        juce::Logger::writeToLog("Length: " + juce::String(reader->lengthInSamples / reader->sampleRate) + " seconds");
        
        // Create a new reader source
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(
            new juce::AudioFormatReaderSource(reader, true));
        
        // Set it as the transport source
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        
        // Transfer ownership
        readerSource.reset(newSource.release());
        
        juce::Logger::writeToLog("Audio file loaded successfully - Transport length: " + juce::String(transportSource.getLengthInSeconds()) + " sec");
    }
    else
    {
        juce::Logger::writeToLog("ERROR: Could not create reader for " + audioURL.toString(false));
        juce::Logger::writeToLog("File may be corrupt or format not supported");
    }
}

// R1C: Set volume/gain for mixing
void DJAudioPlayer::setGain(double gain)
{
    // Clamp gain between 0.0 and 1.0
    if (gain < 0 || gain > 1.0)
    {
        juce::Logger::writeToLog("DJAudioPlayer::setGain gain should be between 0 and 1");
        return;
    }
    
    // Set the gain on the transport source
    transportSource.setGain(gain);
}

// R1D: Set playback speed
void DJAudioPlayer::setSpeed(double ratio)
{
    // Validate speed ratio
    if (ratio < 0 || ratio > 100.0)
    {
        juce::Logger::writeToLog("DJAudioPlayer::setSpeed ratio should be between 0 and 100");
        return;
    }
    
    // Store current speed
    currentSpeed = ratio;
    
    // Set the resampling ratio (1.0 = normal speed)
    resampleSource.setResamplingRatio(ratio);
}

double DJAudioPlayer::getSpeed() const
{
    return currentSpeed;
}

void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
    // Validate position
    if (pos < 0 || pos > 1.0)
    {
        juce::Logger::writeToLog("DJAudioPlayer::setPositionRelative pos should be between 0 and 1");
        return;
    }
    
    double posInSecs = transportSource.getLengthInSeconds() * pos;
    setPosition(posInSecs);
}

void DJAudioPlayer::skipForward(double seconds)
{
    double currentPos = transportSource.getCurrentPosition();
    double newPos = currentPos + seconds;
    double trackLength = transportSource.getLengthInSeconds();
    
    juce::Logger::writeToLog("Skip forward: current=" + juce::String(currentPos) + 
                           " + " + juce::String(seconds) + " = " + juce::String(newPos));
    
    // Clamp to track length
    if (newPos > trackLength)
    {
        newPos = trackLength;
        juce::Logger::writeToLog("Clamped to track length: " + juce::String(trackLength));
    }
    
    setPosition(newPos);
    juce::Logger::writeToLog("New position set to: " + juce::String(newPos));
}

void DJAudioPlayer::skipBackward(double seconds)
{
    double currentPos = transportSource.getCurrentPosition();
    double newPos = currentPos - seconds;
    
    juce::Logger::writeToLog("Skip backward: current=" + juce::String(currentPos) + 
                           " - " + juce::String(seconds) + " = " + juce::String(newPos));
    
    // Clamp to 0
    if (newPos < 0.0)
    {
        newPos = 0.0;
        juce::Logger::writeToLog("Clamped to zero");
    }
    
    setPosition(newPos);
}

void DJAudioPlayer::start()
{
    juce::Logger::writeToLog("=== DJAudioPlayer::start() called ===");
    juce::Logger::writeToLog("Transport length: " + juce::String(transportSource.getLengthInSeconds()) + " seconds");
    juce::Logger::writeToLog("Current position: " + juce::String(transportSource.getCurrentPosition()) + " seconds");
    juce::Logger::writeToLog("Gain: " + juce::String(transportSource.getGain()));
    juce::Logger::writeToLog("Before start - isPlaying: " + juce::String(transportSource.isPlaying() ? "true" : "false"));
    
    if (transportSource.getLengthInSeconds() <= 0.0)
    {
        juce::Logger::writeToLog("ERROR: No audio loaded (length is 0)");
        return;
    }
    
    transportSource.start();
    
    juce::Logger::writeToLog("After start() - isPlaying: " + juce::String(transportSource.isPlaying() ? "true" : "false"));
    
    if (!transportSource.isPlaying())
    {
        juce::Logger::writeToLog("WARNING: start() was called but transport is not playing!");
    }
}

void DJAudioPlayer::stop()
{
    transportSource.stop();
}

double DJAudioPlayer::getPositionRelative()
{
    double length = transportSource.getLengthInSeconds();
    if (length > 0)
    {
        return transportSource.getCurrentPosition() / length;
    }
    return 0.0;
}

bool DJAudioPlayer::isPlaying()
{
    return transportSource.isPlaying();
}

float DJAudioPlayer::getAudioLevel() const
{
    return currentAudioLevel.load();
}

void DJAudioPlayer::setLooping(bool shouldLoop, double loopStart, double loopEnd)
{
    looping = shouldLoop;
    loopStartPos = juce::jlimit(0.0, 1.0, loopStart);
    loopEndPos = juce::jlimit(0.0, 1.0, loopEnd);
}

//==============================================================================
// R4: 3-Band Equalizer Functions

void DJAudioPlayer::setLowEQ(double gainDB)
{
    lowEQGain = gainDB;
    
    // Configure low shelf filter (20Hz-300Hz) for both channels
    auto coefficients = juce::IIRCoefficients::makeLowShelf(
        currentSampleRate,
        200.0,  // Center frequency (200Hz for low band)
        0.7,    // Q factor
        juce::Decibels::decibelsToGain(static_cast<float>(gainDB))
    );
    
    for (int channel = 0; channel < 2; ++channel)
    {
        lowShelfFilter[channel].setCoefficients(coefficients);
    }
}

void DJAudioPlayer::setMidEQ(double gainDB)
{
    midEQGain = gainDB;
    
    // Configure peak filter (300Hz-4kHz) for both channels
    auto coefficients = juce::IIRCoefficients::makePeakFilter(
        currentSampleRate,
        1500.0, // Center frequency (1.5kHz for mid band)
        1.0,    // Q factor
        juce::Decibels::decibelsToGain(static_cast<float>(gainDB))
    );
    
    for (int channel = 0; channel < 2; ++channel)
    {
        peakFilter[channel].setCoefficients(coefficients);
    }
}

void DJAudioPlayer::setHighEQ(double gainDB)
{
    highEQGain = gainDB;
    
    // Configure high shelf filter (4kHz-20kHz) for both channels
    auto coefficients = juce::IIRCoefficients::makeHighShelf(
        currentSampleRate,
        6000.0, // Center frequency (6kHz for high band)
        0.7,    // Q factor
        juce::Decibels::decibelsToGain(static_cast<float>(gainDB))
    );
    
    for (int channel = 0; channel < 2; ++channel)
    {
        highShelfFilter[channel].setCoefficients(coefficients);
    }
}

double DJAudioPlayer::getLowEQ() const
{
    return lowEQGain;
}

double DJAudioPlayer::getMidEQ() const
{
    return midEQGain;
}

double DJAudioPlayer::getHighEQ() const
{
    return highEQGain;
}
