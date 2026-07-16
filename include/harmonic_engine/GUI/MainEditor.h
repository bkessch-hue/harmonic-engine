// MainEditor.h — Root UI component. Now a thin orchestrator that
// creates panels, wires callbacks, and delegates layout to the
// component hierarchy (Dashboard / BrowserPanel / InspectorPanel / etc).

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/Engine.h"
#include "harmonic_engine/GUI/DesignTokens.h"
#include "harmonic_engine/GUI/MenuBar.h"
#include "harmonic_engine/GUI/TransportBar.h"
#include "harmonic_engine/GUI/TimelineView.h"
#include "harmonic_engine/GUI/MixerView.h"
#include "harmonic_engine/GUI/PianoRoll.h"
#include "harmonic_engine/GUI/StepSequencer.h"
#include "harmonic_engine/GUI/GlobalTracks.h"
#include "harmonic_engine/GUI/BrowserPanel.h"
#include "harmonic_engine/GUI/InspectorPanel.h"
#include "harmonic_engine/GUI/BottomWorkspace.h"
#include "harmonic_engine/GUI/StatusBar.h"
#include "harmonic_engine/GUI/LookAndFeel.h"
#include "harmonic_engine/GUI/VirtualKeyboard.h"

namespace harmonic_engine
{

class MainEditor : public juce::Component,
                   private juce::Timer,
                   public juce::KeyListener
{
public:
    MainEditor(Engine& engine);
    ~MainEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override;

    Engine& getEngine();

private:
    Engine& audioEngine;
    HarmonicLookAndFeel lookAndFeel;

    // ── Top area ────────────────────────────────────────────
    HarmonicMenuBar menuBar;
    TransportBar transportBar;

    // ── Side panels ─────────────────────────────────────────
    BrowserPanel browserPanel;
    InspectorPanel inspectorPanel;

    // ── Centre: arrangement + global tracks ─────────────────
    GlobalTracks globalTracks;
    TimelineView timelineView;

    // ── Bottom workspace (tabbed) ───────────────────────────
    BottomWorkspace bottomWorkspace;
    MixerView mixerView;
    PianoRoll pianoRoll;
    StepSequencer stepSequencer; // requires Transport + DrumSequencer

    // ── Status bar ──────────────────────────────────────────
    StatusBar statusBar;

    // ── Virtual piano keyboard ──────────────────────────────
    VirtualKeyboard virtualKeyboard;

    // ── Panel geometry ──────────────────────────────────────
    int browserWidth   = Tokens::kBrowserDefaultWidth;
    int inspectorWidth = Tokens::kInspectorDefaultWidth;
    int bottomHeight   = Tokens::kBottomDefaultHeight;
    bool browserVisible  = true;
    bool inspectorVisible = true;
    bool bottomVisible   = true;

    // ── State ───────────────────────────────────────────────
    bool metronomeEnabled = true;
    bool countInEnabled = false;
    bool inputMonitoringEnabled = false;
    bool recordingOverdub = true;
    bool loopEnabled = false;
    bool loopRecordEnabled = false;

    std::shared_ptr<AudioClip> clipboardClip;

    // ── Methods ─────────────────────────────────────────────
    void timerCallback() override;
    void setupCallbacks();
    void syncTransportState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainEditor)
};

} // namespace harmonic_engine
