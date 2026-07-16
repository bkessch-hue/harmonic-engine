#include "harmonic_engine/AudioEngine/AudioEffects.h"

namespace harmonic_engine
{

// ============================================================================
// EffectChain
// ============================================================================

void EffectChain::addEffect(EffectType type)
{
    std::unique_ptr<EffectBase> effect;

    switch (type)
    {
        case EffectType::Equalizer:
            effect = std::make_unique<ParametricEQ>();
            break;
        case EffectType::Compressor:
            effect = std::make_unique<DynamicsCompressor>();
            break;
        case EffectType::Reverb:
            effect = std::make_unique<ReverbEffect>();
            break;
        case EffectType::Delay:
            effect = std::make_unique<DelayEffect>();
            break;
        case EffectType::Chorus:
            effect = std::make_unique<ChorusEffect>();
            break;
        case EffectType::Distortion:
            effect = std::make_unique<DistortionEffect>();
            break;
        case EffectType::Limiter:
            effect = std::make_unique<LimiterEffect>();
            break;
        default:
            return;
    }

    effects.push_back(std::move(effect));
}

void EffectChain::removeEffect(int index)
{
    if (index >= 0 && index < static_cast<int>(effects.size()))
        effects.erase(effects.begin() + index);
}

EffectBase* EffectChain::getEffect(int index) const
{
    if (index >= 0 && index < static_cast<int>(effects.size()))
        return effects[index].get();
    return nullptr;
}

void EffectChain::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (auto& effect : effects)
        effect->prepareToPlay(sampleRate, samplesPerBlock);
}

void EffectChain::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    for (auto& effect : effects)
    {
        if (!effect->isBypassed())
            effect->processBlock(buffer, numSamples);
    }
}

juce::ValueTree EffectChain::getState() const
{
    juce::ValueTree state("EffectChain");

    for (auto& effect : effects)
        state.addChild(effect->getState(), -1, nullptr);

    return state;
}

void EffectChain::setState(const juce::ValueTree& state)
{
    if (!state.hasType("EffectChain"))
        return;

    effects.clear();

    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto childState = state.getChild(i);
        auto typeName = childState.getProperty("Type", "").toString();

        if (typeName == "Equalizer")
            addEffect(EffectType::Equalizer);
        else if (typeName == "Compressor")
            addEffect(EffectType::Compressor);
        else if (typeName == "Reverb")
            addEffect(EffectType::Reverb);
        else if (typeName == "Delay")
            addEffect(EffectType::Delay);
        else if (typeName == "Chorus")
            addEffect(EffectType::Chorus);
        else if (typeName == "Distortion")
            addEffect(EffectType::Distortion);
        else if (typeName == "Limiter")
            addEffect(EffectType::Limiter);
        else
            continue;

        if (!effects.empty())
            effects.back()->setState(childState);
    }
}

// ============================================================================
// ParametricEQ
// ============================================================================

ParametricEQ::ParametricEQ()
{
    bandParams[0] = { 100.0f, 0.0f, 1.0f };
    bandParams[1] = { 500.0f, 0.0f, 1.0f };
    bandParams[2] = { 2000.0f, 0.0f, 1.0f };
    bandParams[3] = { 8000.0f, 0.0f, 1.0f };
}

void ParametricEQ::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };
    chain.prepare(spec);
    updateFilters();
}

void ParametricEQ::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (bypassed)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    chain.process(context);
}

void ParametricEQ::updateFilters()
{
    for (int i = 0; i < NumBands; ++i)
    {
        auto& params = bandParams[i];
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, params.frequency, params.q,
            juce::Decibels::decibelsToGain(params.gain));

        switch (i)
        {
            case 0: *chain.template get<0>().coefficients = *coeffs; break;
            case 1: *chain.template get<1>().coefficients = *coeffs; break;
            case 2: *chain.template get<2>().coefficients = *coeffs; break;
            case 3: *chain.template get<3>().coefficients = *coeffs; break;
            default: break;
        }
    }
}

