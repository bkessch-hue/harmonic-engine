#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "harmonic_engine/AudioEngine/Instrument.h"
#include <vector>
#include <memory>
#include <functional>

namespace harmonic_engine
{

struct InstrumentInfo
{
    juce::String name;
    InstrumentType type;
};

class InstrumentManager
{
public:
    InstrumentManager();
    ~InstrumentManager();

    static InstrumentManager& getInstance();

    int getNumInstruments() const;
    InstrumentInfo getInstrumentInfo(int index) const;

    std::unique_ptr<juce::Synthesiser> createInstrument(InstrumentType type) const;

    juce::StringArray getInstrumentNames() const;
    int getInstrumentIndexByName(const juce::String& name) const;

    static const std::vector<InstrumentInfo>& getAvailableInstruments();

private:
    void addInstrument(const juce::String& name, InstrumentType type);

    std::vector<InstrumentInfo> instruments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentManager)
};

} // namespace harmonic_engine
