#include "PlaylistComponent.h"
#include <fstream>

//==============================================================================
PlaylistComponent::PlaylistComponent(DJAudioPlayer* _player1,
                                     DJAudioPlayer* _player2,
                                     DeckGUI* _deckGUI1,
                                     DeckGUI* _deckGUI2,
                                     juce::AudioFormatManager& _formatManager)
    : player1(_player1), 
      player2(_player2), 
      deckGUI1(_deckGUI1), 
      deckGUI2(_deckGUI2), 
      formatManager(_formatManager),
      waveform1(_player1, 1),
      waveform2(_player2, 2),
      filterMode(0)
{
    // Set up the table component
    addAndMakeVisible(tableComponent);
    tableComponent.setModel(this);
    tableComponent.setColour(juce::ListBox::backgroundColourId, juce::Colours::black);
    tableComponent.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff00d9ff).withAlpha(0.3f));
    
    // Table columns
    tableComponent.getHeader().addColumn("Track Title", 1, 250);
    tableComponent.getHeader().addColumn("Duration", 2, 70);
    tableComponent.getHeader().addColumn("Deck 1", 3, 80);
    tableComponent.getHeader().addColumn("Deck 2", 4, 80);
    tableComponent.getHeader().addColumn("Unload 1", 5, 80);
    tableComponent.getHeader().addColumn("Unload 2", 6, 80);
    tableComponent.getHeader().addColumn("FAV", 7, 50); // Favorite column
    tableComponent.getHeader().addColumn("DEL", 8, 50); // Delete column
    
    // Style table header - black background with white text
    auto& header = tableComponent.getHeader();
    header.setColour(juce::TableHeaderComponent::backgroundColourId, juce::Colours::black);
    header.setColour(juce::TableHeaderComponent::textColourId, juce::Colours::white);
    header.setColour(juce::TableHeaderComponent::outlineColourId, juce::Colour(0xff00d9ff).withAlpha(0.3f));

    // Add the "Add Tracks" button - white with black text, bold font
    addAndMakeVisible(addButton);
    addButton.addListener(this);
    addButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    addButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    addButton.setLookAndFeel(&glossyLookAndFeel);
    
    // Add search box
    addAndMakeVisible(searchBox);
    addAndMakeVisible(searchLabel);
    searchBox.setTextToShowWhenEmpty("Search tracks...", juce::Colours::grey);
    searchBox.addListener(this);
    searchBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1a1a1a));
    searchBox.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    searchBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::white.withAlpha(0.3f));
    searchLabel.setText("Search:", juce::dontSendNotification);
    searchLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    searchLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    
    // Add favorites filter button - white with black text, bold font
    addAndMakeVisible(favoritesFilterButton);
    favoritesFilterButton.addListener(this);
    favoritesFilterButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    favoritesFilterButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    favoritesFilterButton.setLookAndFeel(&glossyLookAndFeel);
    favoritesFilterButton.setClickingTogglesState(true);
    
    // BPM Analysis section components
    addAndMakeVisible(waveform1);
    addAndMakeVisible(waveform2);
    // Show deck labels above waveforms
    addAndMakeVisible(bpmLabel1);
    addAndMakeVisible(bpmLabel2);
    addAndMakeVisible(bpmSectionTitle);
    
    bpmSectionTitle.setText("BPM DETECTION", juce::dontSendNotification);
    bpmSectionTitle.setColour(juce::Label::textColourId, juce::Colours::white);
    bpmSectionTitle.setFont(juce::Font(16.0f, juce::Font::bold));
    bpmSectionTitle.setJustificationType(juce::Justification::centred);
    
    // Set deck labels (not BPM)
    bpmLabel1.setText("DECK 1", juce::dontSendNotification);
    bpmLabel1.setColour(juce::Label::textColourId, juce::Colour(0xff00d4ff)); // Cyan
    bpmLabel1.setFont(juce::Font(14.0f, juce::Font::bold));
    bpmLabel1.setJustificationType(juce::Justification::centred);
    
    bpmLabel2.setText("DECK 2", juce::dontSendNotification);
    bpmLabel2.setColour(juce::Label::textColourId, juce::Colour(0xffff1493)); // Pink
    bpmLabel2.setFont(juce::Font(14.0f, juce::Font::bold));
    bpmLabel2.setJustificationType(juce::Justification::centred);
    
    updateFilteredTracks();
    startTimer(500);
    
    // Load saved tracks
    threadPool.addJob([this]()
    {
        loadLibraryState();
        
        // Update UI on message thread after loading
        juce::MessageManager::callAsync([this]()
        {
            updateFilteredTracks();
            tableComponent.updateContent();
        });
    });
}

