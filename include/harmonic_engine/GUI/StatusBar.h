// StatusBar.h — Bottom status bar showing audio device, sample rate,
// buffer size, CPU usage, and MIDI activity.

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/GUI/DesignTokens.h"
#include "harmonic_engine/AudioEngine/Engine.h"

namespace harmonic_engine
{

class StatusBar : public juce::Component,
                  private juce::Timer
{
public:
    explicit StatusBar(Engine& engine);
    ~StatusBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMidiActivity(bool active);

private:
    Engine& audioEngine;

    bool midiActivity = false;
    int midiActivityTimeout = 0;

    juce::String getCpuUsageString() const;
    juce::String getAudioDeviceString() const;
    juce::String getSampleRateString() const;
    juce::String getBufferSizeString() const;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusBar)
};

} // namespace harmonic_engine
