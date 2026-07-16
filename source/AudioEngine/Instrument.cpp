#include "harmonic_engine/AudioEngine/Instrument.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace harmonic_engine
{

// ============================================================================
// WavetableSynthVoice — 8 waveforms with smooth morphing
// ============================================================================

WavetableSynthVoice::WavetableSynthVoice()
{
    for (int i = 0; i < NumWaveforms; ++i)
    {
        tables[i].resize(TableSize);
        generateTable(static_cast<Waveform>(i), tables[i]);
    }
}

void WavetableSynthVoice::generateTable(Waveform wf, std::vector<float>& table)
{
    for (int i = 0; i < TableSize; ++i)
    {
        double ph = static_cast<double>(i) / static_cast<double>(TableSize);
        double val = 0.0;
        switch (wf)
        {
            case Sine:
                val = std::sin(2.0 * juce::MathConstants<double>::pi * ph);
                break;
            case Saw:
                val = 2.0 * ph - 1.0;
                break;
            case Square:
                val = ph < 0.5 ? 1.0 : -1.0;
                break;
            case Triangle:
                val = ph < 0.5 ? 4.0 * ph - 1.0 : 3.0 - 4.0 * ph;
                break;
            case Pulse25:
                val = ph < 0.25 ? 1.0 : -1.0;
                break;
            case Noise:
                val = std::sin(2.0 * juce::MathConstants<double>::pi * ph)
                    + 0.5 * std::sin(4.0 * juce::MathConstants<double>::pi * ph)
                    + 0.25 * std::sin(8.0 * juce::MathConstants<double>::pi * ph)
                    + 0.125 * std::sin(16.0 * juce::MathConstants<double>::pi * ph);
                val *= 0.5;
                break;
            case BrightSaw:
                for (int h = 1; h <= 8; ++h)
                    val += (h % 2 == 1 ? 1.0 : -1.0) * std::sin(2.0 * juce::MathConstants<double>::pi * ph * h) / h;
                val *= 0.5;
                break;
            case OrganLike:
                val = std::sin(2.0 * juce::MathConstants<double>::pi * ph)
                    + 0.5 * std::sin(2.0 * juce::MathConstants<double>::pi * ph * 2.0)
                    + 0.33 * std::sin(2.0 * juce::MathConstants<double>::pi * ph * 3.0)
                    + 0.25 * std::sin(2.0 * juce::MathConstants<double>::pi * ph * 4.0)
                    + 0.2 * std::sin(2.0 * juce::MathConstants<double>::pi * ph * 6.0);
                val *= 0.3;
                break;
        }
        table[i] = static_cast<float>(val);
    }
}

void WavetableSynthVoice::setMorphPosition(float morph)
{
    morphPosition = juce::jlimit(0.0f, 1.0f, morph);
}

void WavetableSynthVoice::setWaveformOffset(int offset)
{
    morphPosition = juce::jlimit(0, NumWaveforms - 1, offset) / static_cast<float>(NumWaveforms - 1);
}

void WavetableSynthVoice::setPhaseDistortion(float amount)
{
    phaseDistortion = juce::jlimit(0.0f, 1.0f, amount);
}

float WavetableSynthVoice::generateSample(double ph, double, float amp, float)
{
    // Phase distortion: bend the phase toward 0 or 1 edges
    if (phaseDistortion > 0.0f)
    {
        double bend = phaseDistortion * 0.4;
        ph = ph + bend * std::sin(2.0 * juce::MathConstants<double>::pi * ph);
        if (ph < 0.0) ph += 1.0;
        if (ph >= 1.0) ph -= 1.0;
    }

    // Map morphPosition (0..1) to two adjacent table indices
    float tablePos = morphPosition * (NumWaveforms - 1);
    int idxA = static_cast<int>(tablePos);
    int idxB = std::min(idxA + 1, NumWaveforms - 1);
    float frac = tablePos - idxA;

    auto& tableA = tables[idxA];
    auto& tableB = tables[idxB];

    double pos = ph * TableSize;
    int t0 = static_cast<int>(pos) % TableSize;
    int t1 = (t0 + 1) % TableSize;
    float f0 = static_cast<float>(pos - std::floor(pos));

    float a = tableA[t0] * (1.0f - f0) + tableA[t1] * f0;
    float b = tableB[t0] * (1.0f - f0) + tableB[t1] * f0;

    return (a * (1.0f - frac) + b * frac) * amp;
}

// ============================================================================
// SubtractiveSynthVoice — SVF filter + ADSR + filter envelope + LFO
// ============================================================================

SubtractiveSynthVoice::SubtractiveSynthVoice() = default;

void SubtractiveSynthVoice::setFilterCutoff(double hz)
{
    filterCutoff = juce::jlimit(20.0, 20000.0, hz);
    filterBaseCutoff = filterCutoff;
}

void SubtractiveSynthVoice::setFilterResonance(double q)
{
    filterResonance = juce::jlimit(0.0, 1.0, q);
}

void SubtractiveSynthVoice::setFilterType(int type)
{
    filterType = juce::jlimit(0, 2, type);
}

void SubtractiveSynthVoice::setFilterEnvAmount(double amount)
{
    filterEnvAmount = juce::jlimit(0.0, 1.0, amount);
}

void SubtractiveSynthVoice::setAttack(double seconds)
{
    attackRate = seconds > 0.0 ? 1.0 / (seconds * currentSampleRate) : 1.0;
}

void SubtractiveSynthVoice::setDecay(double seconds)
{
    decayRate = seconds > 0.0 ? 1.0 / (seconds * currentSampleRate) : 1.0;
}

void SubtractiveSynthVoice::setSustain(double level)
{
    sustainLevel = juce::jlimit(0.0, 1.0, level);
}

void SubtractiveSynthVoice::setRelease(double seconds)
{
    releaseRate = seconds > 0.0 ? 1.0 / (seconds * currentSampleRate) : 1.0;
}

void SubtractiveSynthVoice::setFilterAttack(double seconds)
{
    filterAttackRate = seconds > 0.0 ? 1.0 / (seconds * currentSampleRate) : 1.0;
}

void SubtractiveSynthVoice::setFilterDecay(double seconds)
{
    filterDecayRate = seconds > 0.0 ? 1.0 / (seconds * currentSampleRate) : 1.0;
}

void SubtractiveSynthVoice::setFilterSustain(double level)
{
    filterSustainLevel = juce::jlimit(0.0, 1.0, level);
}

void SubtractiveSynthVoice::setFilterRelease(double seconds)
{
    filterReleaseRate = seconds > 0.0 ? 1.0 / (seconds * currentSampleRate) : 1.0;
}

void SubtractiveSynthVoice::setLFOFrequency(double hz)
{
    lfoFrequency = juce::jlimit(0.01, 100.0, hz);
}

void SubtractiveSynthVoice::setLFODepth(double depth)
{
    lfoDepth = juce::jlimit(0.0, 1.0, depth);
}

void SubtractiveSynthVoice::setLFOToFilter(double amount)
{
    lfoToFilter = juce::jlimit(0.0, 1.0, amount);
}

void SubtractiveSynthVoice::startNote(int midiNoteNumber, float velocity,
                                       juce::SynthesiserSound* sound,
                                       int currentPitchWheelPosition)
{
    HarmonicSynthVoice::startNote(midiNoteNumber, velocity, sound, currentPitchWheelPosition);
    currentSampleRate = getSampleRate();

    envStage = Attack;
    envLevel = 0.0;
    attackRate = 0.01;
    decayRate = 0.005;
    sustainLevel = 0.7;
    releaseRate = 0.02;

    filterEnvStage = Attack;
    filterEnvLevel = 0.0;
    filterAttackRate = 0.005;
    filterDecayRate = 0.002;
    filterSustainLevel = 0.3;
    filterReleaseRate = 0.01;
    filterEnvAmount = 0.5;

    filterBaseCutoff = 2000.0 * (1.0 + velocity * 2.0);
    filterCutoff = filterBaseCutoff;
    filterResonance = 0.3;

    svfLow = 0.0; svfBand = 0.0; svfHigh = 0.0;

    lfoPhase = 0.0;
    lfoFrequency = 4.0;
    lfoDepth = 0.0;
    lfoToFilter = 0.0;
}

void SubtractiveSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity);
    if (allowTailOff)
    {
        if (envStage != Off)
            envStage = Release;
        if (filterEnvStage != Off)
            filterEnvStage = Release;
    }
    else
    {
        clearCurrentNote();
    }
}

