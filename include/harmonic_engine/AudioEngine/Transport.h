#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace harmonic_engine
{

class Transport
{
public:
    Transport();
    ~Transport();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    void start();
    void stop();
    void pause();
    void record();

    bool isPlaying() const;
    bool isRecording() const;
    bool isPaused() const;

    double getPositionInSeconds() const;
    int64_t getPositionInSamples() const;
    void setPositionInSamples(int64_t position);
    void setPositionInSeconds(double seconds);

    double getTempo() const;
    void setTempo(double bpm);

    int getSampleRate() const;

    double getLoopStart() const;
    double getLoopEnd() const;
    void setLoopPoints(double startSeconds, double endSeconds);
    bool isLooping() const;
    void setLooping(bool shouldLoop);
    int getLoopPassCount() const;
    void resetLoopPassCount();
    void setLoopPassCallback(std::function<void(int pass)> callback);

    std::function<void()> onPositionChanged;
    std::function<void()> onStateChanged;

private:
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    int64_t playheadPosition = 0;

    bool playing = false;
    bool recording = false;
    bool paused = false;

    double tempo = 120.0;

    double loopStartSeconds = 0.0;
    double loopEndSeconds = 4.0;
    bool looping = false;
    int loopPassCount = 0;
    std::function<void(int)> onLoopPass;

    mutable juce::SpinLock positionLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Transport)
};

} // namespace harmonic_engine
