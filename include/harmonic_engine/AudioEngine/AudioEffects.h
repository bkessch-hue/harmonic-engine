#pragma once

#include "EffectBase.h"
#include <juce_dsp/juce_dsp.h>

namespace harmonic_engine
{

class ParametricEQ : public EffectBase
{
public:
    static constexpr int NumBands = 4;

    static constexpr float MinFrequency = 20.0f;
    static constexpr float MaxFrequency = 20000.0f;
    static constexpr float MinGain = -24.0f;
    static constexpr float MaxGain = 24.0f;
    static constexpr float MinQ = 0.1f;
    static constexpr float MaxQ = 10.0f;

    ParametricEQ();
    ~ParametricEQ() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;

    juce::String getName() const override { return "Parametric EQ"; }
    EffectType getType() const override { return EffectType::Equalizer; }

    juce::ValueTree getState() const override;
    void setState(const juce::ValueTree& state) override;

    void setBandFrequency(int band, float frequency);
    void setBandGain(int band, float gainDb);
    void setBandQ(int band, float q);

    float getBandFrequency(int band) const;
    float getBandGain(int band) const;
    float getBandQ(int band) const;

private:
    void updateFilters();

    struct BandParams
    {
        float frequency = 1000.0f;
        float gain = 0.0f;
        float q = 1.0f;
    };

    BandParams bandParams[NumBands];

    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefficients = juce::dsp::IIR::Coefficients<float>;

    juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter> chain;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricEQ)
};

class DynamicsCompressor : public EffectBase
{
public:
    static constexpr float MinThreshold = -60.0f;
    static constexpr float MaxThreshold = 0.0f;
    static constexpr float MinRatio = 1.0f;
    static constexpr float MaxRatio = 20.0f;
    static constexpr float MinAttack = 0.1f;
    static constexpr float MaxAttack = 100.0f;
    static constexpr float MinRelease = 10.0f;
    static constexpr float MaxRelease = 1000.0f;
    static constexpr float MinKnee = 0.0f;
    static constexpr float MaxKnee = 30.0f;
    static constexpr float MinMakeUpGain = 0.0f;
    static constexpr float MaxMakeUpGain = 24.0f;

    DynamicsCompressor();
    ~DynamicsCompressor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;

    juce::String getName() const override { return "Dynamics Compressor"; }
    EffectType getType() const override { return EffectType::Compressor; }

    juce::ValueTree getState() const override;
    void setState(const juce::ValueTree& state) override;

    void setThreshold(float thresholdDb);
    void setRatio(float ratio);
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    void setKnee(float kneeDb);
    void setMakeUpGain(float gainDb);

    float getThreshold() const;
    float getRatio() const;
    float getAttack() const;
    float getRelease() const;
    float getKnee() const;
    float getMakeUpGain() const;

private:
    juce::dsp::Compressor<float> compressor;

    float threshold = -20.0f;
    float ratio = 4.0f;
    float attack = 5.0f;
    float release = 200.0f;
    float knee = 6.0f;
    float makeUpGain = 0.0f;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsCompressor)
};

class ReverbEffect : public EffectBase
{
public:
    static constexpr float MinLevel = 0.0f;
    static constexpr float MaxLevel = 1.0f;
    static constexpr float MinRoomSize = 0.0f;
    static constexpr float MaxRoomSize = 1.0f;

    ReverbEffect();
    ~ReverbEffect() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;

    juce::String getName() const override { return "Reverb"; }
    EffectType getType() const override { return EffectType::Reverb; }

    juce::ValueTree getState() const override;
    void setState(const juce::ValueTree& state) override;

    void setRoomSize(float size);
    void setDamping(float damping);
    void setWetLevel(float level);
    void setDryLevel(float level);
    void setWidth(float width);
    void setFreezeMode(float mode);

    float getRoomSize() const;
    float getDamping() const;
    float getWetLevel() const;
    float getDryLevel() const;
    float getWidth() const;
    float getFreezeMode() const;

private:
    juce::dsp::Reverb reverb;

    float roomSize = 0.5f;
    float damping = 0.5f;
    float wetLevel = 0.3f;
    float dryLevel = 0.7f;
    float width = 1.0f;
    float freezeMode = 0.0f;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbEffect)
};

class DelayEffect : public EffectBase
{
public:
    static constexpr float MinDelayTime = 0.001f;
    static constexpr float MaxDelayTime = 2.0f;
    static constexpr float MinFeedback = 0.0f;
    static constexpr float MaxFeedback = 0.95f;
    static constexpr float MinMix = 0.0f;
    static constexpr float MaxMix = 1.0f;
    static constexpr float MinLfoRate = 0.0f;
    static constexpr float MaxLfoRate = 10.0f;
    static constexpr float MinLfoDepth = 0.0f;
    static constexpr float MaxLfoDepth = 1.0f;