void SubtractiveSynthVoice::applyEnvelope()
{
    switch (envStage)
    {
        case Attack:
            envLevel += attackRate;
            if (envLevel >= 1.0)
            {
                envLevel = 1.0;
                envStage = Decay;
            }
            break;
        case Decay:
            envLevel -= decayRate;
            if (envLevel <= sustainLevel)
            {
                envLevel = sustainLevel;
                envStage = Sustain;
            }
            break;
        case Release:
            envLevel -= releaseRate;
            if (envLevel <= 0.0)
            {
                envLevel = 0.0;
                envStage = Off;
                clearCurrentNote();
            }
            break;
        default:
            break;
    }
}

void SubtractiveSynthVoice::applyFilterEnvelope()
{
    switch (filterEnvStage)
    {
        case Attack:
            filterEnvLevel += filterAttackRate;
            if (filterEnvLevel >= 1.0)
            {
                filterEnvLevel = 1.0;
                filterEnvStage = Decay;
            }
            break;
        case Decay:
            filterEnvLevel -= filterDecayRate;
            if (filterEnvLevel <= filterSustainLevel)
            {
                filterEnvLevel = filterSustainLevel;
                filterEnvStage = Sustain;
            }
            break;
        case Release:
            filterEnvLevel -= filterReleaseRate;
            if (filterEnvLevel <= 0.0)
            {
                filterEnvLevel = 0.0;
                filterEnvStage = Off;
            }
            break;
        default:
            break;
    }
}

