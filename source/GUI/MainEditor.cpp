#include "harmonic_engine/GUI/MainEditor.h"

namespace harmonic_engine
{

MainEditor::MainEditor(Engine& engine)
    : audioEngine(engine),
      menuBar(engine),
      transportBar(engine.getTransport()),
      globalTracks(engine.getTransport()),
      timelineView(engine.getTransport(), engine.getTrackManager(), engine.getFileLoader()),
      mixerView(engine.getTrackManager()),
      pianoRoll(engine.getTransport()),
      stepSequencer(engine.getTransport(), engine.getDrumSequencer()),
      statusBar(engine),
      virtualKeyboard(engine)
{
    setLookAndFeel(&lookAndFeel);
    addKeyListener(this);

    // ── Add all child components ────────────────────────────
    addAndMakeVisible(menuBar.getComponent());
    addAndMakeVisible(transportBar);
    addAndMakeVisible(browserPanel);
    addAndMakeVisible(inspectorPanel);
    addAndMakeVisible(globalTracks);
    addAndMakeVisible(timelineView);
    addAndMakeVisible(bottomWorkspace);
    addAndMakeVisible(statusBar);
    addAndMakeVisible(virtualKeyboard);

    // ── Wire bottom workspace tabs ──────────────────────────
    bottomWorkspace.setMixerComponent(&mixerView);
    bottomWorkspace.setPianoRollComponent(&pianoRoll);
    bottomWorkspace.setStepSequencerComponent(&stepSequencer);
    // Audio Editor and Automation are placeholders for now

    // ── Wire callbacks ──────────────────────────────────────
    setupCallbacks();
    syncTransportState();

    setSize(1400, 900);
    startTimerHz(30);
}

MainEditor::~MainEditor()
{
    removeKeyListener(this);
    stopTimer();
    setLookAndFeel(nullptr);
}

// ── Layout ──────────────────────────────────────────────────

void MainEditor::paint(juce::Graphics& g)
{
    g.fillAll(Tokens::Colours::bgDarkest());
}

void MainEditor::resized()
{
    auto bounds = getLocalBounds();

    // 1. Menu bar — full width, fixed height
    menuBar.getComponent().setBounds(bounds.removeFromTop(Tokens::kMenuBarHeight));

    // 2. Transport bar — full width, fixed height
    transportBar.setBounds(bounds.removeFromTop(Tokens::kTransportHeight));

    // 3. Virtual keyboard — full width, fixed height (from bottom)
    const int keyboardHeight = 100;
    virtualKeyboard.setBounds(bounds.removeFromBottom(keyboardHeight));

    // 4. Status bar — full width, fixed height (from bottom)
    statusBar.setBounds(bounds.removeFromBottom(Tokens::kStatusBarHeight));

    // 4. Bottom workspace — full width, variable height (from bottom)
    if (bottomVisible)
    {
        bounds.removeFromBottom(Tokens::kSpace2);
        bottomWorkspace.setBounds(bounds.removeFromBottom(bottomHeight));
    }

    // 5. Browser — left side, full remaining height
    if (browserVisible)
    {
        browserPanel.setBounds(bounds.removeFromLeft(browserWidth));
        bounds.removeFromLeft(Tokens::kSpace2); // resizer gap
    }
    else
    {
        browserPanel.setVisible(false);
    }

    // 6. Inspector — right side, full remaining height
    if (inspectorVisible)
    {
        bounds.removeFromRight(Tokens::kSpace2);
        inspectorPanel.setBounds(bounds.removeFromRight(inspectorWidth));
    }
    else
    {
        inspectorPanel.setVisible(false);
    }

    // 7. Centre: Global tracks on top, timeline fills rest
    if (globalTracks.isVisible())
    {
        globalTracks.setBounds(bounds.removeFromTop(Tokens::kGlobalTracksHeight));
    }

    timelineView.setBounds(bounds);
}

// ── Keyboard shortcuts ──────────────────────────────────────

bool MainEditor::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (key == juce::KeyPress::spaceKey)
    {
        if (audioEngine.getTransport().isPlaying())
            audioEngine.getTransport().stop();
        else
            audioEngine.getTransport().start();
        return true;
    }
    if (key == juce::KeyPress('r'))
    {
        if (audioEngine.isRecording())
            audioEngine.stopRecording();
        else
            audioEngine.startRecording();
        return true;
    }
    if (key == juce::KeyPress('.'))
    {
        audioEngine.getTransport().stop();
        return true;
    }
    if (key == juce::KeyPress('p'))
    {
        audioEngine.getTransport().pause();
        return true;
    }
    if (key == juce::KeyPress('a'))
    {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr) track->setRecordArmed(!track->isRecordArmed());
        return true;
    }
    if (key == juce::KeyPress('l'))
    {
        loopEnabled = !loopEnabled;
        audioEngine.getTransport().setLooping(loopEnabled);
        transportBar.setLooping(loopEnabled);
        return true;
    }
    if (key == juce::KeyPress('r', juce::ModifierKeys::shiftModifier, 0))
    {
        loopRecordEnabled = !loopRecordEnabled;
        audioEngine.setLoopRecordEnabled(loopRecordEnabled);
        transportBar.setLoopRecording(loopRecordEnabled);
        if (loopRecordEnabled)
        {
            loopEnabled = true;
            audioEngine.getTransport().setLooping(true);
            transportBar.setLooping(true);
        }
        return true;
    }
    if (key == juce::KeyPress('m'))
    {
        metronomeEnabled = !metronomeEnabled;
        audioEngine.setMetronomeEnabled(metronomeEnabled);
        transportBar.setMetronomeOn(metronomeEnabled);
        return true;
    }
    // F1 = toggle browser, F2 = toggle inspector, F3 = toggle bottom
    if (key == juce::KeyPress::F1Key)
    {
        browserVisible = !browserVisible;
        browserPanel.setVisible(browserVisible);
        resized();
        return true;
    }
    if (key == juce::KeyPress::F2Key)
    {
        inspectorVisible = !inspectorVisible;
        inspectorPanel.setVisible(inspectorVisible);
        resized();
        return true;
    }
    if (key == juce::KeyPress::F3Key)
    {
        bottomVisible = !bottomVisible;
        bottomWorkspace.setVisible(bottomVisible);
        resized();
        return true;
    }
    return false;
}

