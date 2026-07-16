#include "harmonic_engine/GUI/MenuBar.h"

namespace harmonic_engine
{

HarmonicMenuBar::HarmonicMenuBar(Engine& engine)
    : audioEngine(engine),
      menuBarComponent(this)
{
}

HarmonicMenuBar::~HarmonicMenuBar()
{
}

juce::StringArray HarmonicMenuBar::getMenuBarNames()
{
    return { "File", "Edit", "Track", "Transport", "Record", "View" };
}

juce::PopupMenu HarmonicMenuBar::getMenuForIndex(int topLevelMenuIndex,
                                                  const juce::String& menuName)
{
    juce::PopupMenu menu;
    juce::ignoreUnused(menuName);

    if (topLevelMenuIndex == 0) // File
    {
        menu.addItem(newProject,      "New Project           Cmd+N");
        menu.addSeparator();
        menu.addItem(openProject,     "Open Project...       Cmd+O");
        menu.addSeparator();
        menu.addItem(saveProject,     "Save Project          Cmd+S");
        menu.addItem(saveProjectAs,   "Save Project As...    Cmd+Shift+S");
        menu.addSeparator();
        menu.addItem(importAudio,     "Import Audio...       Cmd+I");
        menu.addItem(exportAudio,     "Export Audio...       Cmd+E");
        menu.addSeparator();
        menu.addItem(audioSettings,   "Audio Settings...     Cmd+,");
    }
    else if (topLevelMenuIndex == 1) // Edit
    {
        menu.addItem(undoAction,      "Undo                  Cmd+Z");
        menu.addItem(redoAction,      "Redo                  Cmd+Shift+Z");
        menu.addSeparator();
        menu.addItem(cutAction,       "Cut                   Cmd+X");
        menu.addItem(copyAction,      "Copy                  Cmd+C");
        menu.addItem(pasteAction,     "Paste                 Cmd+V");
        menu.addItem(deleteAction,    "Delete");
        menu.addSeparator();
        menu.addItem(selectAllAction, "Select All Clips      Cmd+A");
    }
    else if (topLevelMenuIndex == 2) // Track
    {
        menu.addItem(addAudioTrack,   "Add Audio Track       Cmd+T");
        menu.addItem(addMidiTrack,    "Add MIDI Track        Cmd+Shift+T");
        menu.addItem(duplicateTrack,  "Duplicate Track");
        menu.addSeparator();
        menu.addItem(deleteTrack,     "Delete Track");
        menu.addItem(renameTrack,     "Rename Track...");
        menu.addSeparator();
        menu.addItem(muteSelected,    "Mute Selected Track   M");
        menu.addItem(soloSelected,    "Solo Selected Track   S");
        menu.addSeparator();
        menu.addItem(freezeTrack,     "Freeze Track");
    }
    else if (topLevelMenuIndex == 3) // Transport
    {
        menu.addItem(playAction,      "Play                  Space");
        menu.addItem(pauseAction,     "Pause                 P");
        menu.addItem(stopAction,      "Stop                  .");
        menu.addSeparator();
        menu.addItem(recordAction,    "Record                R");
        menu.addSeparator();
        menu.addItem(skipToStart,     "Skip to Start");
        menu.addItem(skipToEnd,       "Skip to End");
    }
    else if (topLevelMenuIndex == 4) // Record
    {
        menu.addItem(armSelectedTrack,   "Arm Selected Track    A");
        menu.addItem(disarmAllTracks,    "Disarm All Tracks");
        menu.addSeparator();
        menu.addItem(recordOverdub,      "Overdub Mode", true, overdubMode);
        menu.addItem(recordReplace,      "Replace Mode", true, !overdubMode);
        menu.addSeparator();
        menu.addItem(toggleMetronome,    "Metronome             Cmd+M", true, metronomeOn);
        menu.addItem(toggleCountIn,      "Count-In (1 Bar)      C", true, countInOn);
        menu.addItem(toggleInputMonitoring, "Input Monitoring    I", true, inputMonOn);
    }
    else if (topLevelMenuIndex == 5) // View
    {
        menu.addItem(zoomIn,          "Zoom In               Cmd++");
        menu.addItem(zoomOut,         "Zoom Out              Cmd+-");
        menu.addItem(zoomToFit,       "Zoom to Fit           Cmd+0");
        menu.addSeparator();
        menu.addItem(toggleMixer,     "Toggle Mixer Panel");
        menu.addItem(toggleTimeline,  "Toggle Timeline Panel");
        menu.addItem(showTransportBar,"Toggle Transport Bar");
    }

    return menu;
}

void HarmonicMenuBar::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    juce::ignoreUnused(topLevelMenuIndex);