PlaylistComponent::~PlaylistComponent()
{
    // Stop timer
    stopTimer();
    
    // Wait for all background jobs to complete before destruction
    threadPool.removeAllJobs(true, 5000); // Wait up to 5 seconds
    
    // R2C: Save library state on exit (synchronous in destructor to ensure completion)
    saveLibraryState();
}

//==============================================================================
void PlaylistComponent::paint(juce::Graphics& g)
{
    // Glossy black background
    g.fillAll(juce::Colours::black);
    
    // Draw "MUSIC LIBRARY" centered in LEFT section only
    auto leftSection = getLocalBounds().removeFromLeft(static_cast<int>(getWidth() * 0.55));
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("MUSIC LIBRARY", leftSection.removeFromTop(25),
               juce::Justification::centred, true);
}

void PlaylistComponent::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(25); // Space for title
    
    // Split into LEFT (Music Library) and RIGHT (BPM Detection)
    auto leftSection = area.removeFromLeft(static_cast<int>(getWidth() * 0.55)); // 55% for library
    auto rightSection = area; // Remaining 45% for BPM analysis
    
    // === LEFT SECTION: Music Library ===
    // Search box at top
    auto searchArea = leftSection.removeFromTop(30);
    searchLabel.setBounds(searchArea.removeFromLeft(60));
    searchBox.setBounds(searchArea.reduced(5));
    
    // Filter buttons at bottom - increased height to 45
    auto buttonArea = leftSection.removeFromBottom(45);
    addButton.setBounds(buttonArea.removeFromLeft(leftSection.getWidth() / 2).reduced(5));
    favoritesFilterButton.setBounds(buttonArea.reduced(5));
    
    // Table fills remaining space in left section
    tableComponent.setBounds(leftSection);
    
    // === RIGHT SECTION: BPM Detection & Waveforms ===
    rightSection.reduce(10, 5); // Add padding
    
    // Title at top
    bpmSectionTitle.setBounds(rightSection.removeFromTop(30));
    rightSection.removeFromTop(5); // Gap
    
    // Deck 1 Waveform with label
    bpmLabel1.setBounds(rightSection.removeFromTop(22)); // DECK 1 label
    auto waveform1Area = rightSection.removeFromTop((rightSection.getHeight() - 32) / 2);
    waveform1.setBounds(waveform1Area.reduced(5));
    
    rightSection.removeFromTop(10); // Gap between waveforms
    
    // Deck 2 Waveform with label
    bpmLabel2.setBounds(rightSection.removeFromTop(22)); // DECK 2 label
    waveform2.setBounds(rightSection.reduced(5));
}

//==============================================================================
int PlaylistComponent::getNumRows()
{
    const juce::ScopedLock lock(tracksMutex);
    return static_cast<int>(filteredTrackIndices.size());
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g, int rowNumber, 
                                           int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colour(0xff1a1a1a)); // Dark gray highlight
    else
        g.fillAll(rowNumber % 2 == 0 ? juce::Colour(0xff0a0a0a) : juce::Colours::black); // Alternating black rows
}

// R2A: Display track details (filename and duration)
void PlaylistComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId,
                                  int width, int height, bool rowIsSelected)
{
    const juce::ScopedLock lock(tracksMutex);
    
    if (rowNumber >= 0 && rowNumber < static_cast<int>(filteredTrackIndices.size()))
    {
        int trackIndex = filteredTrackIndices[rowNumber];
        
        if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
        {
            // Dark neon pink for track names
            g.setColour(juce::Colour(0xffff1493)); // Dark neon pink
            
            if (columnId == 1) // Track title column
            {
                // Show star "*" for favorites
                juce::String displayText = juce::String(tracks[trackIndex].title);
                if (tracks[trackIndex].isFavorite)
                    displayText = "* " + displayText;
                    
                g.drawText(displayText, 2, 0, width - 4, height,
                           juce::Justification::centredLeft, true);
            }
            else if (columnId == 2) // Duration column (R2A: display track duration)
            {
                g.setColour(juce::Colours::white); // White for duration
                g.drawText(tracks[trackIndex].getFormattedDuration(), 2, 0, width - 4, height,
                           juce::Justification::centredLeft, true);
            }
        }
    }
}

