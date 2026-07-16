#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace harmonic_engine
{

class HarmonicEngineApplication : public juce::JUCEApplication
{
public:
    HarmonicEngineApplication();

    static HarmonicEngineApplication& getApp();

    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const juce::String& commandLine) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonicEngineApplication)
};

} // namespace harmonic_engine
