#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace harmonic_engine
{

class AudioRecorder
{
public:
    AudioRecorder(juce::TimeSliceThread& backgroundThread);
    ~AudioRecorder();

    void startRecording(const juce::File& file,
                        double sampleRate,
                        int numChannels);

    void stopRecording();

    bool isRecording() const;
    juce::File getCurrentFile() const;

    void writeBlock(const float* const* inputChannelData,
                    int numInputChannels,
                    int numSamples);

    juce::int64 getNextSampleNum() const;

private:
    juce::TimeSliceThread& backgroundThread;

    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };

    juce::WavAudioFormat wavFormat;
    juce::File currentFile;
    juce::int64 nextSampleNum = 0;

    double currentSampleRate = 44100.0;
    int currentNumChannels = 2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecorder)
};

} // namespace harmonic_engine