    switch (menuItemID)
    {
        case newProject:
            if (onNewProject) onNewProject();
            break;
        case openProject:
            if (onOpenProject) onOpenProject();
            break;
        case saveProject:
            if (onSaveProject) onSaveProject();
            break;
        case saveProjectAs:
            if (onSaveProjectAs) onSaveProjectAs();
            break;
        case importAudio:
            if (onImportAudio) onImportAudio();
            break;
        case exportAudio:
            if (onExportAudio) onExportAudio();
            break;
        case audioSettings:
            if (onAudioSettings) onAudioSettings();
            break;
        case undoAction:
            if (onUndo) onUndo();
            break;
        case redoAction:
            if (onRedo) onRedo();
            break;
        case cutAction:
            if (onCut) onCut();
            break;
        case copyAction:
            if (onCopy) onCopy();
            break;
        case pasteAction:
            if (onPaste) onPaste();
            break;
        case deleteAction:
            if (onDelete) onDelete();
            break;
        case selectAllAction:
            if (onSelectAll) onSelectAll();
            break;
        case addAudioTrack:
            if (onAddAudioTrack) onAddAudioTrack();
            break;
        case addMidiTrack:
            if (onAddMidiTrack) onAddMidiTrack();
            break;
        case duplicateTrack:
            if (onDuplicateTrack) onDuplicateTrack();
            break;
        case deleteTrack:
            if (onDeleteTrack) onDeleteTrack();
            break;
        case renameTrack:
            if (onRenameTrack) onRenameTrack();
            break;
        case muteSelected:
            if (onMuteSelected) onMuteSelected();
            break;
        case soloSelected:
            if (onSoloSelected) onSoloSelected();
            break;
        case freezeTrack:
            if (onFreezeTrack) onFreezeTrack();
            break;
        case playAction:
            if (onPlay) onPlay();
            break;
        case stopAction:
            if (onStop) onStop();
            break;
        case pauseAction:
            if (onPause) onPause();
            break;
        case recordAction:
            if (onRecord) onRecord();
            break;
        case skipToStart:
            if (onSkipToStart) onSkipToStart();
            break;
        case skipToEnd:
            if (onSkipToEnd) onSkipToEnd();
            break;
        case armSelectedTrack:
            if (onArmSelectedTrack) onArmSelectedTrack();
            break;
        case disarmAllTracks:
            if (onDisarmAllTracks) onDisarmAllTracks();
            break;
        case toggleMetronome:
            if (onToggleMetronome) onToggleMetronome();
            break;
        case toggleCountIn:
            if (onToggleCountIn) onToggleCountIn();
            break;
        case toggleInputMonitoring:
            if (onToggleInputMonitoring) onToggleInputMonitoring();
            break;
        case recordOverdub:
            if (onRecordOverdub) onRecordOverdub();
            break;
        case recordReplace:
            if (onRecordReplace) onRecordReplace();
            break;
        case zoomIn:
            if (onZoomIn) onZoomIn();
            break;
        case zoomOut:
            if (onZoomOut) onZoomOut();
            break;
        case zoomToFit:
            if (onZoomToFit) onZoomToFit();
            break;
        case toggleMixer:
            if (onToggleMixer) onToggleMixer();
            break;
        case toggleTimeline:
            if (onToggleTimeline) onToggleTimeline();
            break;
        case showTransportBar:
            if (onShowTransportBar) onShowTransportBar();
            break;
        default:
            break;
    }
}

void HarmonicMenuBar::setMetronomeState(bool on) { metronomeOn = on; }
void HarmonicMenuBar::setCountInState(bool on) { countInOn = on; }
void HarmonicMenuBar::setInputMonitorState(bool on) { inputMonOn = on; }
void HarmonicMenuBar::setOverdubState(bool overdub) { overdubMode = overdub; }
void HarmonicMenuBar::setPlayingState(bool playing) { playingState = playing; }
void HarmonicMenuBar::setRecordingState(bool recording) { recordingState = recording; }

} // namespace harmonic_engine