juce::ValueTree ParametricEQ::getState() const
{
    juce::ValueTree state("Equalizer");
    state.setProperty("Type", "Equalizer", nullptr);
    state.setProperty("Bypassed", bypassed, nullptr);

    for (int i = 0; i < NumBands; ++i)
    {
        juce::ValueTree bandState("Band" + juce::String(i + 1));
        bandState.setProperty("Frequency", bandParams[i].frequency, nullptr);
        bandState.setProperty("Gain", bandParams[i].gain, nullptr);
        bandState.setProperty("Q", bandParams[i].q, nullptr);
        state.addChild(bandState, -1, nullptr);
    }

    return state;
}

void ParametricEQ::setState(const juce::ValueTree& state)
{
    if (!state.hasType("Equalizer"))
        return;

    bypassed = state.getProperty("Bypassed", false);

    for (int i = 0; i < NumBands; ++i)
    {
        juce::ValueTree bandState;
        for (int c = 0; c < state.getNumChildren(); ++c)
        {
            auto child = state.getChild(c);
            if (child.hasType("Band" + juce::String(i + 1)))
            {
                bandState = child;
                break;
            }
        }
        if (bandState.isValid())
        {
            bandParams[i].frequency = bandState.getProperty("Frequency", bandParams[i].frequency);
            bandParams[i].gain = bandState.getProperty("Gain", bandParams[i].gain);
            bandParams[i].q = bandState.getProperty("Q", bandParams[i].q);
        }
    }

    if (currentSampleRate > 0)
        updateFilters();
}

void ParametricEQ::setBandFrequency(int band, float frequency)
{
    if (band >= 0 && band < NumBands)
    {
        bandParams[band].frequency = juce::jlimit(MinFrequency, MaxFrequency, frequency);
        updateFilters();
    }
}

void ParametricEQ::setBandGain(int band, float gainDb)
{
    if (band >= 0 && band < NumBands)
    {
        bandParams[band].gain = juce::jlimit(MinGain, MaxGain, gainDb);
        updateFilters();
    }
}

void ParametricEQ::setBandQ(int band, float q)
{
    if (band >= 0 && band < NumBands)
    {
        bandParams[band].q = juce::jlimit(MinQ, MaxQ, q);
        updateFilters();
    }
}

float ParametricEQ::getBandFrequency(int band) const
{
    return (band >= 0 && band < NumBands) ? bandParams[band].frequency : 0.0f;
}

float ParametricEQ::getBandGain(int band) const
{
    return (band >= 0 && band < NumBands) ? bandParams[band].gain : 0.0f;
}

float ParametricEQ::getBandQ(int band) const
{
    return (band >= 0 && band < NumBands) ? bandParams[band].q : 1.0f;
}

// ============================================================================
// DynamicsCompressor
// ============================================================================

DynamicsCompressor::DynamicsCompressor() = default;

void DynamicsCompressor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };
    compressor.prepare(spec);

    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    compressor.setAttack(attack / 1000.0);
    compressor.setRelease(release / 1000.0);
}

void DynamicsCompressor::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (bypassed)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);

    if (makeUpGain > 0.0f)
    {
        float gainLinear = juce::Decibels::decibelsToGain(makeUpGain);
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.applyGain(channel, 0, numSamples, gainLinear);
    }
}

juce::ValueTree DynamicsCompressor::getState() const
{
    juce::ValueTree state("Compressor");
    state.setProperty("Type", "Compressor", nullptr);
    state.setProperty("Bypassed", bypassed, nullptr);
    state.setProperty("Threshold", threshold, nullptr);
    state.setProperty("Ratio", ratio, nullptr);
    state.setProperty("Attack", attack, nullptr);
    state.setProperty("Release", release, nullptr);
    state.setProperty("Knee", knee, nullptr);
    state.setProperty("MakeUpGain", makeUpGain, nullptr);
    return state;
}

