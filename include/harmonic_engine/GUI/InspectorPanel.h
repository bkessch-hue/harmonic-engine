// InspectorPanel.h — Collapsible right panel showing selected track
// properties: name, I/O, volume, pan, inserts (EffectRack), sends.

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/GUI/DesignTokens.h"
#include "harmonic_engine/AudioEngine/Track.h"
#include "harmonic_engine/GUI/EffectRack.h"

namespace harmonic_engine
{

class InspectorPanel : public juce::Component
{
public:
    InspectorPanel();
    ~InspectorPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTrack(Track* track);
    Track* getTrack() const;

    void setEffectChain(EffectChain* chain);

    // Callbacks
    std::function<void(float)> onVolumeChanged;
    std::function<void(float)> onPanChanged;

private:
    Track* currentTrack = nullptr;

    // ── Track Info Section ──────────────────────────────────
    juce::Label nameLabel;
    juce::Label typeLabel;

    // ── Channel Section ─────────────────────────────────────
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Slider panSlider;
    juce::Label panLabel;

    // ── Sends Section ───────────────────────────────────────
    juce::Slider sendASlider;
    juce::Label sendALabel;
    juce::Slider sendBSlider;
    juce::Label sendBLabel;

    // ── Output Section ──────────────────────────────────────
    juce::ComboBox outputCombo;
    juce::Label outputLabel;

    // ── Inserts Section (wraps EffectRack) ──────────────────
    EffectRack effectRack;

    void updateFromTrack();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectorPanel)
};

} // namespace harmonic_engine
