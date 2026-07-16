#include "harmonic_engine/AudioEngine/AudioRecorder.h"

namespace harmonic_engine
{

AudioRecorder::AudioRecorder(juce::TimeSliceThread& backgroundThread)
    : backgroundThread(backgroundThread)
{
}

AudioRecorder::~AudioRecorder()
{
    stopRecording();
}

void AudioRecorder::startRecording(const juce::File& file,
                                   double sampleRate,
                                   int numChannels)
{
    stopRecording();

    currentFile = file;
    currentSampleRate = sampleRate;
    currentNumChannels = numChannels;

    juce::ScopedLock lock(writerLock);

    auto options = juce::AudioFormatWriterOptions{}
        .withSampleRate(sampleRate)
        .withNumChannels(static_cast<int>(numChannels))
        .withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);

    std::unique_ptr<juce::OutputStream> stream = file.createOutputStream();
    if (stream == nullptr) return;

    auto writer = wavFormat.createWriterFor(stream, options);

    if (writer != nullptr)
    {
        threadedWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
            writer.release(), backgroundThread, 32768);
        activeWriter = threadedWriter.get();
        nextSampleNum = 0;
        writer.release();
    }
}

void AudioRecorder::stopRecording()
{
    juce::AudioFormatWriter::ThreadedWriter* writerActive = nullptr;

    {
        juce::ScopedLock lock(writerLock);
        writerActive = activeWriter.exchange(nullptr);
    }

    if (writerActive != nullptr)
    {
        threadedWriter.reset();
    }

    currentFile = juce::File();
    nextSampleNum = 0;
}

bool AudioRecorder::isRecording() const
{
    return activeWriter.load() != nullptr;
}

juce::File AudioRecorder::getCurrentFile() const
{
    return currentFile;
}

void AudioRecorder::writeBlock(const float* const* inputChannelData,
                               int numInputChannels,
                               int numSamples)
{
    auto* writer = activeWriter.load();
    if (writer != nullptr)
    {
        writer->write(inputChannelData, numSamples);
        nextSampleNum += numSamples;
    }
}

juce::int64 AudioRecorder::getNextSampleNum() const
{
    return nextSampleNum;
}

} // namespace harmonic_engine