// R2B: Create load buttons for each deck
juce::Component* PlaylistComponent::refreshComponentForCell(int rowNumber, int columnId,
                                                      bool isRowSelected, 
                                                      juce::Component* existingComponentToUpdate)
{
    if (columnId == 3 || columnId == 4) // Load to Deck 1 or Deck 2 buttons
    {
        auto* button = static_cast<juce::TextButton*>(existingComponentToUpdate);
        
        if (button == nullptr)
        {
            button = new juce::TextButton();
        }
        
        // Determine button state based on loaded/playing status
        juce::String buttonText = "LOAD";
        juce::File currentTrackFile;
        
        {
            const juce::ScopedLock lock(tracksMutex);
            if (rowNumber >= 0 && rowNumber < static_cast<int>(filteredTrackIndices.size()))
            {
                int trackIndex = filteredTrackIndices[rowNumber];
                if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
                {
                    currentTrackFile = tracks[trackIndex].file;
                }
            }
        }
        
        // Check if this track is loaded or playing
        if (columnId == 3) // Deck 1
        {
            if (loadedTrackDeck1 == currentTrackFile)
            {
                buttonText = playingDeck1 ? "PLAYING" : "LOADED";
            }
        }
        else // Deck 2
        {
            if (loadedTrackDeck2 == currentTrackFile)
            {
                buttonText = playingDeck2 ? "PLAYING" : "LOADED";
            }
        }
        
        button->setButtonText(buttonText);
        
        // All buttons white with black text
        button->setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        button->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        
        // Apply 3D glossy effect
        button->setLookAndFeel(&glossyLookAndFeel);
        
        button->onClick = [this, rowNumber, columnId]()
        {
            // Thread-safe access to tracks
            const juce::ScopedLock lock(tracksMutex);
            
            if (rowNumber >= 0 && rowNumber < static_cast<int>(filteredTrackIndices.size()))
            {
                int trackIndex = filteredTrackIndices[rowNumber];
                
                if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
                {
                    // R2B: Load from library to specific deck using DeckGUI's loadTrackWithInfo
                    if (columnId == 3) // Load to Deck 1
                    {
                        if (deckGUI1 != nullptr)
                        {
                            deckGUI1->loadTrackWithInfo(tracks[trackIndex].file);
                            loadedTrackDeck1 = tracks[trackIndex].file;
                            playingDeck1 = false; // Not playing yet, just loaded
                            
                            // Calculate and set BPM for Deck 1
                            double bpm = calculateBPM(tracks[trackIndex].file);
                            waveform1.setBPM(bpm);
                            bpmLabel1.setText("DECK 1", 
                                            juce::dontSendNotification);
                            
                            juce::Logger::writeToLog("Loaded to Deck 1: " + juce::String(tracks[trackIndex].title));
                            tableComponent.updateContent(); // Refresh to show "LOADED" state
                        }
                    }
                    else if (columnId == 4) // Load to Deck 2
                    {
                        if (deckGUI2 != nullptr)
                        {
                            deckGUI2->loadTrackWithInfo(tracks[trackIndex].file);
                            loadedTrackDeck2 = tracks[trackIndex].file;
                            playingDeck2 = false; // Not playing yet, just loaded
                            
                            // Calculate and set BPM for Deck 2
                            double bpm = calculateBPM(tracks[trackIndex].file);
                            waveform2.setBPM(bpm);
                            bpmLabel2.setText("DECK 2", 
                                            juce::dontSendNotification);
                            
                            juce::Logger::writeToLog("Loaded to Deck 2: " + juce::String(tracks[trackIndex].title));
                            tableComponent.updateContent(); // Refresh to show "LOADED" state
                        }
                    }
                }
            }
        };
        
        return button;
    }
    else if (columnId == 5 || columnId == 6) // Unload buttons for Deck 1 and Deck 2
    {
        auto* button = static_cast<juce::TextButton*>(existingComponentToUpdate);
        
        if (button == nullptr)
        {
            button = new juce::TextButton();
        }
        
        button->setButtonText("UNLOAD");
        button->setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        button->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        button->setLookAndFeel(&glossyLookAndFeel);
        
        button->onClick = [this, columnId]()
        {
            if (columnId == 5) // Unload Deck 1
            {
                if (deckGUI1 != nullptr)
                {
                    deckGUI1->unloadTrack();
                    loadedTrackDeck1 = juce::File();
                    playingDeck1 = false;
                    juce::Logger::writeToLog("Unloaded Deck 1");
                    tableComponent.updateContent(); // Refresh display
                }
            }
            else if (columnId == 6) // Unload Deck 2
            {
                if (deckGUI2 != nullptr)
                {
                    deckGUI2->unloadTrack();
                    loadedTrackDeck2 = juce::File();
                    playingDeck2 = false;
                    juce::Logger::writeToLog("Unloaded Deck 2");
                    tableComponent.updateContent(); // Refresh display
                }
            }
        };
        
        return button;
    }
    else if (columnId == 7) // Favorite button
    {
        auto* button = static_cast<juce::TextButton*>(existingComponentToUpdate);
        
        if (button == nullptr)
        {
            button = new juce::TextButton();
        }
        
        // Get the favorite state for this track
        bool isFav = false;
        {
            const juce::ScopedLock lock(tracksMutex);
            if (rowNumber >= 0 && rowNumber < static_cast<int>(filteredTrackIndices.size()))
            {
                int trackIndex = filteredTrackIndices[rowNumber];
                if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
                {
                    isFav = tracks[trackIndex].isFavorite;
                }
            }
        }
        
        button->setButtonText(isFav ? "FAV" : "+");
        button->setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        button->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        button->setLookAndFeel(&glossyLookAndFeel);
        
        button->onClick = [this, rowNumber]()
        {
            // Thread-safe access to tracks
            const juce::ScopedLock lock(tracksMutex);
            
            if (rowNumber >= 0 && rowNumber < static_cast<int>(filteredTrackIndices.size()))
            {
                int trackIndex = filteredTrackIndices[rowNumber];
                
                if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
                {
                    // Toggle favorite status
                    tracks[trackIndex].isFavorite = !tracks[trackIndex].isFavorite;
                    juce::Logger::writeToLog("Favorite toggled: " + juce::String(tracks[trackIndex].title));
                    
                    // Save to persist the change
                    saveLibraryState();
                    
                    // Update the display
                    tableComponent.updateContent();
                }
            }
        };
        
        return button;
    }
    else if (columnId == 8) // Delete button
    {
        auto* button = static_cast<juce::TextButton*>(existingComponentToUpdate);
        
        if (button == nullptr)
        {
            button = new juce::TextButton();
        }
        
        button->setButtonText("DEL");
        button->setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        button->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        button->setLookAndFeel(&glossyLookAndFeel);
        
        button->onClick = [this, rowNumber]()
        {
            // Thread-safe access to tracks
            const juce::ScopedLock lock(tracksMutex);
            
            if (rowNumber >= 0 && rowNumber < static_cast<int>(filteredTrackIndices.size()))
            {
                int trackIndex = filteredTrackIndices[rowNumber];
                
                if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
                {
                    juce::String trackTitle = juce::String(tracks[trackIndex].title);
                    
                    // Remove the track from the library
                    tracks.erase(tracks.begin() + trackIndex);
                    
                    juce::Logger::writeToLog("Deleted track: " + trackTitle);
                    
                    // Save to persist the change
                    saveLibraryState();
                    
                    // Update filtered indices and display
                    updateFilteredTracks();
                    tableComponent.updateContent();
                }
            }
        };
        
        return button;
    }
    
    return nullptr;
}