void DynamicsCompressor::setState(const juce::ValueTree& state)
{
    if (!state.hasType("Compressor"))
        return;

    bypassed = state.getProperty("Bypassed", false);
    threshold = state.getProperty("Threshold", threshold);
    ratio = state.getProperty("Ratio", ratio);
    attack = state.getProperty("Attack", attack);
    release = state.getProperty("Release", release);
    knee = state.getProperty("Knee", knee);
    makeUpGain = state.getProperty("MakeUpGain", makeUpGain);

    if (currentSampleRate > 0)
    {
        compressor.setThreshold(threshold);
        compressor.setRatio(ratio);
        compressor.setAttack(attack / 1000.0);
        compressor.setRelease(release / 1000.0);
    }
}

void DynamicsCompressor::setThreshold(float thresholdDb)
{
    threshold = juce::jlimit(MinThreshold, MaxThreshold, thresholdDb);
    compressor.setThreshold(threshold);
}

void DynamicsCompressor::setRatio(float r)
{
    ratio = juce::jlimit(MinRatio, MaxRatio, r);
    compressor.setRatio(ratio);
}

void DynamicsCompressor::setAttack(float attackMs)
{
    attack = juce::jlimit(MinAttack, MaxAttack, attackMs);
    compressor.setAttack(attack / 1000.0);
}

void DynamicsCompressor::setRelease(float releaseMs)
{
    release = juce::jlimit(MinRelease, MaxRelease, releaseMs);
    compressor.setRelease(release / 1000.0);
}

void DynamicsCompressor::setKnee(float kneeDb)
{
    knee = juce::jlimit(MinKnee, MaxKnee, kneeDb);
}

void DynamicsCompressor::setMakeUpGain(float gainDb)
{
    makeUpGain = juce::jlimit(MinMakeUpGain, MaxMakeUpGain, gainDb);
}

float DynamicsCompressor::getThreshold() const { return threshold; }
float DynamicsCompressor::getRatio() const { return ratio; }
float DynamicsCompressor::getAttack() const { return attack; }
float DynamicsCompressor::getRelease() const { return release; }
float DynamicsCompressor::getKnee() const { return knee; }
float DynamicsCompressor::getMakeUpGain() const { return makeUpGain; }

// ============================================================================
// ReverbEffect
// ============================================================================

ReverbEffect::ReverbEffect() = default;

void ReverbEffect::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };
    reverb.prepare(spec);

    juce::dsp::Reverb::Parameters params;
    params.roomSize = roomSize;
    params.damping = damping;
    params.wetLevel = wetLevel;
    params.dryLevel = dryLevel;
    params.width = width;
    params.freezeMode = freezeMode;
    reverb.setParameters(params);
}

void ReverbEffect::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (bypassed)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

juce::ValueTree ReverbEffect::getState() const
{
    juce::ValueTree state("Reverb");
    state.setProperty("Type", "Reverb", nullptr);
    state.setProperty("Bypassed", bypassed, nullptr);
    state.setProperty("RoomSize", roomSize, nullptr);
    state.setProperty("Damping", damping, nullptr);
    state.setProperty("WetLevel", wetLevel, nullptr);
    state.setProperty("DryLevel", dryLevel, nullptr);
    state.setProperty("Width", width, nullptr);
    state.setProperty("FreezeMode", freezeMode, nullptr);
    return state;
}

void ReverbEffect::setState(const juce::ValueTree& state)
{
    if (!state.hasType("Reverb"))
        return;

    bypassed = state.getProperty("Bypassed", false);
    roomSize = state.getProperty("RoomSize", roomSize);
    damping = state.getProperty("Damping", damping);
    wetLevel = state.getProperty("WetLevel", wetLevel);
    dryLevel = state.getProperty("DryLevel", dryLevel);
    width = state.getProperty("Width", width);
    freezeMode = state.getProperty("FreezeMode", freezeMode);

    juce::dsp::Reverb::Parameters params;
    params.roomSize = roomSize;
    params.damping = damping;
    params.wetLevel = wetLevel;
    params.dryLevel = dryLevel;
    params.width = width;
    params.freezeMode = freezeMode;
    reverb.setParameters(params);
}

