#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/TrackManager.h"
#include "harmonic_engine/AudioEngine/MixerGraph.h"

namespace harmonic_engine
{

class ChannelStrip : public juce::Component
{
public:
    ChannelStrip(Track& track);
    ~ChannelStrip() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeter();

private:
    Track& track;

    juce::Slider gainSlider;
    juce::Slider panSlider;
    juce::ToggleButton muteButton;
    juce::ToggleButton soloButton;
    juce::Label nameLabel;
    juce::Slider sendASlider;
    juce::Slider sendBSlider;

    float currentPeakLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
};

class MixerView : public juce::Component
{
public:
    MixerView(TrackManager& trackManager);
    ~MixerView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters();

    void setMasterPeakLevels(float left, float right);

private:
    TrackManager& trackManager;
    std::vector<std::unique_ptr<ChannelStrip>> channelStrips;

    float masterPeakLeft = 0.0f;
    float masterPeakRight = 0.0f;

    void rebuildStrips();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerView)
};

} // namespace harmonic_engine