void PlaylistComponent::buttonClicked(juce::Button* button)
{
    if (button == &addButton)
    {
        // Use async file chooser to prevent UI blocking
        auto chooserFlags = juce::FileBrowserComponent::openMode | 
                          juce::FileBrowserComponent::canSelectFiles |
                          juce::FileBrowserComponent::canSelectMultipleItems;
        
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select audio files to add to library...", 
            juce::File{}, 
            "*.mp3;*.wav;*.aiff;*.flac;*.ogg");
        
        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& chooser)
        {
            auto files = chooser.getResults();
            
            if (files.size() > 0)
            {
                // Process files in background thread to avoid blocking UI
                for (const auto& file : files)
                {
                    threadPool.addJob([this, file]()
                    {
                        // R2A: Add track with full metadata (runs on background thread)
                        addTrackToLibrary(file);
                        
                        // Update UI on message thread
                        juce::MessageManager::callAsync([this]()
                        {
                            updateFilteredTracks();
                            tableComponent.updateContent();
                        });
                    });
                }
                
                // Save library state after all files are processed
                threadPool.addJob([this]()
                {
                    // R2C: Save after adding tracks
                    saveLibraryState();
                });
            }
        });
    }
    else if (button == &favoritesFilterButton) // Toggle favorites filter
    {
        filterMode = favoritesFilterButton.getToggleState() ? 1 : 0;
        favoritesFilterButton.setButtonText(filterMode == 1 ? "SHOW ALL" : "SHOW FAVORITES");
        updateFilteredTracks();
        tableComponent.updateContent();
    }
}

