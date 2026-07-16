#include "harmonic_engine/AudioEngine/Transport.h"

namespace harmonic_engine
{

Transport::Transport()
{
}

Transport::~Transport()
{
}

void Transport::prepareToPlay(double newSampleRate, int newSamplesPerBlock)
{
    const juce::SpinLock::ScopedLockType lock(positionLock);
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;
}

void Transport::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    ignoreUnused(midiMessages);

    if (playing)
    {
        const juce::SpinLock::ScopedLockType lock(positionLock);
        int64_t prevPosition = playheadPosition;
        playheadPosition += buffer.getNumSamples();

        if (looping)
        {
            int64_t loopStartSamples = static_cast<int64_t>(loopStartSeconds * sampleRate);
            int64_t loopEndSamples = static_cast<int64_t>(loopEndSeconds * sampleRate);
            int64_t loopLength = loopEndSamples - loopStartSamples;

            if (loopLength > 0 && playheadPosition >= loopEndSamples)
            {
                int wraps = static_cast<int>((playheadPosition - loopStartSamples) / loopLength);
                loopPassCount += wraps;
                playheadPosition = loopStartSamples + ((playheadPosition - loopStartSamples) % loopLength);

                if (onLoopPass)
                    onLoopPass(loopPassCount);
            }
        }

        if (onPositionChanged)
            onPositionChanged();
    }
}

void Transport::start()
{
    {
        const juce::SpinLock::ScopedLockType lock(positionLock);
        playing = true;
        recording = false;
        paused = false;
    }

    if (onStateChanged)
        onStateChanged();
}

void Transport::stop()
{
    {
        const juce::SpinLock::ScopedLockType lock(positionLock);
        playing = false;
        recording = false;
        paused = false;
        loopPassCount = 0;
    }

    if (onStateChanged)
        onStateChanged();
}

void Transport::pause()
{
    {
        const juce::SpinLock::ScopedLockType lock(positionLock);
        paused = !paused;
        playing = !paused;
    }

    if (onStateChanged)
        onStateChanged();
}

void Transport::record()
{
    {
        const juce::SpinLock::ScopedLockType lock(positionLock);
        recording = true;
        playing = true;
        paused = false;
    }

    if (onStateChanged)
        onStateChanged();
}

bool Transport::isPlaying() const
{
    const juce::SpinLock::ScopedLockType lock(positionLock);
    return playing;
}

bool Transport::isRecording() const
{
    const juce::SpinLock::ScopedLockType lock(positionLock);
    return recording;
}

bool Transport::isPaused() const
{
    const juce::SpinLock::ScopedLockType lock(positionLock);
    return paused;
}

double Transport::getPositionInSeconds() const
{
    const juce::SpinLock::ScopedLockType lock(positionLock);
    return static_cast<double>(playheadPosition) / sampleRate;
}

int64_t Transport::getPositionInSamples() const
{
    const juce::SpinLock::ScopedLockType lock(positionLock);
    return playheadPosition;
}

void Transport::setPositionInSamples(int64_t position)
{
    {
        const juce::SpinLock::ScopedLockType lock(positionLock);
        playheadPosition = position;
    }

    if (onPositionChanged)
        onPositionChanged();
}

void Transport::setPositionInSeconds(double seconds)
{
    setPositionInSamples(static_cast<int64_t>(seconds * sampleRate));
}

double Transport::getTempo() const
{
    return tempo;
}

void Transport::setTempo(double bpm)
{
    tempo = bpm;
}

double Transport::getLoopStart() const { return loopStartSeconds; }
double Transport::getLoopEnd() const { return loopEndSeconds; }

void Transport::setLoopPoints(double startSeconds, double endSeconds)
{
    loopStartSeconds = juce::jmax(0.0, startSeconds);
    loopEndSeconds = juce::jmax(loopStartSeconds + 0.1, endSeconds);
}

bool Transport::isLooping() const { return looping; }
void Transport::setLooping(bool shouldLoop) { looping = shouldLoop; }

int Transport::getLoopPassCount() const { return loopPassCount; }
void Transport::resetLoopPassCount() { loopPassCount = 0; }
void Transport::setLoopPassCallback(std::function<void(int)> callback) { onLoopPass = callback; }

int Transport::getSampleRate() const
{
    return static_cast<int>(sampleRate);
}

} // namespace harmonic_engine
