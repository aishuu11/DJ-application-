#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "GlossyButtonLookAndFeel.h"

/**
 * PlaylistComponent - Music library/playlist interface
 * R2: Playlist with track management (add, search, favorite, delete)
 */
class PlaylistComponent : public juce::Component,
                          public juce::TableListBoxModel,
                          public juce::Button::Listener,
                          public juce::TextEditor::Listener,
                          public juce::FileDragAndDropTarget,
                          public juce::Timer
{
public:
    /**
     * Constructor - Creates playlist component
     * @param player1 Pointer to deck 1 audio player
     * @param player2 Pointer to deck 2 audio player
     * @param deckGUI1 Pointer to deck 1 GUI
     * @param deckGUI2 Pointer to deck 2 GUI
     * @param formatManager Audio format manager
     */
    PlaylistComponent(DJAudioPlayer* player1,
                     DJAudioPlayer* player2,
                     DeckGUI* deckGUI1,
                     DeckGUI* deckGUI2,
                     juce::AudioFormatManager& formatManager);
    
    /**
     * Destructor
     */
    ~PlaylistComponent() override;

    /**
     * paint - Draws the playlist component
     * @param g Graphics context
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * resized - Arranges child components
     */
    void resized() override;

    // TableListBoxModel overrides
    /**
     * getNumRows - Returns number of rows in table
     * @return Number of tracks to display
     */
    int getNumRows() override;
    
    /**
     * paintRowBackground - Paints row background
     * @param g Graphics context
     * @param rowNumber Row index
     * @param width Row width
     * @param height Row height
     * @param rowIsSelected Whether row is selected
     */
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    
    /**
     * paintCell - Paints individual cell
     * @param g Graphics context
     * @param rowNumber Row index
     * @param columnId Column ID
     * @param width Cell width
     * @param height Cell height
     * @param rowIsSelected Whether row is selected
     */
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    
    /**
     * refreshComponentForCell - Creates/updates custom components for cells
     * @param rowNumber Row index
     * @param columnId Column ID
     * @param isRowSelected Whether row is selected
     * @param existingComponentToUpdate Existing component to reuse
     * @return Component for the cell
     */
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

    /**
     * buttonClicked - Handles button clicks
     * @param button The button that was clicked
     */
    void buttonClicked(juce::Button* button) override;

    /**
     * setDeck1Playing - Updates deck 1 playing status
     * @param isPlaying Whether deck 1 is playing
     */
    void setDeck1Playing(bool isPlaying);
    
    /**
     * setDeck2Playing - Updates deck 2 playing status
     * @param isPlaying Whether deck 2 is playing
     */
    void setDeck2Playing(bool isPlaying);
    
    /**
     * setDeck1Zoom - Updates deck 1 zoom level
     * @param level Zoom level (1, 2, or 3)
     */
    void setDeck1Zoom(int level);
    
    /**
     * setDeck2Zoom - Updates deck 2 zoom level
     * @param level Zoom level (1, 2, or 3)
     */
    void setDeck2Zoom(int level);
    
    /**
     * syncWaveforms - Visually synchronizes waveforms
     * @param shouldSync Whether to show sync effect
     */
    void syncWaveforms(bool shouldSync);

    // TextEditor::Listener overrides
    /**
     * textEditorReturnKeyPressed - Handles search box enter key
     * @param editor The text editor
     */
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    
    /**
     * textEditorTextChanged - Handles search text changes
     * @param editor The text editor
     */
    void textEditorTextChanged(juce::TextEditor& editor) override;

    // FileDragAndDropTarget overrides
    /**
     * isInterestedInFileDrag - Checks if files are acceptable
     * @param files List of file paths
     * @return true if files can be loaded
     */
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    
    /**
     * filesDropped - Handles dropped files
     * @param files List of file paths
     * @param x X coordinate of drop
     * @param y Y coordinate of drop
     */
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    /**
     * timerCallback - Updates playing status indicators
     */
    void timerCallback() override;

private:
    /**
     * TrackInfo - Stores track metadata
     */
    struct TrackInfo
    {
        juce::File file;
        std::string title;            // Track title
        std::string duration;         // Formatted duration string (MM:SS)
        double lengthInSeconds = 0.0; // Raw duration in seconds
        bool isFavorite = false;
        
        juce::String getFormattedDuration() const;
    };
    
    /**
     * loadLibraryState - Loads saved library from file
     */
    void loadLibraryState();
    
    /**
     * saveLibraryState - Saves library to file
     */
    void saveLibraryState();
    
    /**
     * getLibraryFilePath - Gets path to library save file
     * @return Library file path
     */
    juce::File getLibraryFilePath();
    
    /**
     * updateFilteredTracks - Updates filtered track list based on search/filter
     */
    void updateFilteredTracks();
    
    /**
     * matchesSearchQuery - Check if track matches search query
     * @param track Track to check
     * @param query Search query string
     * @return true if track matches query
     */
    bool matchesSearchQuery(const TrackInfo& track, const juce::String& query);
    
    /**
     * getTrackDuration - Extract duration from audio file
     * @param file Audio file to analyze
     * @return Formatted duration string (MM:SS)
     */
    std::string getTrackDuration(const juce::File& file);
    
    /**
     * calculateBPM - Calculate BPM of audio file
     * @param file Audio file to analyze
     * @return BPM value
     */
    double calculateBPM(const juce::File& file);
    
    /**
     * addTrackToLibrary - Add a track to the library
     * @param file Audio file to add
     */
    void addTrackToLibrary(const juce::File& file);
    
    /**
     * WaveformDisplay - Simple waveform visualization component
     */
    class WaveformDisplay : public juce::Component
    {
    public:
        WaveformDisplay(DJAudioPlayer* player = nullptr, int deckNum = 0);
        
        void paint(juce::Graphics& g) override;
        void updateWaveform();
        void setSynced(bool isSynced);
        void setBPM(double newBPM);
        void setZoomLevel(int level);
        
    private:
        DJAudioPlayer* audioPlayer;
        int deckNumber;
        bool isSynced = false;
        double currentBPM = 0.0;
        int zoomLevel = 1;
        float previousLevel = 0.0f;
        
        std::vector<float> waveformData;
        std::vector<bool> beatMarkers;
        
        juce::Colour neonPink{0xffff1493}; // Deep pink neon
    };
    
    // Pointers to audio players and deck GUIs
    DJAudioPlayer* player1;
    DJAudioPlayer* player2;
    DeckGUI* deckGUI1;
    DeckGUI* deckGUI2;
    
    // Audio format manager
    juce::AudioFormatManager& formatManager;
    
    // UI components
    juce::TableListBox tableComponent;
    
    // Button to add tracks
    juce::TextButton addButton{"ADD TRACKS"};
    
    // Search box
    juce::TextEditor searchBox;
    juce::Label searchLabel;
    
    // Track library (all tracks)
    std::vector<TrackInfo> tracks;
    
    // Filtered tracks based on search/favorites
    std::vector<int> filteredTrackIndices;
    
    // BPM detection waveforms (must be before filterMode in declaration order)
    WaveformDisplay waveform1;
    WaveformDisplay waveform2;
    
    // Filter mode: 0 = show all, 1 = show favorites only
    int filterMode;
    
    // Button to filter favorites
    juce::TextButton favoritesFilterButton{"SHOW FAVORITES"};
    
    // File chooser for loading tracks
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    // Thread pool for background tasks
    juce::ThreadPool threadPool{2}; // 2 threads for background work
    
    // Custom look and feel
    GlossyButtonLookAndFeel glossyLookAndFeel;
    
    // Playing status for each deck
    bool deck1Playing = false;
    bool deck2Playing = false;
    
    // Track playing state (for playlist display)
    bool playingDeck1 = false;
    bool playingDeck2 = false;
    juce::Label bpmLabel1;
    juce::Label bpmLabel2;
    juce::Label bpmSectionTitle;
    bool waveformsSynced = false;
    
    // Thread safety
    juce::CriticalSection tracksMutex;
    
    // Track loading state for each deck
    juce::File loadedTrackDeck1;
    juce::File loadedTrackDeck2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};