void PlaylistComponent::setDeck1Playing(bool isPlaying)
{
    playingDeck1 = isPlaying;
    tableComponent.updateContent(); // Refresh to show playing state
}

void PlaylistComponent::setDeck2Playing(bool isPlaying)
{
    playingDeck2 = isPlaying;
    tableComponent.updateContent(); // Refresh to show playing state
}

void PlaylistComponent::setDeck1Zoom(int level)
{
    waveform1.setZoomLevel(level);
}

void PlaylistComponent::setDeck2Zoom(int level)
{
    waveform2.setZoomLevel(level);
}

void PlaylistComponent::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    // Search functionality - filter tracks on Enter key
    updateFilteredTracks();
    tableComponent.updateContent();
}

void PlaylistComponent::textEditorTextChanged(juce::TextEditor& editor)
{
    // Real-time search filtering as user types
    updateFilteredTracks();
    tableComponent.updateContent();
}

// Drag and drop support
bool PlaylistComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Check if any of the files are audio files
    for (const auto& file : files)
    {
        juce::File f(file);
        juce::String extension = f.getFileExtension().toLowerCase();
        
        if (extension == ".mp3" || extension == ".wav" || extension == ".aiff" || 
            extension == ".flac" || extension == ".ogg")
        {
            return true;
        }
    }
    return false;
}

void PlaylistComponent::filesDropped(const juce::StringArray& files, int x, int y)
{
    // Add all dropped audio files to library
    for (const auto& filePath : files)
    {
        juce::File file(filePath);
        
        threadPool.addJob([this, file]()
        {
            addTrackToLibrary(file);
            
            // Update UI on message thread
            juce::MessageManager::callAsync([this]()
            {
                updateFilteredTracks();
                tableComponent.updateContent();
            });
        });
    }
    
    // Save library state after all files are processed
    threadPool.addJob([this]()
    {
        saveLibraryState();
    });
}

// Update filtered tracks based on search query and filter mode
void PlaylistComponent::updateFilteredTracks()
{
    const juce::ScopedLock lock(tracksMutex);
    
    filteredTrackIndices.clear();
    juce::String searchQuery = searchBox.getText().toLowerCase();
    
    for (int i = 0; i < static_cast<int>(tracks.size()); ++i)
    {
        const auto& track = tracks[i];
        
        // Apply favorites filter
        if (filterMode == 1 && !track.isFavorite)
            continue;
        
        // Apply search filter
        if (searchQuery.isNotEmpty() && !matchesSearchQuery(track, searchQuery))
            continue;
        
        filteredTrackIndices.push_back(i);
    }
}

// Check if track matches search query
bool PlaylistComponent::matchesSearchQuery(const TrackInfo& track, const juce::String& query)
{
    juce::String title = juce::String(track.title).toLowerCase();
    juce::String duration = juce::String(track.duration).toLowerCase();
    
    return title.contains(query) || duration.contains(query);
}

