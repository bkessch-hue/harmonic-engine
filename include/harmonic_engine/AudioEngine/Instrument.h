#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <array>
#include <vector>

namespace harmonic_engine
{

class HarmonicSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int midiNoteNumber) override { juce::ignoreUnused(midiNoteNumber); return true; }
    bool appliesToChannel(int midiChannel) override { juce::ignoreUnused(midiChannel); return true; }
};

class HarmonicSynthVoice : public juce::SynthesiserVoice
{
public:
    HarmonicSynthVoice() = default;
    ~HarmonicSynthVoice() override = default;

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        juce::ignoreUnused(sound);
        return true;
    }

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override
    {
        juce::ignoreUnused(sound, currentPitchWheelPosition);
        frequency = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
        amplitude = velocity * 0.5f;
        tailOff = 0.0;
        phase = 0.0;
        phase2 = 0.0;
    }

    void stopNote(float velocity, bool allowTailOff) override
    {
        juce::ignoreUnused(velocity);
        if (allowTailOff)
            tailOff = 1.0;
        else
            clearCurrentNote();
    }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override
    {
        if (frequency <= 0.0f) return;

        const double sampleRate = getSampleRate();
        if (sampleRate <= 0.0) return;

        const double phaseIncrement = frequency / sampleRate;

        for (int i = startSample; i < startSample + numSamples; ++i)
        {
            float sample = generateSample(phase, phase2, amplitude, tailOff);

            if (tailOff > 0.0 && tailOff < 1.0)
            {
                sample *= tailOff;
                tailOff -= 0.005;
                if (tailOff <= 0.0)
                {
                    clearCurrentNote();
                    break;
                }
            }

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample(ch, i, sample);

            phase += phaseIncrement;
            phase2 += phaseIncrement * 2.0;
            if (phase >= 1.0) phase -= 1.0;
            if (phase2 >= 1.0) phase2 -= 1.0;
        }
    }

    void pitchWheelMoved(int newPitchWheelValue) override { juce::ignoreUnused(newPitchWheelValue); }
    void controllerMoved(int controllerNumber, int newControllerValue) override { juce::ignoreUnused(controllerNumber, newControllerValue); }

protected:
    float frequency = 0.0f;
    float amplitude = 0.0f;
    float tailOff = 0.0f;
    double phase = 0.0;
    double phase2 = 0.0;

    virtual float generateSample(double ph, double ph2, float amp, float tail) = 0;
};

class SineVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double, float amp, float) override
    {
        return amp * static_cast<float>(std::sin(2.0 * 3.14159265 * ph));
    }
};

class SawVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double, float amp, float) override
    {
        return amp * static_cast<float>(2.0 * ph - 1.0);
    }
};

class SquareVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double, float amp, float) override
    {
        return amp * (ph < 0.5 ? 1.0f : -1.0f);
    }
};

class TriangleVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double, float amp, float) override
    {
        float val = static_cast<float>(ph < 0.5 ? 4.0 * ph - 1.0 : 3.0 - 4.0 * ph);
        return amp * val;
    }
};

class FMSynthVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double ph2, float amp, float) override
    {
        float modulator = static_cast<float>(std::sin(2.0 * 3.14159265 * ph2 * 3.0)) * 0.3f;
        float carrier = static_cast<float>(std::sin(2.0 * 3.14159265 * (ph + modulator)));
        return amp * carrier;
    }
};

class ElectricPianoVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double, float amp, float) override
    {
        float fundamental = static_cast<float>(std::sin(2.0 * 3.14159265 * ph));
        float h2 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph * 2.0)) * 0.5f;
        float h3 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph * 3.0)) * 0.25f;
        float h5 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph * 5.0)) * 0.1f;
        return amp * (fundamental + h2 + h3 + h5) * 0.5f;
    }
};

class OrganVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double, float amp, float) override
    {
        float h1 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph));
        float h2 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph * 2.0)) * 0.8f;
        float h3 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph * 3.0)) * 0.6f;
        float h4 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph * 4.0)) * 0.4f;
        float h6 = static_cast<float>(std::sin(2.0 * 3.14159265 * ph * 6.0)) * 0.2f;
        return amp * (h1 + h2 + h3 + h4 + h6) * 0.3f;
    }
};

class PluckVoice : public HarmonicSynthVoice
{
protected:
    float generateSample(double ph, double, float amp, float tail) override
    {
        float noise = static_cast<float>(std::sin(2.0 * 3.14159265 * ph) *
                        (1.0 + 0.5 * std::sin(2.0 * 3.14159265 * ph * 7.0)));
        float body = amp * noise;
        float brightness = tail > 0.5f ? 1.0f : tail * 2.0f;
        return body * brightness;
    }
};

// ============================================================================
// WavetableSynthVoice — 8 waveforms with smooth crossfade morphing
// ============================================================================

class WavetableSynthVoice : public HarmonicSynthVoice
{
public:
    WavetableSynthVoice();
    void setMorphPosition(float morph);      // 0..1 blends between adjacent waveforms
    void setWaveformOffset(int offset);       // snap to a specific waveform (0-7)
    void setPhaseDistortion(float amount);    // 0..1 waveshaping distortion

protected:
    float generateSample(double ph, double, float amp, float) override;

private:
    static constexpr int TableSize = 2048;
    static constexpr int NumWaveforms = 8;

    enum Waveform { Sine, Saw, Square, Triangle, Pulse25, Noise, BrightSaw, OrganLike };

    std::array<std::vector<float>, NumWaveforms> tables;
    float morphPosition = 0.0f;
    float phaseDistortion = 0.0f;

    void generateTable(Waveform wf, std::vector<float>& table);
};