double SubtractiveSynthVoice::runSVF(double sample, double cutoffNorm, double resonance)
{
    if (cutoffNorm <= 0.0 || cutoffNorm >= 1.0)
        return sample;

    double g = std::tan(juce::MathConstants<double>::pi * cutoffNorm);
    double k = 2.0 * resonance;
    double g1 = 1.0 / (1.0 + g * (g + k));
    double g2 = g * g1;
    double g3 = g * g2;

    double hp = sample - svfLow - k * svfBand;
    double bp = g2 * hp + svfBand;
    double lp = g3 * hp + svfLow;

    svfLow = lp + g * svfBand;
    svfBand = bp + g * svfLow;

    svfLow = juce::jlimit(-1.0, 1.0, svfLow);
    svfBand = juce::jlimit(-1.0, 1.0, svfBand);
    svfHigh = juce::jlimit(-1.0, 1.0, hp);

    switch (filterType)
    {
        case 0:  return lp;
        case 1:  return bp;
        case 2:  return hp;
        default: return lp;
    }
}

float SubtractiveSynthVoice::generateSample(double ph, double, float amp, float)
{
    if (currentSampleRate <= 0.0)
        currentSampleRate = getSampleRate();

    applyEnvelope();
    applyFilterEnvelope();

    if (envStage == Off)
        return 0.0f;

    // LFO
    lfoPhase += lfoFrequency / currentSampleRate;
    if (lfoPhase >= 1.0) lfoPhase -= 1.0;
    double lfo = std::sin(lfoPhase * 2.0 * juce::MathConstants<double>::pi);

    // Pitch modulation from LFO
    double pitchMod = 1.0 + lfo * lfoDepth * 0.05;
    double modulatedPhase = ph * pitchMod;
    if (modulatedPhase >= 1.0) modulatedPhase -= 1.0;
    if (modulatedPhase < 0.0) modulatedPhase += 1.0;

    // Saw oscillator
    double osc = 2.0 * modulatedPhase - 1.0;

    // LFO → filter modulation
    double lfoFilterMod = lfo * lfoToFilter * 4000.0;

    // Envelope → filter modulation
    double filterMod = filterEnvLevel * filterEnvAmount * 8000.0;

    // Combined cutoff
    double dynamicCutoff = filterBaseCutoff + filterMod + lfoFilterMod;
    dynamicCutoff = juce::jlimit(50.0, currentSampleRate / 2.5, dynamicCutoff);

    double cutoffNorm = dynamicCutoff / (currentSampleRate / 2.0);
    if (cutoffNorm >= 1.0) cutoffNorm = 0.999;
    if (cutoffNorm <= 0.0) cutoffNorm = 0.001;

    double filtered = runSVF(osc, cutoffNorm, filterResonance);

    return amp * static_cast<float>(envLevel * filtered);
}

