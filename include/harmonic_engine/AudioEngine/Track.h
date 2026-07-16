#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <vector>
#include <memory>
#include "harmonic_engine/AudioEngine/AudioClip.h"
#include "harmonic_engine/AudioEngine/MidiClip.h"
#include "harmonic_engine/AudioEngine/Instrument.h"
#include "harmonic_engine/AudioEngine/EffectBase.h"

namespace harmonic_engine
{

class SampleLoader;

class Track
{
public:
    Track(int trackNumber);
    ~Track();

    int getTrackNumber() const;
    void setTrackNumber(int number);

    juce::String getName() const;
    void setName(const juce::String& newName);

    float getGain() const;
    void setGain(float newGain);

    float getPan() const;
    void setPan(float newPan);

    bool isMuted() const;
    void setMuted(bool muted);

    bool isSoloed() const;
    void setSoloed(bool soloed);

    bool isRecordArmed() const;
    void setRecordArmed(bool armed);

    juce::Colour getTrackColour() const;
    void setTrackColour(juce::Colour colour);

    float getPeakLevel() const;
    void updatePeakLevel(float level);

    bool isMidiTrack() const;
    void setMidiTrack(bool isMidi);
    InstrumentType getInstrumentType() const;
    void setInstrument(InstrumentType type);

    void addClip(std::shared_ptr<AudioClip> clip);
    void removeClip(const juce::String& clipName);
    void clearClips();
    int getNumClips() const;
    std::shared_ptr<AudioClip> getClip(int index) const;
    std::shared_ptr<AudioClip> getClipAtTime(double timeSeconds) const;

    void addMidiClip(std::shared_ptr<MidiClip> clip);
    void removeMidiClip(int index);
    void clearMidiClips();
    int getNumMidiClips() const;
    std::shared_ptr<MidiClip> getMidiClip(int index) const;

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& outputBuffer,
                      juce::MidiBuffer& midiMessages,
                      double positionInSeconds);
    void processMidiBlock(juce::AudioBuffer<float>& outputBuffer,
                          juce::MidiBuffer& midiMessages,
                          double positionInSeconds);
    void releaseResources();

    const juce::AudioBuffer<float>& getOutputBuffer() const;

    std::shared_ptr<AudioClip> importAudioFile(const juce::File& file, double timelinePosition = 0.0);
    bool canImportFile(const juce::File& file);

    void setSampleLoader(SampleLoader* loader);

    void splitClip(int clipIndex, double splitTimeSeconds);
    void deleteClip(int clipIndex);
    void moveClip(int clipIndex, double newStartTime);

    std::function<void()> onTrackChanged;
    std::function<void()> onClipsChanged;

    EffectChain& getEffectChain();
    float getBusSendLevel(int busIndex) const;
    void setBusSendLevel(int busIndex, float level);

private:
    int trackNumber;
    juce::String name;
    float gain = 1.0f;
    float pan = 0.0f;
    bool muted = false;
    bool soloed = false;
    bool recordArmed = false;
    bool midiTrack = false;
    InstrumentType instrumentType = InstrumentType::Sine;
    juce::Colour trackColour;

    float peakLevel = 0.0f;

    std::atomic<float> atomicGain{ 1.0f };
    std::atomic<float> atomicPan{ 0.0f };
    std::atomic<bool> atomicMuted{ false };
    std::atomic<bool> atomicSoloed{ false };

    double currentSampleRate = 44100.0;
    int currentSamplesPerBlock = 512;

    std::vector<std::shared_ptr<AudioClip>> clips;
    std::vector<std::shared_ptr<MidiClip>> midiClips;
    juce::AudioBuffer<float> outputBuffer;
    juce::AudioBuffer<float> tempClipBuffer;

    EffectChain effectChain;
    float busSendA = 0.0f;
    float busSendB = 0.0f;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::Synthesiser> synth;
    SampleLoader* sampleLoader = nullptr;

    static juce::Colour getTrackColourForNumber(int number);
    void renderClipBlock(AudioClip& clip, juce::AudioBuffer<float>& destBuffer,
                         int destStartSample, int numSamples, double positionInSeconds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Track)
};

} // namespace harmonic_engine
