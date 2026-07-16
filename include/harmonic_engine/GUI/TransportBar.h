// TransportBar.h — Unified professional transport control bar.
// Contains: rewind, fast-forward, stop, play, record, loop,
// metronome, tempo, time signature, musical position, and zoom.

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/GUI/DesignTokens.h"
#include "harmonic_engine/AudioEngine/Transport.h"

namespace harmonic_engine
{

class TransportBar : public juce::Component,
                     public juce::Button::Listener
{
public:
    TransportBar(Transport& transport);
    ~TransportBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateDisplay();

    void buttonClicked(juce::Button* button) override;

    // State sync
    void setPlaying(bool playing);
    void setRecording(bool recording);
    void setLooping(bool looping);
    void setLoopRecording(bool loopRecording);
    void setMetronomeOn(bool on);

    // Callbacks for external wiring
    std::function<void()> onRewind;
    std::function<void()> onFastForward;
    std::function<void()> onPlay;
    std::function<void()> onStop;
    std::function<void()> onPause;
    std::function<void()> onRecord;
    std::function<void()> onToggleLoop;
    std::function<void()> onToggleLoopRecord;
    std::function<void()> onToggleMetronome;
    std::function<void(double)> onTempoChanged;
    std::function<void(double)> onZoomChanged;

    void setZoomRange(double minPPS, double maxPPS);
    void setZoomValue(double pps);
    double getZoomValue() const;

private:
    Transport& transport;

    // ── Transport buttons (left group) ──────────────────────
    juce::TextButton rewindBtn   { "|<<" };
    juce::TextButton ffwdBtn     { ">>|" };
    juce::TextButton stopBtn     { "Stop" };
    juce::TextButton playBtn     { "Play" };
    juce::TextButton recordBtn      { "Rec" };
    juce::TextButton loopRecordBtn  { "LoopRec" };
    juce::TextButton loopBtn        { "Loop" };
    juce::TextButton metronomeBtn   { "Click" };

    // ── Tempo / Time Sig (centre group) ─────────────────────
    juce::Label tempoLabel;
    juce::Slider tempoSlider;
    juce::Label timeSigLabel;

    // ── Position display (centre-right) ─────────────────────
    juce::Label positionLabel;

    // ── Zoom (right group) ──────────────────────────────────
    juce::Label zoomLabel;
    juce::Slider zoomSlider;

    void updatePositionDisplay();
    void setupButton(juce::TextButton& btn, const juce::String& tip);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};

} // namespace harmonic_engine
