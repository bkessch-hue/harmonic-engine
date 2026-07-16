#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <map>
#include <memory>

namespace harmonic_engine
{

class SampleLoader
{
public:
    SampleLoader();
    ~SampleLoader();

    struct SampleInfo
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
        juce::int64 lengthInSamples = 0;
    };

    SampleInfo* load(const juce::File& file);
    SampleInfo* find(const juce::File& file);
    void release(const juce::File& file);
    void clearUnused();
    void clearAll();

    bool isLoaded(const juce::File& file) const;

private:
    struct CachedSample
    {
        SampleInfo info;
        int refCount = 0;
    };

    juce::AudioFormatManager formatManager;
    std::map<juce::String, CachedSample> cache;
    mutable juce::CriticalSection mutex;

    juce::String getKeyFor(const juce::File& file) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLoader)
};

} // namespace harmonic_engine