// ============================================================================
// SamplerSynthVoice — multi-zone WAV mapping with loop points + ADSR
// ============================================================================

SamplerSynthVoice::SamplerSynthVoice()
{
    setAttack(0.01);
    setDecay(0.1);
    setSustain(1.0);
    setRelease(0.1);
}

void SamplerSynthVoice::addZone(const SampleZone& zone)
{
    zones.push_back(zone);
}

void SamplerSynthVoice::clearZones()
{
    zones.clear();
}

int SamplerSynthVoice::getNumZones() const
{
    return static_cast<int>(zones.size());
}

const SampleZone& SamplerSynthVoice::getZone(int index) const
{
    return zones[index];
}

void SamplerSynthVoice::setAttack(double seconds)
{
    attackRate = seconds > 0.0 ? 1.0 / (seconds * getSampleRate()) : 1.0;
}

void SamplerSynthVoice::setDecay(double seconds)
{
    decayRate = seconds > 0.0 ? 1.0 / (seconds * getSampleRate()) : 1.0;
}

void SamplerSynthVoice::setSustain(double level)
{
    sustainLevel = juce::jlimit(0.0, 1.0, level);
}

void SamplerSynthVoice::setRelease(double seconds)
{
    releaseRate = seconds > 0.0 ? 1.0 / (seconds * getSampleRate()) : 1.0;
}

int SamplerSynthVoice::findZoneForNote(int midiNoteNumber) const
{
    for (int i = 0; i < static_cast<int>(zones.size()); ++i)
    {
        if (midiNoteNumber >= zones[i].lowNote && midiNoteNumber <= zones[i].highNote)
            return i;
    }
    // Fallback: find closest root note
    int best = -1;
    int bestDist = 999;
    for (int i = 0; i < static_cast<int>(zones.size()); ++i)
    {
        int dist = std::abs(midiNoteNumber - zones[i].rootNote);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

void SamplerSynthVoice::startNote(int midiNoteNumber, float velocity,
                                   juce::SynthesiserSound* sound,
                                   int currentPitchWheelPosition)
{
    juce::ignoreUnused(sound, currentPitchWheelPosition);

    int zoneIndex = findZoneForNote(midiNoteNumber);
    if (zoneIndex < 0 || zoneIndex >= static_cast<int>(zones.size()))
    {
        noteActive = false;
        return;
    }

    activeZone = zoneIndex;
    const auto& zone = zones[zoneIndex];

    if (zone.data.getNumSamples() == 0)
    {
        noteActive = false;
        return;
    }

    readPosition = 0.0;
    noteAmplitude = velocity * 0.5f * zone.gain;
    noteActive = true;

    envStage = Attack;
    envLevel = 0.0;

    double targetFreq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    double baseFreq = juce::MidiMessage::getMidiNoteInHertz(zone.rootNote);
    playbackRate = targetFreq / baseFreq;

    double sampleRateRatio = zone.sourceSampleRate / getSampleRate();
    playbackRate *= sampleRateRatio;

    if (getSampleRate() > 0.0)
    {
        setAttack(0.01);
        setDecay(0.1);
        setRelease(0.1);
    }
}

void SamplerSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity);
    if (allowTailOff && envStage != Off)
    {
        envStage = Release;
    }
    else
    {
        noteActive = false;
        clearCurrentNote();
    }
}

void SamplerSynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                         int startSample, int numSamples)
{
    if (!noteActive || activeZone < 0 || activeZone >= static_cast<int>(zones.size()))
        return;

    const auto& zone = zones[activeZone];
    if (zone.data.getNumSamples() == 0)
    {
        noteActive = false;
        return;
    }

    const int numChannels = outputBuffer.getNumChannels();
    const int numSourceChannels = zone.data.getNumChannels();
    const int totalSamples = zone.data.getNumSamples();
    const double sampleRate = getSampleRate();

    for (int i = startSample; i < startSample + numSamples; ++i)
    {
        // ADSR
        switch (envStage)
        {
            case Attack:
                envLevel += attackRate;
                if (envLevel >= 1.0) { envLevel = 1.0; envStage = Decay; }
                break;
            case Decay:
                envLevel -= decayRate;
                if (envLevel <= sustainLevel) { envLevel = sustainLevel; envStage = Sustain; }
                break;
            case Release:
                envLevel -= releaseRate;
                if (envLevel <= 0.0) { envLevel = 0.0; envStage = Off; noteActive = false; clearCurrentNote(); }
                break;
            default:
                break;
        }

        if (envStage == Off)
            break;

        int readPos = static_cast<int>(readPosition);

        // Loop handling
        if (zone.loopEnabled && zone.loopStart >= 0 && zone.loopEnd > zone.loopStart)
        {
            if (readPos >= zone.loopEnd)
                readPosition = zone.loopStart + (readPosition - zone.loopEnd);
            readPos = static_cast<int>(readPosition);
        }
        else if (readPos >= totalSamples - 1)
        {
            if (envStage == Sustain)
            {
                envStage = Release;
                continue;
            }
            noteActive = false;
            clearCurrentNote();
            break;
        }

        if (readPos >= totalSamples - 1)
        {
            noteActive = false;
            clearCurrentNote();
            break;
        }

        float frac = static_cast<float>(readPosition - readPos);
        int nextPos = std::min(readPos + 1, totalSamples - 1);

        float sampleL = zone.data.getSample(0, readPos) * (1.0f - frac)
                      + zone.data.getSample(0, nextPos) * frac;

        float sampleR = sampleL;
        if (numSourceChannels > 1)
            sampleR = zone.data.getSample(1, readPos) * (1.0f - frac)
                    + zone.data.getSample(1, nextPos) * frac;

        float env = static_cast<float>(envLevel);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s = (ch == 0) ? sampleL : sampleR;
            outputBuffer.addSample(ch, i, s * noteAmplitude * env);
        }

        readPosition += playbackRate;
    }
}

// ============================================================================
// DrumSynthVoice — algorithmic kick/snare/hihat/tom/clap
// ============================================================================

DrumSynthVoice::DrumSynthVoice() = default;

void DrumSynthVoice::setDrumType(DrumType type)
{
    drumType = type;
}

void DrumSynthVoice::setTuning(double semitones)
{
    tuningSemitones = juce::jlimit(-24.0, 24.0, semitones);
}

void DrumSynthVoice::setDecay(double factor)
{
    decayFactor = juce::jlimit(0.2, 2.0, factor);
}

void DrumSynthVoice::setDirtiness(double amount)
{
    dirtiness = juce::jlimit(0.0, 1.0, amount);
}

int DrumSynthVoice::getDefaultNote(DrumType type)
{
    switch (type)
    {
        case Kick:        return 36;
        case Snare:       return 38;
        case HihatClosed: return 42;
        case HihatOpen:   return 46;
        case Tom:         return 45;
        case Clap:        return 39;
        case Rimshot:     return 37;
        case Crash:       return 49;
    }
    return 36;
}

