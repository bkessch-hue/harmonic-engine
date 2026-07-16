#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include "harmonic_engine/AudioEngine/Track.h"
#include "harmonic_engine/AudioEngine/EffectBase.h"

namespace harmonic_engine
{

class MixerGraph
{
public:
    MixerGraph();
    ~MixerGraph();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& outputBuffer,
                      juce::MidiBuffer& midiMessages,
                      std::vector<Track*>& tracks,
                      int numSamples);

    float getMasterPeakLevel(int channel) const;
    void setMasterGain(float gain);
    float getMasterGain() const;

    EffectChain& getBusAEffectChain();
    EffectChain& getBusBEffectChain();
    void setBusAGain(float gain);
    void setBusBGain(float gain);
    float getBusAGain() const;
    float getBusBGain() const;

private:
    double currentSampleRate = 44100.0;
    int currentSamplesPerBlock = 512;

    float masterGain = 1.0f;
    std::atomic<float> masterPeakLeft{ 0.0f };
    std::atomic<float> masterPeakRight{ 0.0f };

    EffectChain busAChain;
    EffectChain busBChain;
    float busAGain = 1.0f;
    float busBGain = 1.0f;

    juce::AudioBuffer<float> tempBuffer;
    juce::AudioBuffer<float> busABuffer;
    juce::AudioBuffer<float> busBBuffer;

    void updatePeakLevels(const juce::AudioBuffer<float>& buffer, int numSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerGraph)
};

} // namespace harmonic_engine