void ReverbEffect::setRoomSize(float size)
{
    roomSize = juce::jlimit(MinRoomSize, MaxRoomSize, size);
    auto params = reverb.getParameters();
    params.roomSize = roomSize;
    reverb.setParameters(params);
}

void ReverbEffect::setDamping(float d)
{
    damping = juce::jlimit(MinLevel, MaxLevel, d);
    auto params = reverb.getParameters();
    params.damping = damping;
    reverb.setParameters(params);
}

void ReverbEffect::setWetLevel(float level)
{
    wetLevel = juce::jlimit(MinLevel, MaxLevel, level);
    auto params = reverb.getParameters();
    params.wetLevel = wetLevel;
    reverb.setParameters(params);
}

void ReverbEffect::setDryLevel(float level)
{
    dryLevel = juce::jlimit(MinLevel, MaxLevel, level);
    auto params = reverb.getParameters();
    params.dryLevel = dryLevel;
    reverb.setParameters(params);
}

void ReverbEffect::setWidth(float w)
{
    width = juce::jlimit(MinLevel, MaxLevel, w);
    auto params = reverb.getParameters();
    params.width = width;
    reverb.setParameters(params);
}

void ReverbEffect::setFreezeMode(float mode)
{
    freezeMode = juce::jlimit(MinLevel, MaxLevel, mode);
    auto params = reverb.getParameters();
    params.freezeMode = freezeMode;
    reverb.setParameters(params);
}

float ReverbEffect::getRoomSize() const { return roomSize; }
float ReverbEffect::getDamping() const { return damping; }
float ReverbEffect::getWetLevel() const { return wetLevel; }
float ReverbEffect::getDryLevel() const { return dryLevel; }
float ReverbEffect::getWidth() const { return width; }
float ReverbEffect::getFreezeMode() const { return freezeMode; }

// ============================================================================
// DelayEffect
// ============================================================================

DelayEffect::DelayEffect() = default;

void DelayEffect::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;

    delayBufferSize = static_cast<int>((MaxDelayTime + 0.1) * sampleRate) + samplesPerBlock;
    delayBuffer.setSize(2, delayBufferSize);
    delayBuffer.clear();

    writePosition = 0;
    readPosition = 0.0f;
    lfoPhase = 0.0f;
}

void DelayEffect::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (bypassed)
        return;

    const auto lock = juce::SpinLock::ScopedLockType(parameterLock);

    const int numChannels = buffer.getNumChannels();
    const float sampleRateFloat = static_cast<float>(currentSampleRate);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float lfoValue = 0.0f;
        if (lfoDepth > 0.0f && lfoRate > 0.0f)
        {
            lfoValue = std::sin(lfoPhase * 2.0f * juce::MathConstants<float>::pi);
            lfoPhase += lfoRate / sampleRateFloat;
            if (lfoPhase >= 1.0f)
                lfoPhase -= 1.0f;
        }

        float delaySamples = delayTime * sampleRateFloat;
        float modulatedDelay = delaySamples + lfoValue * lfoDepth * delaySamples;
        modulatedDelay = juce::jmax(1.0f, modulatedDelay);

        float currentReadPos = static_cast<float>(writePosition) - modulatedDelay;
        while (currentReadPos < 0.0f)
            currentReadPos += static_cast<float>(delayBufferSize);

        int readPos0 = static_cast<int>(currentReadPos) % delayBufferSize;
        int readPos1 = (readPos0 + 1) % delayBufferSize;
        float frac = currentReadPos - std::floor(currentReadPos);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* delayData = delayBuffer.getWritePointer(channel);

            float delayedSample = delayData[readPos0] * (1.0f - frac)
                                + delayData[readPos1] * frac;

            float inputSample = buffer.getSample(channel, sample);

            delayData[writePosition] = inputSample + delayedSample * feedback;

            buffer.setSample(channel, sample,
                inputSample * (1.0f - mix) + delayedSample * mix);
        }

        writePosition = (writePosition + 1) % delayBufferSize;
    }
}