void DrumSynthVoice::startNote(int midiNoteNumber, float velocity,
                                juce::SynthesiserSound* sound,
                                int currentPitchWheelPosition)
{
    juce::ignoreUnused(sound, currentPitchWheelPosition);
    samplePosition = 0.0;
    vel = velocity;
    active = true;
    noteNumber = midiNoteNumber;
    resetFilter();
    noisePhase = 0.0;

    switch (drumType)
    {
        case Kick:        noteDuration = 0.4 * decayFactor;  break;
        case Snare:       noteDuration = 0.3 * decayFactor;  break;
        case HihatClosed: noteDuration = 0.1 * decayFactor;  break;
        case HihatOpen:   noteDuration = 0.3 * decayFactor;  break;
        case Tom:         noteDuration = 0.3 * decayFactor;  break;
        case Clap:        noteDuration = 0.15 * decayFactor; break;
        case Rimshot:     noteDuration = 0.1 * decayFactor;  break;
        case Crash:       noteDuration = 0.8 * decayFactor;  break;
    }
}

void DrumSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity);
    if (allowTailOff)
        active = false;
    else
        clearCurrentNote();
}

void DrumSynthVoice::resetFilter()
{
    svfLow = 0.0; svfBand = 0.0; svfHigh = 0.0;
}

double DrumSynthVoice::runSVF(double sample, double cutoff, double resonance)
{
    double g = std::tan(juce::MathConstants<double>::pi * juce::jlimit(0.0, 0.99, cutoff));
    double k = 2.0 * resonance;
    double g1 = 1.0 / (1.0 + g * (g + k));
    double g2 = g * g1;
    double g3 = g * g2;

    double hp = sample - svfLow - k * svfBand;
    double bp = g2 * hp + svfBand;
    double lp = g3 * hp + svfLow;

    svfLow = lp + g * svfBand;
    svfBand = bp + g * svfLow;

    return lp;
}

void DrumSynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                      int startSample, int numSamples)
{
    if (!active)
        return;

    const double sampleRate = getSampleRate();
    const int numChannels = outputBuffer.getNumChannels();

    for (int i = startSample; i < startSample + numSamples; ++i)
    {
        double t = samplePosition / sampleRate;
        if (t >= noteDuration)
        {
            active = false;
            clearCurrentNote();
            break;
        }

        float sample = 0.0f;
        switch (drumType)
        {
            case Kick:    sample = renderKick(t);    break;
            case Snare:   sample = renderSnare(t);   break;
            case HihatClosed:
            case HihatOpen: sample = renderHihat(t); break;
            case Tom:     sample = renderTom(t);     break;
            case Clap:    sample = renderClap(t);    break;
            case Rimshot: sample = renderRimshot(t); break;
            case Crash:   sample = renderCrash(t);   break;
        }

        for (int ch = 0; ch < numChannels; ++ch)
            outputBuffer.addSample(ch, i, sample);

        samplePosition += 1.0;
    }
}

float DrumSynthVoice::renderKick(double t)
{
    // Frequency sweep with tuning offset
    double tuningRatio = std::pow(2.0, tuningSemitones / 12.0);
    double freq = (150.0 - t * 400.0) * tuningRatio;
    freq = juce::jmax(25.0, freq);
    double env = std::exp(-t * 12.0);

    // Click at attack
    double click = (t < 0.005) ? (1.0 - t / 0.005) * 0.3 : 0.0;

    // Body with distortion
    double body = std::sin(2.0 * juce::MathConstants<double>::pi * freq * t);
    if (dirtiness > 0.0)
    {
        double drive = 1.0 + dirtiness * 8.0;
        body = std::tanh(body * drive) / std::tanh(drive);
    }

    return vel * static_cast<float>(env * body + click);
}

