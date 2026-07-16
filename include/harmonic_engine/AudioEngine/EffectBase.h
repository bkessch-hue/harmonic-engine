#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>
#include <memory>

namespace harmonic_engine
{

enum class EffectType
{
    Equalizer,
    Compressor,
    Reverb,
    Delay,
    Chorus,
    Distortion,
    Limiter
};

class EffectBase
{
public:
    virtual ~EffectBase() = default;

    virtual void prepareToPlay(double sampleRate, int samplesPerBlock) = 0;
    virtual void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) = 0;

    virtual juce::String getName() const = 0;
    virtual EffectType getType() const = 0;

    bool isBypassed() const { return bypassed; }
    void setBypassed(bool shouldBypass) { bypassed = shouldBypass; }

    virtual juce::ValueTree getState() const = 0;
    virtual void setState(const juce::ValueTree& state) = 0;

protected:
    bool bypassed = false;
};

class EffectChain
{
public:
    EffectChain() = default;
    ~EffectChain() = default;

    void addEffect(EffectType type);
    void removeEffect(int index);

    int getNumEffects() const { return static_cast<int>(effects.size()); }
    EffectBase* getEffect(int index) const;

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples);

    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);

private:
    std::vector<std::unique_ptr<EffectBase>> effects;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectChain)
};

} // namespace harmonic_engine