juce::ValueTree DelayEffect::getState() const
{
    juce::ValueTree state("Delay");
    state.setProperty("Type", "Delay", nullptr);
    state.setProperty("Bypassed", bypassed, nullptr);
    state.setProperty("DelayTime", delayTime, nullptr);
    state.setProperty("Feedback", feedback, nullptr);
    state.setProperty("Mix", mix, nullptr);
    state.setProperty("LfoRate", lfoRate, nullptr);
    state.setProperty("LfoDepth", lfoDepth, nullptr);
    return state;
}

void DelayEffect::setState(const juce::ValueTree& state)
{
    if (!state.hasType("Delay"))
        return;

    bypassed = state.getProperty("Bypassed", false);

    {
        const auto lock = juce::SpinLock::ScopedLockType(parameterLock);
        delayTime = state.getProperty("DelayTime", delayTime);
        feedback = state.getProperty("Feedback", feedback);
        mix = state.getProperty("Mix", mix);
        lfoRate = state.getProperty("LfoRate", lfoRate);
        lfoDepth = state.getProperty("LfoDepth", lfoDepth);
    }
}

void DelayEffect::setDelayTime(float timeSeconds)
{
    const auto lock = juce::SpinLock::ScopedLockType(parameterLock);
    delayTime = juce::jlimit(MinDelayTime, MaxDelayTime, timeSeconds);
}

void DelayEffect::setFeedback(float fb)
{
    const auto lock = juce::SpinLock::ScopedLockType(parameterLock);
    feedback = juce::jlimit(MinFeedback, MaxFeedback, fb);
}

void DelayEffect::setMix(float mixLevel)
{
    const auto lock = juce::SpinLock::ScopedLockType(parameterLock);
    mix = juce::jlimit(MinMix, MaxMix, mixLevel);
}

void DelayEffect::setLfoRate(float rateHz)
{
    const auto lock = juce::SpinLock::ScopedLockType(parameterLock);
    lfoRate = juce::jlimit(MinLfoRate, MaxLfoRate, rateHz);
}

void DelayEffect::setLfoDepth(float depth)
{
    const auto lock = juce::SpinLock::ScopedLockType(parameterLock);
    lfoDepth = juce::jlimit(MinLfoDepth, MaxLfoDepth, depth);
}

float DelayEffect::getDelayTime() const { return delayTime; }
float DelayEffect::getFeedback() const { return feedback; }
float DelayEffect::getMix() const { return mix; }
float DelayEffect::getLfoRate() const { return lfoRate; }
float DelayEffect::getLfoDepth() const { return lfoDepth; }

// ============================================================================
// DistortionEffect
// ============================================================================

DistortionEffect::DistortionEffect() = default;

void DistortionEffect::prepareToPlay(double, int) {}

void DistortionEffect::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (bypassed) return;

    const int numChannels = buffer.getNumChannels();
    float outputGain = juce::Decibels::decibelsToGain(output);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < numSamples; ++s)
        {
            float dry = data[s];
            float wet = applyDistortion(dry * drive);
            data[s] = dry * (1.0f - mix) + wet * mix;
            data[s] *= outputGain;
        }
    }
}

float DistortionEffect::applyDistortion(float sample) const
{
    switch (distortionType)
    {
        case Type::SoftClip:
            return std::tanh(sample);
        case Type::HardClip:
            return juce::jlimit(-1.0f, 1.0f, sample);
        case Type::Tanh:
            return std::tanh(sample * 2.0f) / std::tanh(2.0f);
        case Type::Waveshape:
        {
            float s = juce::jlimit(-1.0f, 1.0f, sample);
            return s * (3.0f - s * s) * 0.5f;
        }
        case Type::Fuzz:
        {
            float s = sample * 5.0f;
            s = juce::jlimit(-1.0f, 1.0f, s);
            return std::floor(s * 8.0f) / 8.0f;
        }
        default:
            return sample;
    }
}