float DrumSynthVoice::renderSnare(double t)
{
    double tuningRatio = std::pow(2.0, tuningSemitones / 12.0);
    double freq = 200.0 * tuningRatio;
    double sineEnv = std::exp(-t * 15.0);
    double noiseEnv = std::exp(-t * 10.0);

    // Filtered noise
    double noise = 2.0 * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);
    noise += 2.0 * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);
    noise *= 0.5;

    double tone = std::sin(2.0 * juce::MathConstants<double>::pi * freq * t) * 0.5;
    double n = noiseEnv * noise * 0.5;

    // Dirtiness adds ring modulation
    if (dirtiness > 0.0)
        n += dirtiness * 0.3 * noise * sineEnv * std::sin(2.0 * juce::MathConstants<double>::pi * 80.0 * t);

    return vel * static_cast<float>(sineEnv * tone + n);
}

float DrumSynthVoice::renderHihat(double t)
{
    double env = std::exp(-t * (drumType == HihatClosed ? 40.0 : 12.0));

    // Metallic noise via multiple sine resonances
    noisePhase += 0.1;
    double metallic = std::sin(2.0 * juce::MathConstants<double>::pi * t * 2000.0 + noisePhase) * 0.3
                    + std::sin(2.0 * juce::MathConstants<double>::pi * t * 3160.0 + noisePhase * 1.7) * 0.2
                    + std::sin(2.0 * juce::MathConstants<double>::pi * t * 4200.0 + noisePhase * 2.3) * 0.15
                    + std::sin(2.0 * juce::MathConstants<double>::pi * t * 5800.0 + noisePhase * 3.1) * 0.1;

    double noise = 2.0 * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);
    double signal = metallic * 0.6 + noise * 0.4;

    // Dirtiness adds distortion
    if (dirtiness > 0.0)
    {
        double drive = 1.0 + dirtiness * 5.0;
        signal = std::tanh(signal * drive) / std::tanh(drive);
    }

    return vel * static_cast<float>(env * signal * 0.5);
}

float DrumSynthVoice::renderTom(double t)
{
    double tuningRatio = std::pow(2.0, tuningSemitones / 12.0);
    double freq = (120.0 - t * 200.0) * tuningRatio;
    freq = juce::jmax(35.0, freq);
    double env = std::exp(-t * 8.0);

    double tone = std::sin(2.0 * juce::MathConstants<double>::pi * freq * t);

    // Add harmonic richness
    tone += 0.3 * std::sin(2.0 * juce::MathConstants<double>::pi * freq * 2.0 * t);

    return vel * static_cast<float>(env * tone * 0.8);
}

float DrumSynthVoice::renderClap(double t)
{
    double env = std::exp(-t * 20.0);
    double noise = 2.0 * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);

    // Initial transient burst (multiple mini-pulses)
    double transient = 0.0;
    for (int p = 0; p < 5; ++p)
    {
        double pulseTime = t - p * 0.002;
        if (pulseTime > 0.0 && pulseTime < 0.003)
            transient += (1.0 - pulseTime / 0.003) * 0.2;
    }

    double signal = env * noise * 0.6 + transient * 0.4;
    return vel * static_cast<float>(signal);
}

float DrumSynthVoice::renderRimshot(double t)
{
    double env = std::exp(-t * 30.0);
    double noise = 2.0 * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);
    double tone = std::sin(2.0 * juce::MathConstants<double>::pi * 800.0 * t) * 0.3;
    return vel * static_cast<float>(env * (noise * 0.5 + tone));
}

float DrumSynthVoice::renderCrash(double t)
{
    double env = std::exp(-t * 3.0);
    double noise = 2.0 * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);

    double metallic = std::sin(2.0 * juce::MathConstants<double>::pi * 200.0 * t) * 0.3
                    + std::sin(2.0 * juce::MathConstants<double>::pi * 400.0 * t) * 0.2
                    + std::sin(2.0 * juce::MathConstants<double>::pi * 600.0 * t) * 0.1
                    + std::sin(2.0 * juce::MathConstants<double>::pi * 800.0 * t) * 0.08;

    double signal = noise * 0.4 + metallic;
    return vel * static_cast<float>(env * signal);
}

} // namespace harmonic_engine
