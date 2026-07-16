#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/Engine.h"

namespace harmonic_engine
{

class HarmonicMenuBar : public juce::MenuBarModel
{
public:
    HarmonicMenuBar(Engine& engine);
    ~HarmonicMenuBar() override;

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex,
                                    const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    juce::MenuBarComponent& getComponent() { return menuBarComponent; }

    void setMetronomeState(bool on);
    void setCountInState(bool on);
    void setInputMonitorState(bool on);
    void setOverdubState(bool overdub);
    void setPlayingState(bool playing);
    void setRecordingState(bool recording);

    std::function<void()> onNewProject;
    std::function<void()> onOpenProject;
    std::function<void()> onSaveProject;
    std::function<void()> onSaveProjectAs;
    std::function<void()> onExportAudio;
    std::function<void()> onImportAudio;
    std::function<void()> onUndo;
    std::function<void()> onRedo;
    std::function<void()> onCut;
    std::function<void()> onCopy;
    std::function<void()> onPaste;
    std::function<void()> onDelete;
    std::function<void()> onSelectAll;
    std::function<void()> onAddAudioTrack;
    std::function<void()> onAddMidiTrack;
    std::function<void()> onDuplicateTrack;
    std::function<void()> onDeleteTrack;
    std::function<void()> onRenameTrack;
    std::function<void()> onMuteSelected;
    std::function<void()> onSoloSelected;
    std::function<void()> onFreezeTrack;
    std::function<void()> onPlay;
    std::function<void()> onStop;
    std::function<void()> onPause;
    std::function<void()> onRecord;
    std::function<void()> onSkipToStart;
    std::function<void()> onSkipToEnd;
    std::function<void()> onArmSelectedTrack;
    std::function<void()> onDisarmAllTracks;
    std::function<void()> onToggleMetronome;
    std::function<void()> onToggleCountIn;
    std::function<void()> onToggleInputMonitoring;
    std::function<void()> onRecordOverdub;
    std::function<void()> onRecordReplace;
    std::function<void()> onZoomIn;
    std::function<void()> onZoomOut;
    std::function<void()> onZoomToFit;
    std::function<void()> onToggleMixer;
    std::function<void()> onToggleTimeline;
    std::function<void()> onShowTransportBar;
    std::function<void()> onAudioSettings;

private:
    Engine& audioEngine;
    juce::MenuBarComponent menuBarComponent;

    bool metronomeOn = true;
    bool countInOn = false;
    bool inputMonOn = false;
    bool overdubMode = true;
    bool playingState = false;
    bool recordingState = false;

    enum MenuIDs
    {
        newProject = 1,
        openProject,
        saveProject,
        saveProjectAs,
        exportAudio,
        importAudio,
        undoAction,
        redoAction,
        cutAction,
        copyAction,
        pasteAction,
        deleteAction,
        selectAllAction,
        addAudioTrack,
        addMidiTrack,
        duplicateTrack,
        deleteTrack,
        renameTrack,
        muteSelected,
        soloSelected,
        freezeTrack,
        playAction,
        stopAction,
        pauseAction,
        recordAction,
        skipToStart,
        skipToEnd,
        armSelectedTrack,
        disarmAllTracks,
        toggleMetronome,
        toggleCountIn,
        toggleInputMonitoring,
        recordOverdub,
        recordReplace,
        zoomIn,
        zoomOut,
        zoomToFit,
        toggleMixer,
        toggleTimeline,
        showTransportBar,
        audioSettings
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonicMenuBar)
};

} // namespace harmonic_engine