// ============================================================================
// SubtractiveSynthVoice — SVF filter + ADSR + filter envelope + LFO
// ============================================================================

class SubtractiveSynthVoice : public HarmonicSynthVoice
{
public:
    SubtractiveSynthVoice();

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;

    void setFilterCutoff(double hz);
    void setFilterResonance(double q);
    void setFilterType(int type);             // 0=LP, 1=BP, 2=HP
    void setFilterEnvAmount(double amount);   // 0..1 envelope→cutoff modulation

    void setAttack(double seconds);
    void setDecay(double seconds);
    void setSustain(double level);            // 0..1
    void setRelease(double seconds);
    void setFilterAttack(double seconds);
    void setFilterDecay(double seconds);
    void setFilterSustain(double level);
    void setFilterRelease(double seconds);

    void setLFOFrequency(double hz);
    void setLFODepth(double depth);           // 0..1 pitch modulation
    void setLFOToFilter(double amount);       // 0..1 filter cutoff modulation

protected:
    float generateSample(double ph, double, float amp, float) override;

private:
    enum EnvStage { Off, Attack, Decay, Sustain, Release };

    // Amp envelope
    EnvStage envStage = Off;
    double envLevel = 0.0;
    double attackRate = 0.0;
    double decayRate = 0.0;
    double sustainLevel = 0.7;
    double releaseRate = 0.0;

    // Filter envelope
    EnvStage filterEnvStage = Off;
    double filterEnvLevel = 0.0;
    double filterAttackRate = 0.0;
    double filterDecayRate = 0.0;
    double filterSustainLevel = 0.5;
    double filterReleaseRate = 0.0;
    double filterEnvAmount = 0.5;

    // Filter
    int filterType = 0;                       // 0=LP, 1=BP, 2=HP
    double filterCutoff = 2000.0;
    double filterResonance = 0.3;
    double filterBaseCutoff = 2000.0;
    double cutoffMod = 0.0;

    // Per-voice filter state (SVF)
    double svfLow = 0.0, svfBand = 0.0, svfHigh = 0.0;

    // LFO
    double lfoPhase = 0.0;
    double lfoFrequency = 4.0;
    double lfoDepth = 0.0;
    double lfoToFilter = 0.0;

    double currentSampleRate = 44100.0;

    void applyEnvelope();
    void applyFilterEnvelope();
    double runSVF(double sample, double cutoffNormalized, double resonance);
};

// ============================================================================
// SamplerSynthVoice — multi-zone WAV mapping with loop points + ADSR
// ============================================================================

struct SampleZone
{
    juce::AudioBuffer<float> data;
    double sourceSampleRate = 44100.0;
    int rootNote = 60;
    int lowNote = 0;
    int highNote = 127;
    int loopStart = -1;
    int loopEnd = -1;
    bool loopEnabled = false;
    float gain = 1.0f;
};

class SamplerSynthVoice : public HarmonicSynthVoice
{
public:
    SamplerSynthVoice();

    void addZone(const SampleZone& zone);
    void clearZones();
    int getNumZones() const;
    const SampleZone& getZone(int index) const;

    void setAttack(double seconds);
    void setDecay(double seconds);
    void setSustain(double level);
    void setRelease(double seconds);

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override;

protected:
    float generateSample(double, double, float, float) override { return 0.0f; }

private:
    std::vector<SampleZone> zones;
    int activeZone = -1;
    double readPosition = 0.0;
    double playbackRate = 1.0;
    float noteAmplitude = 0.0f;
    bool noteActive = false;

    // ADSR
    enum EnvStage { Off, Attack, Decay, Sustain, Release };
    EnvStage envStage = Off;
    double envLevel = 0.0;
    double attackRate = 0.0;
    double decayRate = 0.0;
    double sustainLevel = 1.0;
    double releaseRate = 0.0;

    int findZoneForNote(int midiNoteNumber) const;
};

// ============================================================================
// DrumSynthVoice — algorithmic kick/snare/hihat/tom/clap
// ============================================================================

class DrumSynthVoice : public HarmonicSynthVoice
{
public:
    enum DrumType { Kick, Snare, HihatClosed, HihatOpen, Tom, Clap, Rimshot, Crash };

    DrumSynthVoice();

    void setDrumType(DrumType type);
    DrumType getDrumType() const { return drumType; }
    int getMidiNote() const { return midiNote; }

    void setTuning(double semitones);
    void setDecay(double factor);              // 0.2..2.0 time stretch
    void setDirtiness(double amount);          // 0..1 noise/chaos

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override;

    static int getDefaultNote(DrumType type);

protected:
    float generateSample(double, double, float, float) override { return 0.0f; }

private:
    DrumType drumType = Kick;
    int midiNote = 36;
    double samplePosition = 0.0;
    double noteDuration = 0.5;
    bool active = false;
    float vel = 0.8f;
    int noteNumber = 36;

    double tuningSemitones = 0.0;
    double decayFactor = 1.0;
    double dirtiness = 0.0;

    double svfLow = 0.0, svfBand = 0.0, svfHigh = 0.0;
    double noisePhase = 0.0;

    void resetFilter();
    double runSVF(double sample, double cutoff, double resonance);
    float renderKick(double t);
    float renderSnare(double t);
    float renderHihat(double t);
    float renderTom(double t);
    float renderClap(double t);
    float renderRimshot(double t);
    float renderCrash(double t);
};

enum class InstrumentType
{
    Sine,
    Saw,
    Square,
    Triangle,
    FM,
    ElectricPiano,
    Organ,
    Pluck,
    Wavetable,
    Subtractive,
    Sampler,
    Drum
};

} // namespace harmonic_engine
