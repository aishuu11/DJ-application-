# DJ Application

A C++ desktop DJ mixing application developed with the JUCE framework, supporting dual decks, waveform visualisation, BPM analysis, EQ filtering, hot cues, and interactive jog wheel controls for live audio mixing.

## Features

- Dual deck audio playback
- Playlist management UI
- Deck controls for loading and playback
- Custom GUI styling

## Project Structure

Key files:

- `Main.cpp` / `MainComponent.*` – app entry and main UI container
- `DeckGUI.*` – deck user interface and interactions
- `DJAudioPlayer.*` – audio playback logic
- `PlaylistComponent.*` – playlist/table UI and controls
- `GlossyButtonLookAndFeel.*` – custom button styling

## Build (CMake)

### Prerequisites

- CMake 3.22+
- Xcode command line tools (on macOS)
- A C++17-compatible compiler

### Steps

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## Build (Xcode/Projucer output)

You can also use the generated project under:

- `Builds/MacOSX/Otodecks.xcodeproj`

## Notes

- JUCE source is included in this repository.
- Generated build folders are currently present and can be cleaned if needed.

## Author

Aishwarya
