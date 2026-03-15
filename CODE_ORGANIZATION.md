# Code Organization

## Project Structure

This is a JUCE-based DJ application. JUCE projects keep source files in the root directory by design (managed by Projucer).

### Core Application Files

**Main.cpp**
- Entry point for the application

**MainComponent.h/cpp**
- Main window and audio mixer
- Handles crossfader between two decks
- Top-level waveform visualization

### DJ Deck Components

**DeckGUI.h/cpp**
- Individual deck interface 
- Controls: play, stop, loop, pitch, volume, speed
- Hot cues: 8 programmable cue points per deck
- 3-band EQ (low/mid/high)
- Jog wheel visualization

**DJAudioPlayer.h/cpp**
- Audio playback engine
- Handles loading, playing, looping
- EQ processing

### Playlist & Library

**PlaylistComponent.h/cpp**
- Music library with search and favorites
- Load tracks to either deck
- BPM detection and waveform displays
- Beat markers (yellow lines show detected beats)
- Saves/loads library state

### Visual Styling

**GlossyButtonLookAndFeel.h/cpp**
- Custom 3D glossy button style used throughout

### OOP Design Patterns

**DeckState.h**
- State machine for deck behavior
- States: Empty, Loaded, Playing, Paused, Stopped, Looping, Syncing

**ITrackFeature.h**
- Polymorphic base class for track features
- Implementations: HotCueFeature, EQFeature, BPMFeature

## Build System

- **Otodecks.jucer**: JUCE Projucer project file (regenerates build systems)
- **CMakeLists.txt**: CMake build (has macOS 15 compatibility issues)
- **Builds/MacOSX/**: Xcode project (recommended for macOS)


## Key Features

1. **Dual Deck System**: Two independent DJ decks
2. **Hot Cues**: 8 programmable jump points per deck
3. **Looping**: Set loop in/out points
4. **3-Band EQ**: Low/mid/high frequency control
5. **BPM Detection**: Automatic beat detection (yellow waveform markers)
6. **Music Library**: Search, favorites, persistent storage
7. **Crossfader**: Smooth mixing between decks
8. **Visual Feedback**: Jog wheels, waveforms, beat detection

## Yellow Lines on Waveform

The yellow lines are **beat markers** - they use onset detection to identify beats in your music. When the audio energy suddenly spikes (like a kick drum), the code marks it yellow to help you visualize the rhythm. This makes it easier to mix tracks by matching their beats.
