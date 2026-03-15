#pragma once

#include <JuceHeader.h>

/**
 * DJAudioPlayer - Handles audio playback for a single deck
 * Implements R1A (load audio), R1C (volume control), R1D (speed control)
 */
class DJAudioPlayer : public juce::AudioSource
{
public:
    /**
     * Constructor - Initializes the audio player
     * @param formatManager Reference to the audio format manager
     */
    DJAudioPlayer(juce::AudioFormatManager& formatManager);
    
    /**
     * Destructor - Cleans up resources
     */
    ~DJAudioPlayer();

    /**
     * prepareToPlay - Sets up audio processing
     * @param samplesPerBlockExpected Number of samples in each processing block
     * @param sampleRate Sample rate in Hz
     */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    
    /**
     * getNextAudioBlock - Processes audio output
     * @param bufferToFill Audio buffer to fill with processed audio
     */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    
    /**
     * releaseResources - Releases audio resources
     */
    void releaseResources() override;

    /**
     * loadURL - Loads an audio file from a URL (R1A: Load audio files)
     * @param audioURL URL of the audio file to load
     */
    void loadURL(juce::URL audioURL);
    
    /**
     * setGain - Sets the volume/gain (R1C: Mix by varying volume)
     * @param gain Volume level (0.0 to 1.0)
     */
    void setGain(double gain);
    
    /**
     * setSpeed - Sets the playback speed (R1D: Speed up and slow down)
     * @param ratio Speed ratio (1.0 = normal, 2.0 = double speed, 0.5 = half speed)
     */
    void setSpeed(double ratio);
    
    /**
     * getSpeed - Gets the current playback speed
     * @return Current speed ratio
     */
    double getSpeed() const;
    
    /**
     * setPosition - Sets the playback position
     * @param posInSecs Position in seconds
     */
    void setPosition(double posInSecs);
    
    /**
     * setPositionRelative - Sets position as a fraction of track length
     * @param pos Position from 0.0 to 1.0
     */
    void setPositionRelative(double pos);
    
    /**
     * skipForward - Skip forward by specified seconds
     * @param seconds Number of seconds to skip forward
     */
    void skipForward(double seconds);
    
    /**
     * skipBackward - Skip backward by specified seconds
     * @param seconds Number of seconds to skip backward
     */
    void skipBackward(double seconds);
    
    /**
     * start - Starts playback
     */
    void start();
    
    /**
     * stop - Stops playback
     */
    void stop();
    
    /**
     * getPositionRelative - Gets current position as fraction of track length
     * @return Position from 0.0 to 1.0
     */
    double getPositionRelative();
    
    /**
     * isPlaying - Checks if audio is currently playing
     * @return True if playing, false otherwise
     */
    bool isPlaying();
    
    /**
     * getAudioLevel - Gets current audio amplitude level (0.0 to 1.0)
     * @return Current audio level for visualization
     */
    float getAudioLevel() const;
    
    /**
     * setLooping - Enable/disable looping between two points
     * @param shouldLoop Whether to loop
     * @param loopStart Loop start position (0.0 to 1.0)
     * @param loopEnd Loop end position (0.0 to 1.0)
     */
    void setLooping(bool shouldLoop, double loopStart = 0.0, double loopEnd = 1.0);
    
    /**
     * isLooping - Check if looping is enabled
     * @return true if looping is active
     */
    bool isLooping() const { return looping; }
    
    /**
     * setHighEQ - Sets high frequency EQ gain (R4: 4kHz-20kHz)
     * @param gainDB Gain in decibels (-24 to +12, professional DJ range)
     */
    void setHighEQ(double gainDB);
    
    /**
     * setMidEQ - Sets mid frequency EQ gain (R4: 300Hz-4kHz)
     * @param gainDB Gain in decibels (-24 to +12, professional DJ range)
     */
    void setMidEQ(double gainDB);
    
    /**
     * setLowEQ - Sets low frequency EQ gain (R4: 20Hz-300Hz)
     * @param gainDB Gain in decibels (-24 to +12, professional DJ range)
     */
    void setLowEQ(double gainDB);
    
    /**
     * getHighEQ - Gets high frequency EQ gain (R4)
     * @return Gain in decibels
     */
    double getHighEQ() const;
    
    /**
     * getMidEQ - Gets mid frequency EQ gain (R4)
     * @return Gain in decibels
     */
    double getMidEQ() const;
    
    /**
     * getLowEQ - Gets low frequency EQ gain (R4)
     * @return Gain in decibels
     */
    double getLowEQ() const;

private:
    // Audio format manager reference
    juce::AudioFormatManager& formatManager;
    
    // Reader for audio file data
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    // Transport source for playback control
    juce::AudioTransportSource transportSource;
    
    // Resampling source for speed control (R1D)
    juce::ResamplingAudioSource resampleSource{&transportSource, false, 2};
    
    // Audio level tracking for visualization
    std::atomic<float> currentAudioLevel{0.0f};
    
    // Current playback speed
    double currentSpeed{1.0};
    
    // Loop settings
    bool looping = false;
    double loopStartPos = 0.0; // 0.0 to 1.0
    double loopEndPos = 1.0;   // 0.0 to 1.0
    
    // R4: 3-Band EQ parameters
    double highEQGain{0.0};  // High frequency gain in dB
    double midEQGain{0.0};   // Mid frequency gain in dB
    double lowEQGain{0.0};   // Low frequency gain in dB
    
    // R4: DSP filters for 3-band EQ (separate filters for each channel)
    juce::IIRFilter lowShelfFilter[2];   // Low frequencies (20Hz-300Hz) - stereo
    juce::IIRFilter highShelfFilter[2];  // High frequencies (4kHz-20kHz) - stereo
    juce::IIRFilter peakFilter[2];       // Mid frequencies (300Hz-4kHz) - stereo
    
    double currentSampleRate{44100.0}; // Store sample rate for filter updates

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJAudioPlayer)
};
