#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"

/**
 * ITrackFeature - Base interface for track features (polymorphism)
 * 
 * Purpose: Enable polymorphic behavior for different track features
 * Supports: HotCue, EQ, BPM detection, etc.
 */
class ITrackFeature
{
public:
    virtual ~ITrackFeature() = default;
    
    /**
     * apply - Apply this feature to audio player
     * 
     * Purpose: Execute feature-specific behavior
     * Parameters: player - Target audio player
     */
    virtual void apply(DJAudioPlayer* player) = 0;
    
    /**
     * reset - Reset feature to default state
     * 
     * Purpose: Clear feature settings
     */
    virtual void reset() = 0;
    
    /**
     * getName - Get feature name
     * 
     * Purpose: Identify feature type
     * Returns: Feature name string
     */
    virtual juce::String getName() const = 0;
};

/**
 * HotCueFeature - Manages hot cue points
 * 
 * Purpose: Store and jump to cue points
 */
class HotCueFeature : public ITrackFeature
{
public:
    /**
     * Constructor
     * 
     * Purpose: Initialize cue feature with position
     * Parameters: position - Cue point position (0.0 to 1.0)
     */
    HotCueFeature(double position) : cuePosition(position) {}
    
    /**
     * apply - Jump to cue point
     * 
     * Purpose: Set playback position to cue point
     * Parameters: player - Target audio player
     */
    void apply(DJAudioPlayer* player) override
    {
        if (player && cuePosition >= 0.0)
        {
            player->setPositionRelative(cuePosition);
        }
    }
    
    /**
     * reset - Clear cue point
     * 
     * Purpose: Remove cue point
     */
    void reset() override
    {
        cuePosition = -1.0;
    }
    
    /**
     * getName - Get feature name
     * 
     * Purpose: Identify as hot cue feature
     * Returns: "HotCue"
     */
    juce::String getName() const override
    {
        return "HotCue";
    }
    
    /**
     * getPosition - Get cue position
     * 
     * Purpose: Query cue point location
     * Returns: Position from 0.0 to 1.0
     */
    double getPosition() const
    {
        return cuePosition;
    }
    
    /**
     * setPosition - Update cue position
     * 
     * Purpose: Change cue point location
     * Parameters: position - New position (0.0 to 1.0)
     */
    void setPosition(double position)
    {
        cuePosition = position;
    }

private:
    double cuePosition;
};

/**
 * EQFeature - Manages 3-band equalizer
 * 
 * Purpose: Store and apply EQ settings
 */
class EQFeature : public ITrackFeature
{
public:
    /**
     * Constructor
     * 
     * Purpose: Initialize EQ with flat response
     * Parameters: low, mid, high - EQ gains in dB
     */
    EQFeature(double low = 0.0, double mid = 0.0, double high = 0.0)
        : lowGain(low), midGain(mid), highGain(high) {}
    
    /**
     * apply - Apply EQ settings
     * 
     * Purpose: Set EQ gains on audio player
     * Parameters: player - Target audio player
     */
    void apply(DJAudioPlayer* player) override
    {
        if (player)
        {
            player->setLowEQ(lowGain);
            player->setMidEQ(midGain);
            player->setHighEQ(highGain);
        }
    }
    
    /**
     * reset - Reset EQ to flat
     * 
     * Purpose: Clear all EQ adjustments
     */
    void reset() override
    {
        lowGain = midGain = highGain = 0.0;
    }
    
    /**
     * getName - Get feature name
     * 
     * Purpose: Identify as EQ feature
     * Returns: "EQ"
     */
    juce::String getName() const override
    {
        return "EQ";
    }
    
    /**
     * setLow - Set low band gain
     * 
     * Purpose: Adjust low frequencies
     * Parameters: gain - Gain in dB
     */
    void setLow(double gain) { lowGain = gain; }
    
    /**
     * setMid - Set mid band gain
     * 
     * Purpose: Adjust mid frequencies
     * Parameters: gain - Gain in dB
     */
    void setMid(double gain) { midGain = gain; }
    
    /**
     * setHigh - Set high band gain
     * 
     * Purpose: Adjust high frequencies
     * Parameters: gain - Gain in dB
     */
    void setHigh(double gain) { highGain = gain; }

private:
    double lowGain, midGain, highGain;
};

/**
 * BPMFeature - Manages BPM detection and syncing
 * 
 * Purpose: Store and sync tempo
 */
class BPMFeature : public ITrackFeature
{
public:
    /**
     * Constructor
     * 
     * Purpose: Initialize BPM feature
     * Parameters: bpm - Beats per minute
     */
    BPMFeature(double bpm = 120.0) : bpmValue(bpm) {}
    
    /**
     * apply - Apply BPM sync
     * 
     * Purpose: Adjust playback speed to match BPM
     * Parameters: player - Target audio player
     */
    void apply(DJAudioPlayer* player) override
    {
        // BPM application handled externally
        // This stores the value for sync calculations
    }
    
    /**
     * reset - Reset BPM to default
     * 
     * Purpose: Clear BPM value
     */
    void reset() override
    {
        bpmValue = 120.0;
    }
    
    /**
     * getName - Get feature name
     * 
     * Purpose: Identify as BPM feature
     * Returns: "BPM"
     */
    juce::String getName() const override
    {
        return "BPM";
    }
    
    /**
     * getBPM - Get BPM value
     * 
     * Purpose: Query tempo
     * Returns: Beats per minute
     */
    double getBPM() const
    {
        return bpmValue;
    }
    
    /**
     * setBPM - Set BPM value
     * 
     * Purpose: Update tempo
     * Parameters: bpm - Beats per minute
     */
    void setBPM(double bpm)
    {
        bpmValue = bpm;
    }

private:
    double bpmValue;
};