juce::ValueTree DistortionEffect::getState() const
{
    juce::ValueTree state("Distortion");
    state.setProperty("Type", "Distortion", nullptr);
    state.setProperty("Bypassed", bypassed, nullptr);
    state.setProperty("Drive", drive, nullptr);
    state.setProperty("Mix", mix, nullptr);
    state.setProperty("Output", output, nullptr);
    state.setProperty("DistortionType", static_cast<int>(distortionType), nullptr);
    return state;
}

void DistortionEffect::setState(const juce::ValueTree& state)
{
    if (!state.hasType("Distortion")) return;
    bypassed = state.getProperty("Bypassed", false);
    drive = state.getProperty("Drive", drive);
    mix = state.getProperty("Mix", mix);
    output = state.getProperty("Output", output);
    distortionType = static_cast<Type>(static_cast<int>(state.getProperty("DistortionType", static_cast<int>(Type::Tanh))));
}

void DistortionEffect::setDrive(float d) { drive = juce::jlimit(MinDrive, MaxDrive, d); }
void DistortionEffect::setMix(float m) { mix = juce::jlimit(MinMix, MaxMix, m); }
void DistortionEffect::setOutput(float outputDb) { output = juce::jlimit(MinOutput, MaxOutput, outputDb); }
void DistortionEffect::setType(Type t) { distortionType = t; }
float DistortionEffect::getDrive() const { return drive; }
float DistortionEffect::getMix() const { return mix; }
float DistortionEffect::getOutput() const { return output; }
DistortionEffect::Type DistortionEffect::getDistortionType() const { return distortionType; }

// ============================================================================
// LimiterEffect
// ============================================================================

LimiterEffect::LimiterEffect() = default;

void LimiterEffect::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    envelope = 0.0f;
}

void LimiterEffect::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (bypassed) return;

    const int numChannels = buffer.getNumChannels();
    float thresholdLinear = juce::Decibels::decibelsToGain(threshold);
    float inputGainLinear = juce::Decibels::decibelsToGain(inputGain);
    float outputGainLinear = juce::Decibels::decibelsToGain(outputGain);

    float releaseCoeff = static_cast<float>(std::exp(-1.0 / (currentSampleRate * release)));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < numSamples; ++s)
        {
            float sample = data[s] * inputGainLinear;
            float absSample = std::abs(sample);

            if (absSample > envelope)
                envelope = absSample;
            else
                envelope += (absSample - envelope) * releaseCoeff;

            float gainReduction = 1.0f;
            if (envelope > thresholdLinear)
                gainReduction = thresholdLinear / envelope;

            data[s] = sample * gainReduction * outputGainLinear;
        }
    }
}

juce::ValueTree LimiterEffect::getState() const
{
    juce::ValueTree state("Limiter");
    state.setProperty("Type", "Limiter", nullptr);
    state.setProperty("Bypassed", bypassed, nullptr);
    state.setProperty("Threshold", threshold, nullptr);
    state.setProperty("Release", release, nullptr);
    state.setProperty("InputGain", inputGain, nullptr);
    state.setProperty("OutputGain", outputGain, nullptr);
    return state;
}

void LimiterEffect::setState(const juce::ValueTree& state)
{
    if (!state.hasType("Limiter")) return;
    bypassed = state.getProperty("Bypassed", false);
    threshold = state.getProperty("Threshold", threshold);
    release = state.getProperty("Release", release);
    inputGain = state.getProperty("InputGain", inputGain);
    outputGain = state.getProperty("OutputGain", outputGain);
}

void LimiterEffect::setThreshold(float t) { threshold = juce::jlimit(MinThreshold, MaxThreshold, t); }
void LimiterEffect::setRelease(float r) { release = juce::jlimit(MinRelease, MaxRelease, r); }
void LimiterEffect::setInputGain(float g) { inputGain = juce::jlimit(MinThreshold, MaxThreshold, g); }
void LimiterEffect::setOutputGain(float g) { outputGain = juce::jlimit(MinThreshold, MaxThreshold, g); }
float LimiterEffect::getThreshold() const { return threshold; }
float LimiterEffect::getRelease() const { return release; }
float LimiterEffect::getInputGain() const { return inputGain; }
float LimiterEffect::getOutputGain() const { return outputGain; }