Engine& MainEditor::getEngine()
{
    return audioEngine;
}

// ── Timer ───────────────────────────────────────────────────

void MainEditor::timerCallback()
{
    transportBar.updateDisplay();
    mixerView.updateMeters();
    timelineView.updatePlayhead();
    globalTracks.repaint();
}

// ── Callback wiring ─────────────────────────────────────────

void MainEditor::setupCallbacks()
{
    // ── Transport bar callbacks ─────────────────────────────
    transportBar.onRewind = [this]() {
        audioEngine.getTransport().setPositionInSeconds(0.0);
    };
    transportBar.onFastForward = [this]() {
        audioEngine.getTransport().setPositionInSeconds(
            audioEngine.getProjectLengthInSeconds());
    };
    transportBar.onPlay = [this]() {
        audioEngine.getTransport().start();
    };
    transportBar.onStop = [this]() {
        audioEngine.getTransport().stop();
    };
    transportBar.onPause = [this]() {
        audioEngine.getTransport().pause();
    };
    transportBar.onRecord = [this]() {
        if (audioEngine.isRecording())
            audioEngine.stopRecording();
        else
            audioEngine.startRecording();
    };
    transportBar.onToggleLoop = [this]() {
        loopEnabled = !loopEnabled;
        audioEngine.getTransport().setLooping(loopEnabled);
        transportBar.setLooping(loopEnabled);
    };
    transportBar.onToggleLoopRecord = [this]() {
        loopRecordEnabled = !loopRecordEnabled;
        audioEngine.setLoopRecordEnabled(loopRecordEnabled);
        transportBar.setLoopRecording(loopRecordEnabled);
        if (loopRecordEnabled)
        {
            loopEnabled = true;
            audioEngine.getTransport().setLooping(true);
            transportBar.setLooping(true);
        }
    };
    transportBar.onToggleMetronome = [this]() {
        metronomeEnabled = !metronomeEnabled;
        audioEngine.setMetronomeEnabled(metronomeEnabled);
        transportBar.setMetronomeOn(metronomeEnabled);
    };
    transportBar.onZoomChanged = [this](double pps) {
        timelineView.setPixelsPerSecond(pps);
    };

    // ── Timeline zoom → transport zoom sync ─────────────────
    timelineView.onZoomChanged = [this](double pps) {
        transportBar.setZoomValue(pps);
    };

    // ── Track selection → inspector update ──────────────────
    audioEngine.getTrackManager().onTrackSelectionChanged = [this](int) {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        inspectorPanel.setTrack(track);
        if (track != nullptr && track->isMidiTrack() && track->getNumMidiClips() > 0)
            pianoRoll.setMidiClip(track->getMidiClip(0).get());
        else
            pianoRoll.setMidiClip(nullptr);
        if (track != nullptr && track->isMidiTrack() && track->getNumMidiClips() > 0)
            stepSequencer.setMidiClip(track->getMidiClip(0).get());
        else
            stepSequencer.setMidiClip(nullptr);
    };

    // ── Track changes → rebuild views ──────────────────────
    audioEngine.getTrackManager().onTracksChanged = [this]() {
        timelineView.rebuildClipComponents();
    };

    // ── Menu bar callbacks ──────────────────────────────────
    menuBar.onImportAudio = [this]() {
        juce::FileChooser chooser("Import Audio File",
                                  juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                  "*.wav;*.aiff;*.flac;*.ogg;*.mp3");
        chooser.launchAsync(juce::FileBrowserComponent::openMode |
                            juce::FileBrowserComponent::canSelectFiles,
                            [this](const juce::FileChooser& fc) {
                                auto file = fc.getResult();
                                if (file.existsAsFile())
                                {
                                    auto* track = audioEngine.getTrackManager().getSelectedTrack();
                                    if (track != nullptr)
                                    {
                                        auto reader = audioEngine.getFileLoader().createReaderFor(file);
                                        if (reader != nullptr)
                                        {
                                            double duration = static_cast<double>(reader->lengthInSamples) /
                                                              reader->sampleRate;
                                            auto clip = std::make_shared<AudioClip>(
                                                file.getFileNameWithoutExtension(),
                                                file, 0.0, 0.0, duration, reader->sampleRate);
                                            track->addClip(std::move(clip));
                                            timelineView.rebuildClipComponents();
                                        }
                                    }
                                }
                            });
    };

    menuBar.onExportAudio = [this]() {
        juce::FileChooser chooser("Export Audio File",
                                  juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                  "*.wav");
        chooser.launchAsync(juce::FileBrowserComponent::saveMode |
                            juce::FileBrowserComponent::canSelectFiles,
                            [this](const juce::FileChooser& fc) {
                                auto file = fc.getResult();
                                if (file != juce::File{})
                                {
                                    audioEngine.exportAudio(file, audioEngine.getSampleRate(), 24);
                                    timelineView.rebuildClipComponents();
                                }
                            });
    };

    menuBar.onNewProject = [this]() {
        audioEngine.getTrackManager().clearAllTracks();
        audioEngine.getTrackManager().addTrack();
        audioEngine.getTransport().stop();
        audioEngine.getTransport().setPositionInSeconds(0.0);
        timelineView.rebuildClipComponents();
    };

    menuBar.onAddAudioTrack = [this]() {
        audioEngine.getTrackManager().addTrack();
        timelineView.rebuildClipComponents();
    };

    menuBar.onAddMidiTrack = [this]() {
        auto* track = audioEngine.getTrackManager().addTrack("MIDI Track");
        track->setMidiTrack(true);
        track->setInstrument(InstrumentType::Sine);
        timelineView.rebuildClipComponents();
    };

    menuBar.onDuplicateTrack = [this]() {
        auto* selected = audioEngine.getTrackManager().getSelectedTrack();
        if (selected != nullptr)
        {
            auto* newTrack = audioEngine.getTrackManager().addTrack(selected->getName() + " (Copy)");
            newTrack->setGain(selected->getGain());
            newTrack->setPan(selected->getPan());
            newTrack->setMuted(selected->isMuted());
            newTrack->setSoloed(selected->isSoloed());
            timelineView.rebuildClipComponents();
        }
    };

    menuBar.onDeleteTrack = [this]() {
        auto& tm = audioEngine.getTrackManager();
        if (tm.getNumTracks() > 1)
        {
            int idx = tm.getSelectedTrackIndex();
            auto* track = tm.getSelectedTrack();
            if (track != nullptr)
            {
                tm.removeTrack(track->getTrackNumber());
                if (idx >= tm.getNumTracks()) tm.setSelectedTrackIndex(tm.getNumTracks() - 1);
                timelineView.rebuildClipComponents();
            }
        }
    };

    menuBar.onMuteSelected = [this]() {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr) track->setMuted(!track->isMuted());
    };

    menuBar.onSoloSelected = [this]() {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr) track->setSoloed(!track->isSoloed());
    };

    menuBar.onPlay = [this]() { audioEngine.getTransport().start(); };
    menuBar.onStop = [this]() { audioEngine.getTransport().stop(); };
    menuBar.onPause = [this]() { audioEngine.getTransport().pause(); };
    menuBar.onRecord = [this]() {
        if (audioEngine.isRecording()) audioEngine.stopRecording();
        else audioEngine.startRecording();
    };
    menuBar.onSkipToStart = [this]() { audioEngine.getTransport().setPositionInSeconds(0.0); };
    menuBar.onSkipToEnd = [this]() {
        audioEngine.getTransport().setPositionInSeconds(audioEngine.getProjectLengthInSeconds());
    };

    menuBar.onArmSelectedTrack = [this]() {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr) track->setRecordArmed(!track->isRecordArmed());
    };

    menuBar.onDisarmAllTracks = [this]() {
        auto& tm = audioEngine.getTrackManager();
        for (int i = 0; i < tm.getNumTracks(); ++i)
            if (auto* track = tm.getTrack(i)) track->setRecordArmed(false);
    };

    menuBar.onToggleMetronome = [this]() {
        metronomeEnabled = !metronomeEnabled;
        audioEngine.setMetronomeEnabled(metronomeEnabled);
        transportBar.setMetronomeOn(metronomeEnabled);
    };

    menuBar.onToggleCountIn = [this]() {
        countInEnabled = !countInEnabled;
        audioEngine.setCountInEnabled(countInEnabled);
    };

    menuBar.onToggleInputMonitoring = [this]() {
        inputMonitoringEnabled = !inputMonitoringEnabled;
        audioEngine.setInputMonitoringEnabled(inputMonitoringEnabled);
    };

    menuBar.onFreezeTrack = [this]() {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        int trackIdx = audioEngine.getTrackManager().getSelectedTrackIndex();
        if (track != nullptr && track->getNumClips() > 0)
        {
            auto recDir = AudioFileLoader::getRecordingDirectory();
            auto freezeFile = recDir.getChildFile(track->getName() + "_frozen.wav");
            audioEngine.freezeTrack(trackIdx, freezeFile);
            timelineView.rebuildClipComponents();
        }
    };

    menuBar.onZoomIn = [this]() { transportBar.setZoomValue(transportBar.getZoomValue() * 1.3); };
    menuBar.onZoomOut = [this]() { transportBar.setZoomValue(transportBar.getZoomValue() / 1.3); };
    menuBar.onZoomToFit = [this]() { transportBar.setZoomValue(100.0); };

    menuBar.onCut = [this]() {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr && track->getNumClips() > 0)
        {
            clipboardClip = track->getClip(0);
            track->clearClips();
            timelineView.rebuildClipComponents();
        }
    };

    menuBar.onCopy = [this]() {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr && track->getNumClips() > 0)
            clipboardClip = track->getClip(0);
    };

    menuBar.onPaste = [this]() {
        if (clipboardClip == nullptr) return;
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr)
        {
            auto pasted = std::make_shared<AudioClip>(
                clipboardClip->getName(),
                clipboardClip->getSourceFile(),
                audioEngine.getTransport().getPositionInSeconds(),
                clipboardClip->getSourceOffset(),
                clipboardClip->getClipDuration(),
                clipboardClip->getSourceSampleRate());
            pasted->setGain(clipboardClip->getGain());
            track->addClip(pasted);
            timelineView.rebuildClipComponents();
        }
    };

    menuBar.onDelete = [this]() {
        auto* track = audioEngine.getTrackManager().getSelectedTrack();
        if (track != nullptr) { track->clearClips(); timelineView.rebuildClipComponents(); }
    };

    menuBar.onAudioSettings = [this]() {
        auto& dm = audioEngine.getDeviceManager();
        juce::DialogWindow::LaunchOptions options;
        auto* settingsComp = new juce::AudioDeviceSelectorComponent(
            dm, 0, 2, 0, 2, true, false, true, true);
        options.content.setOwned(settingsComp);
        options.content->setSize(500, 400);
        options.dialogTitle = "Audio Settings";
        options.useNativeTitleBar = true;
        options.launchAsync();
    };

    // ── View toggle callbacks ───────────────────────────────
    menuBar.onToggleMixer = [this]() {
        bottomVisible = !bottomVisible;
        bottomWorkspace.setVisible(bottomVisible);
        resized();
    };
    menuBar.onToggleTimeline = [this]() {
        // Toggle global tracks visibility
        globalTracks.setVisible(!globalTracks.isVisible());
        resized();
    };
}

void MainEditor::syncTransportState()
{
    transportBar.setPlaying(audioEngine.getTransport().isPlaying());
    transportBar.setRecording(audioEngine.isRecording());
    transportBar.setMetronomeOn(metronomeEnabled);
    transportBar.setLooping(loopEnabled);
    transportBar.setLoopRecording(audioEngine.isLoopRecordEnabled());
    loopRecordEnabled = audioEngine.isLoopRecordEnabled();
}

} // namespace harmonic_engine