//==============================================================================
// R2A: Extract track duration from audio file
std::string PlaylistComponent::getTrackDuration(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    
    if (reader != nullptr)
    {
        double lengthInSeconds = reader->lengthInSamples / reader->sampleRate;
        int minutes = static_cast<int>(lengthInSeconds / 60.0);
        int seconds = static_cast<int>(lengthInSeconds) % 60;
        
        delete reader;
        
        // Format as MM:SS
        return juce::String(minutes).paddedLeft('0', 2).toStdString() + ":" + 
               juce::String(seconds).paddedLeft('0', 2).toStdString();
    }
    
    return "00:00";
}

// R2A: Add track with metadata to library
void PlaylistComponent::addTrackToLibrary(const juce::File& file)
{
    // Check if track already exists (with thread-safe read)
    {
        const juce::ScopedLock lock(tracksMutex);
        for (const auto& track : tracks)
        {
            if (track.file == file)
            {
                juce::Logger::writeToLog("Track already in library: " + file.getFileName());
                return;
            }
        }
    }
    
    // Create track info (this is done outside the lock to avoid blocking other threads)
    TrackInfo newTrack;
    newTrack.title = file.getFileNameWithoutExtension().toStdString();
    newTrack.file = file;
    newTrack.duration = getTrackDuration(file);
    
    // Extract seconds for sorting
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        newTrack.lengthInSeconds = reader->lengthInSamples / reader->sampleRate;
        delete reader;
    }
    
    // Add to tracks vector with thread-safe write
    {
        const juce::ScopedLock lock(tracksMutex);
        tracks.push_back(newTrack);
    }
    
    juce::Logger::writeToLog("Added to library: " + juce::String(newTrack.title) + " (" + juce::String(newTrack.duration) + ")");
}

void PlaylistComponent::saveLibraryState()
{
    juce::File libraryFile = getLibraryFilePath();
    
    // Create parent directory if it doesn't exist
    libraryFile.getParentDirectory().createDirectory();
    
    // Write library data
    std::ofstream outFile(libraryFile.getFullPathName().toStdString());
    
    if (outFile.is_open())
    {
        // Thread-safe read of tracks
        const juce::ScopedLock lock(tracksMutex);
        
        outFile << tracks.size() << "\n";
        
        for (const auto& track : tracks)
        {
            outFile << track.file.getFullPathName().toStdString() << "\n";
            outFile << track.title << "\n";
            outFile << track.duration << "\n";
            outFile << track.lengthInSeconds << "\n";
            outFile << (track.isFavorite ? 1 : 0) << "\n"; // Save favorite status
        }
        
        outFile.close();
        juce::Logger::writeToLog("Library saved: " + juce::String(tracks.size()) + " tracks");
    }
    else
    {
        juce::Logger::writeToLog("Failed to save library");
    }
}

void PlaylistComponent::loadLibraryState()
{
    juce::File libraryFile = getLibraryFilePath();
    
    if (!libraryFile.existsAsFile())
    {
        juce::Logger::writeToLog("No saved library found");
        return;
    }
    
    std::ifstream inFile(libraryFile.getFullPathName().toStdString());
    
    if (inFile.is_open())
    {
        int numTracks = 0;
        inFile >> numTracks;
        inFile.ignore(); // Skip newline
        
        // Thread-safe write to tracks
        const juce::ScopedLock lock(tracksMutex);
        tracks.clear();
        
        for (int i = 0; i < numTracks; ++i)
        {
            TrackInfo track;
            std::string filePath;
            
            std::getline(inFile, filePath);
            std::getline(inFile, track.title);
            std::getline(inFile, track.duration);
            inFile >> track.lengthInSeconds;
            inFile.ignore(); // Skip newline
            
            // Load favorite status (default to false if not present for backwards compatibility)
            int favStatus = 0;
            if (inFile >> favStatus)
            {
                track.isFavorite = (favStatus != 0);
                inFile.ignore(); // Skip newline
            }
            else
            {
                track.isFavorite = false;
            }
            
            track.file = juce::File(filePath);
            
            // Only add if file still exists
            if (track.file.existsAsFile())
            {
                tracks.push_back(track);
            }
        }
        
        inFile.close();
        juce::Logger::writeToLog("Library loaded: " + juce::String(tracks.size()) + " tracks");
    }
    else
    {
        juce::Logger::writeToLog("Failed to load library");
    }
}