    DelayEffect();
    ~DelayEffect() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;

    juce::String getName() const override { return "Delay"; }
    EffectType getType() const override { return EffectType::Delay; }

    juce::ValueTree getState() const override;
    void setState(const juce::ValueTree& state) override;

    void setDelayTime(float timeSeconds);
    void setFeedback(float fb);
    void setMix(float mixLevel);
    void setLfoRate(float rateHz);
    void setLfoDepth(float depth);

    float getDelayTime() const;
    float getFeedback() const;
    float getMix() const;
    float getLfoRate() const;
    float getLfoDepth() const;

private:
    juce::AudioBuffer<float> delayBuffer;
    int delayBufferSize = 0;
    int writePosition = 0;
    float readPosition = 0.0f;

    float delayTime = 0.3f;
    float feedback = 0.4f;
    float mix = 0.3f;
    float lfoRate = 0.0f;
    float lfoDepth = 0.0f;

    float lfoPhase = 0.0f;

    double currentSampleRate = 44100.0;
    int currentSamplesPerBlock = 512;

    juce::SpinLock parameterLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayEffect)
};

class DistortionEffect : public EffectBase
{
public:
    static constexpr float MinDrive = 0.0f;
    static constexpr float MaxDrive = 10.0f;
    static constexpr float MinMix = 0.0f;
    static constexpr float MaxMix = 1.0f;
    static constexpr float MinOutput = -24.0f;
    static constexpr float MaxOutput = 24.0f;

    enum class Type { SoftClip, HardClip, Tanh, Waveshape, Fuzz };

    DistortionEffect();
    ~DistortionEffect() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;

    juce::String getName() const override { return "Distortion"; }
    EffectType getType() const override { return EffectType::Distortion; }

    juce::ValueTree getState() const override;
    void setState(const juce::ValueTree& state) override;

    void setDrive(float drive);
    void setMix(float mix);
    void setOutput(float outputDb);
    void setType(Type t);

    float getDrive() const;
    float getMix() const;
    float getOutput() const;
    Type getDistortionType() const;

private:
    float drive = 1.0f;
    float mix = 0.5f;
    float output = 0.0f;
    Type distortionType = Type::Tanh;

    float applyDistortion(float sample) const;
};

class LimiterEffect : public EffectBase
{
public:
    static constexpr float MinThreshold = -60.0f;
    static constexpr float MaxThreshold = 0.0f;
    static constexpr float MinRelease = 0.001f;
    static constexpr float MaxRelease = 1.0f;

    LimiterEffect();
    ~LimiterEffect() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;

    juce::String getName() const override { return "Limiter"; }
    EffectType getType() const override { return EffectType::Limiter; }

    juce::ValueTree getState() const override;
    void setState(const juce::ValueTree& state) override;

    void setThreshold(float thresholdDb);
    void setRelease(float releaseMs);
    void setInputGain(float gainDb);
    void setOutputGain(float gainDb);

    float getThreshold() const;
    float getRelease() const;
    float getInputGain() const;
    float getOutputGain() const;

private:
    float threshold = -6.0f;
    float release = 0.1f;
    float inputGain = 0.0f;
    float outputGain = 0.0f;

    double currentSampleRate = 44100.0;
    float envelope = 0.0f;
};

class ChorusEffect : public EffectBase
{
public:
    static constexpr float MinRate = 0.0f;
    static constexpr float MaxRate = 20.0f;
    static constexpr float MinDepth = 0.0f;
    static constexpr float MaxDepth = 20.0f;
    static constexpr float MinMix = 0.0f;
    static constexpr float MaxMix = 1.0f;

    ChorusEffect();
    ~ChorusEffect() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) override;

    juce::String getName() const override { return "Chorus"; }
    EffectType getType() const override { return EffectType::Chorus; }

    juce::ValueTree getState() const override;
    void setState(const juce::ValueTree& state) override;

    void setRate(float rateHz);
    void setDepth(float depthMs);
    void setMix(float mix);
    void setCentreDelay(float delayMs);
    void setFeedback(float fb);

    float getRate() const;
    float getDepth() const;
    float getMix() const;
    float getCentreDelay() const;
    float getFeedback() const;

private:
    float rate = 0.5f;
    float depth = 3.0f;
    float mix = 0.4f;
    float centreDelay = 10.0f;
    float feedback = 0.0f;

    double currentSampleRate = 44100.0;
    double phase = 0.0;

    juce::AudioBuffer<float> delayBuffer;
    int delayBufferSize = 0;
    int writePosition = 0;
};

} // namespace harmonic_engine