// ============================================================================
// ChorusEffect
// ============================================================================

ChorusEffect::ChorusEffect() = default;

void ChorusEffect::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    phase = 0.0;
    int maxDelaySamples = static_cast<int>((centreDelay + depth + 10.0) * 0.001 * sampleRate) + 2;
    delayBufferSize = maxDelaySamples + 4;
    delayBuffer.setSize(2, delayBufferSize);
    delayBuffer.clear();
    writePosition = 0;
}

void ChorusEffect::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (bypassed) return;

    const int numChannels = juce::jmin(buffer.getNumChannels(), delayBuffer.getNumChannels());
    const float sampleRateFloat = static_cast<float>(currentSampleRate);

    for (int s = 0; s < numSamples; ++s)
    {
        phase += rate / sampleRateFloat;
        if (phase >= 1.0) phase -= 1.0;

        double lfo = std::sin(phase * 2.0 * juce::MathConstants<double>::pi);
        double modulation = lfo * (depth * 0.001 * sampleRateFloat);

        double centreSamples = centreDelay * 0.001 * sampleRateFloat;
        double delaySamples = centreSamples + modulation;
        delaySamples = juce::jmax(1.0, delaySamples);

        double readPos = static_cast<double>(writePosition) - delaySamples;
        while (readPos < 0.0) readPos += delayBufferSize;

        int readIdx0 = static_cast<int>(readPos) % delayBufferSize;
        int readIdx1 = (readIdx0 + 1) % delayBufferSize;
        float frac = static_cast<float>(readPos - std::floor(readPos));

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* inputData = buffer.getReadPointer(ch);
            float* delayData = delayBuffer.getWritePointer(ch);

            float delayedSample = delayData[readIdx0] * (1.0f - frac) + delayData[readIdx1] * frac;
            float inputSample = inputData[s];

            delayData[writePosition] = inputSample + delayedSample * feedback;
            buffer.setSample(ch, s, inputSample * (1.0f - mix) + delayedSample * mix);
        }

        writePosition = (writePosition + 1) % delayBufferSize;
    }
}

juce::ValueTree ChorusEffect::getState() const
{
    juce::ValueTree state("Chorus");
    state.setProperty("Type", "Chorus", nullptr);
    state.setProperty("Bypassed", bypassed, nullptr);
    state.setProperty("Rate", rate, nullptr);
    state.setProperty("Depth", depth, nullptr);
    state.setProperty("Mix", mix, nullptr);
    state.setProperty("CentreDelay", centreDelay, nullptr);
    state.setProperty("Feedback", feedback, nullptr);
    return state;
}

void ChorusEffect::setState(const juce::ValueTree& state)
{
    if (!state.hasType("Chorus")) return;
    bypassed = state.getProperty("Bypassed", false);
    rate = state.getProperty("Rate", rate);
    depth = state.getProperty("Depth", depth);
    mix = state.getProperty("Mix", mix);
    centreDelay = state.getProperty("CentreDelay", centreDelay);
    feedback = state.getProperty("Feedback", feedback);
}

void ChorusEffect::setRate(float r) { rate = juce::jlimit(MinRate, MaxRate, r); }
void ChorusEffect::setDepth(float d) { depth = juce::jlimit(MinDepth, MaxDepth, d); }
void ChorusEffect::setMix(float m) { mix = juce::jlimit(MinMix, MaxMix, m); }
void ChorusEffect::setCentreDelay(float d) { centreDelay = juce::jmax(0.0f, d); }
void ChorusEffect::setFeedback(float fb) { feedback = juce::jlimit(0.0f, 0.95f, fb); }
float ChorusEffect::getRate() const { return rate; }
float ChorusEffect::getDepth() const { return depth; }
float ChorusEffect::getMix() const { return mix; }
float ChorusEffect::getCentreDelay() const { return centreDelay; }
float ChorusEffect::getFeedback() const { return feedback; }

} // namespace harmonic_engine
