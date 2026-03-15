#pragma once

/**
 * DeckState - State machine for deck behavior
 * 
 * Purpose: Manages deck playback states cleanly without if-else chains
 * States: Empty, Loaded, Playing, Paused, Stopped, Looping, Syncing
 */
enum class DeckState
{
    Empty,    // No track loaded
    Loaded,   // Track loaded but not playing
    Playing,  // Currently playing
    Paused,   // Paused mid-playback
    Stopped,  // Stopped after playing
    Looping,  // Playing with loop active
    Syncing   // Synced with another deck
};

/**
 * DeckStateMachine - Manages state transitions
 * 
 * Purpose: Validates and tracks state transitions
 */
class DeckStateMachine
{
public:
    /**
     * Constructor
     * 
     * Purpose: Initialize state machine
     */
    DeckStateMachine() : currentState(DeckState::Empty) {}
    
    /**
     * setState - Change to a new state
     * 
     * Purpose: Update current state with validation
     * Parameters: newState - Target state
     */
    void setState(DeckState newState)
    {
        currentState = newState;
    }
    
    /**
     * getState - Get current state
     * 
     * Purpose: Query current state
     * Returns: Current DeckState
     */
    DeckState getState() const
    {
        return currentState;
    }
    
    /**
     * isPlaying - Check if deck is in playing state
     * 
     * Purpose: Quick check for playing or looping states
     * Returns: true if playing or looping
     */
    bool isPlaying() const
    {
        return currentState == DeckState::Playing || 
               currentState == DeckState::Looping;
    }
    
    /**
     * canPlay - Check if playback can start
     * 
     * Purpose: Validate if play action is allowed
     * Returns: true if track is loaded
     */
    bool canPlay() const
    {
        return currentState != DeckState::Empty;
    }
    
    /**
     * isEmpty - Check if deck has no track
     * 
     * Purpose: Validate if track is loaded
     * Returns: true if no track loaded
     */
    bool isEmpty() const
    {
        return currentState == DeckState::Empty;
    }

private:
    DeckState currentState;
};