// Get path to library data file
juce::File PlaylistComponent::getLibraryFilePath()
{
    // Save in user's app data directory
    juce::File appData = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    
    return appData.getChildFile("Otodecks").getChildFile("library.dat");
}

// Timer callback to periodically check and update playing status
void PlaylistComponent::timerCallback()
{
    // Check if playing states have changed
    bool needsUpdate = false;
    
    if (player1 != nullptr && player1->isPlaying() != playingDeck1)
    {
        playingDeck1 = player1->isPlaying();
        needsUpdate = true;
    }
    
    if (player2 != nullptr && player2->isPlaying() != playingDeck2)
    {
        playingDeck2 = player2->isPlaying();
        needsUpdate = true;
    }
    
    // Only update table if status changed
    if (needsUpdate)
    {
        tableComponent.updateContent();
    }
    
    // Update waveforms continuously
    waveform1.updateWaveform();
    waveform1.repaint();
    waveform2.updateWaveform();
    waveform2.repaint();
}

//==============================================================================
// Sync waveforms when SYNC button is pressed
void PlaylistComponent::syncWaveforms(bool synced)
{
    waveformsSynced = synced;
    waveform1.setSynced(synced);
    waveform2.setSynced(synced);
    waveform1.repaint();
    waveform2.repaint();
}

// Calculate BPM from audio file (simplified version)
double PlaylistComponent::calculateBPM(const juce::File& file)
{
    if (!file.existsAsFile())
        return 0.0;
    
    // Create a reader for the file
    auto inputStream = file.createInputStream();
    if (inputStream == nullptr)
        return 0.0;
        
    auto* reader = formatManager.createReaderFor(std::move(inputStream));
    if (reader == nullptr)
        return 0.0;
        
    // Simplified BPM calculation based on file length
    // In a real implementation, this would analyze the audio data
    // using FFT and beat detection algorithms
    
    // Estimate BPM (typical range 80-180 BPM)
    // This is a placeholder - real BPM detection requires complex DSP
    double estimatedBPM = 120.0 + (std::rand() % 60 - 30); // Random 90-150 for demo
    
    delete reader;
    return estimatedBPM;
}

//==============================================================================
// WaveformDisplay Implementation

PlaylistComponent::WaveformDisplay::WaveformDisplay(DJAudioPlayer* player, int deckNum)
    : audioPlayer(player), deckNumber(deckNum)
{
    // Initialize waveform data
    waveformData.resize(200, 0.0f);
    beatMarkers.resize(200, false);
}

void PlaylistComponent::WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto fullBounds = bounds; // Save original bounds for UI elements
    
    // Background
    g.fillAll(juce::Colours::black);
    
    // Grid lines
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
    for (int i = 0; i < 5; ++i)
    {
        int y = bounds.getHeight() * i / 4;
        g.drawLine(0, y, bounds.getWidth(), y, 0.5f);
    }
    
    // Draw waveform
    if (isSynced)
    {
        // When synced, draw combined waveform effect in WHITE
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("SYNCED", bounds, juce::Justification::centred);
    }
    
    // Draw waveform with pink-white gradient
    juce::Colour waveformPink = juce::Colour(0xffff1493);
    juce::Colour waveformWhite = juce::Colours::white;
    juce::Colour beatHighlight = juce::Colour(0xffffff00); // Yellow for beats
    
    float midY = bounds.getHeight() / 2.0f;
    
    // Adjust scale based on zoom level - higher zoom = more detail
    float scaleY = bounds.getHeight() * 0.45f;
    if (zoomLevel == 2)
        scaleY *= 1.3f; // 30% taller bars for medium zoom
    else if (zoomLevel == 3)
        scaleY *= 1.6f; // 60% taller bars for close zoom
    
    // Get current audio level for visualization
    float currentLevel = 0.0f;
    if (audioPlayer != nullptr)
    {
        currentLevel = audioPlayer->getAudioLevel();
    }
    
    // Detect beat (simple onset detection via energy increase)
    float energyDelta = currentLevel - previousLevel;
    bool isBeat = (energyDelta > 0.15f) && (currentLevel > 0.4f);
    previousLevel = currentLevel;
    
    // Draw waveform bars - bar width changes with zoom level
    float barWidth = bounds.getWidth() / static_cast<float>(waveformData.size());
    
    // Adjust bar width/spacing based on zoom for visual feedback
    float barSpacing = 1.0f;
    if (zoomLevel == 2)
        barSpacing = 1.5f; // More spacing at medium zoom
    else if (zoomLevel == 3)
        barSpacing = 2.0f; // Even more spacing at close zoom
    
    for (int i = 0; i < waveformData.size(); ++i)
    {
        // Shift waveform data left
        if (i < waveformData.size() - 1)
        {
            waveformData[i] = waveformData[i + 1];
            beatMarkers[i] = beatMarkers[i + 1];
        }
        
        // Add new audio level at the end
        if (i == waveformData.size() - 1)
        {
            waveformData[i] = currentLevel;
            beatMarkers[i] = isBeat;
        }
        
        float x = i * barWidth;
        float barHeight = waveformData[i] * scaleY;
        
        // Color: beat markers are brighter (yellow-white), normal bars are gradient
        juce::Colour barColor;
        if (beatMarkers[i])
        {
            barColor = beatHighlight; // Yellow for detected beats
        }
        else
        {
            // Gradient from white (high) to pink (low)
            float intensity = waveformData[i];
            barColor = waveformWhite.interpolatedWith(waveformPink, 1.0f - intensity);
        }
        
        // Draw bar (from center, mirrored) - width affected by zoom
        float effectiveBarWidth = barWidth - barSpacing;
        g.setColour(barColor.withAlpha(0.8f));
        g.fillRect(x, midY - barHeight, effectiveBarWidth, barHeight * 2);
        
        // Glow effect on beats - more pronounced at higher zoom
        if (beatMarkers[i])
        {
            float glowIntensity = 0.4f + (zoomLevel * 0.1f); // Brighter at higher zoom
            g.setColour(beatHighlight.withAlpha(glowIntensity));
            g.fillRect(x - 1, midY - barHeight * 1.2f, effectiveBarWidth + 2, barHeight * 2.4f);
        }
    }
    
    // BPM value display removed per user request
    // (BPM is still calculated internally for sync purposes)
    
    // Draw ZOOM level at top left with prominent background and larger text
    juce::Colour zoomColor;
    if (zoomLevel == 1)
        zoomColor = juce::Colour(0xff00d4ff); // Cyan
    else if (zoomLevel == 2)
        zoomColor = juce::Colour(0xff00ff00); // Green
    else
        zoomColor = juce::Colour(0xffffa500); // Orange
    
    auto zoomArea = getLocalBounds().removeFromTop(30).removeFromLeft(120).reduced(5);
    // Draw semi-transparent background box
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillRoundedRectangle(zoomArea.toFloat(), 4.0f);
    // Draw border with zoom color
    g.setColour(zoomColor);
    g.drawRoundedRectangle(zoomArea.toFloat(), 4.0f, 2.0f);
    // Draw text
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    juce::String zoomText = "ZOOM " + juce::String(zoomLevel) + "x";
    g.drawText(zoomText, zoomArea, juce::Justification::centred);
}

void PlaylistComponent::WaveformDisplay::updateWaveform()
{
    repaint();
}

void PlaylistComponent::WaveformDisplay::setSynced(bool synced)
{
    isSynced = synced;
}

void PlaylistComponent::WaveformDisplay::setBPM(double bpm)
{
    currentBPM = bpm;
}

void PlaylistComponent::WaveformDisplay::setZoomLevel(int level)
{
    juce::Logger::writeToLog("WaveformDisplay::setZoomLevel - Deck " + juce::String(deckNumber) + " set to: " + juce::String(level));
    zoomLevel = level;
    repaint(); // Trigger redraw to show zoom level
}

//==============================================================================
// TrackInfo implementation
juce::String PlaylistComponent::TrackInfo::getFormattedDuration() const
{
    // Duration is already formatted as MM:SS string
    return juce::String(duration);
}
