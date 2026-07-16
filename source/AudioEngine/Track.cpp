#include "harmonic_engine/AudioEngine/Track.h"
#include "harmonic_engine/AudioEngine/InstrumentManager.h"
#include "harmonic_engine/AudioEngine/SampleLoader.h"
#include <algorithm>

namespace harmonic_engine
{

Track::Track(int number)
    : trackNumber(number),
      name("Track " + juce::String(number)),
      trackColour(getTrackColourForNumber(number))
{
    formatManager.registerBasicFormats();
    atomicGain.store(gain);
    atomicPan.store(pan);
    atomicMuted.store(muted);
    atomicSoloed.store(soloed);
}

Track::~Track()
{
}

int Track::getTrackNumber() const
{
    return trackNumber;
}

void Track::setTrackNumber(int number)
{
    trackNumber = number;
}

juce::String Track::getName() const
{
    return name;
}

void Track::setName(const juce::String& newName)
{
    name = newName;
    if (onTrackChanged) onTrackChanged();
}

float Track::getGain() const
{
    return atomicGain.load();
}

void Track::setGain(float newGain)
{
    gain = juce::jlimit(0.0f, 2.0f, newGain);
    atomicGain.store(gain);
    if (onTrackChanged) onTrackChanged();
}

float Track::getPan() const
{
    return atomicPan.load();
}

void Track::setPan(float newPan)
{
    pan = juce::jlimit(-1.0f, 1.0f, newPan);
    atomicPan.store(pan);
    if (onTrackChanged) onTrackChanged();
}

bool Track::isMuted() const
{
    return atomicMuted.load();
}

void Track::setMuted(bool muted)
{
    this->muted = muted;
    atomicMuted.store(muted);
    if (onTrackChanged) onTrackChanged();
}

bool Track::isSoloed() const
{
    return atomicSoloed.load();
}

void Track::setSoloed(bool soloed)
{
    this->soloed = soloed;
    atomicSoloed.store(soloed);
    if (onTrackChanged) onTrackChanged();
}

bool Track::isRecordArmed() const
{
    return recordArmed;
}

void Track::setRecordArmed(bool armed)
{
    recordArmed = armed;
    if (onTrackChanged) onTrackChanged();
}

juce::Colour Track::getTrackColour() const
{
    return trackColour;
}

void Track::setTrackColour(juce::Colour colour)
{
    trackColour = colour;
    if (onTrackChanged) onTrackChanged();
}

float Track::getPeakLevel() const
{
    return peakLevel;
}

void Track::updatePeakLevel(float level)
{
    peakLevel = std::max(peakLevel * 0.999f, level);
}

bool Track::isMidiTrack() const
{
    return midiTrack;
}

void Track::setMidiTrack(bool isMidi)
{
    midiTrack = isMidi;
    if (onTrackChanged) onTrackChanged();
}

InstrumentType Track::getInstrumentType() const
{
    return instrumentType;
}

void Track::setInstrument(InstrumentType type)
{
    instrumentType = type;

    auto& manager = InstrumentManager::getInstance();
    synth = manager.createInstrument(type);
    if (synth != nullptr)
        synth->setCurrentPlaybackSampleRate(currentSampleRate);

    if (onTrackChanged) onTrackChanged();
}

void Track::setSampleLoader(SampleLoader* loader)
{
    sampleLoader = loader;
}

void Track::addClip(std::shared_ptr<AudioClip> clip)
{
    clips.push_back(std::move(clip));
    if (onClipsChanged) onClipsChanged();
}

void Track::removeClip(const juce::String& clipName)
{
    auto it = std::find_if(clips.begin(), clips.end(),
                           [&clipName](const std::shared_ptr<AudioClip>& c) {
                               return c->getName() == clipName;
                           });
    if (it != clips.end())
    {
        clips.erase(it);
        if (onClipsChanged) onClipsChanged();
    }
}

void Track::clearClips()
{
    clips.clear();
    if (onClipsChanged) onClipsChanged();
}

int Track::getNumClips() const
{
    return static_cast<int>(clips.size());
}

std::shared_ptr<AudioClip> Track::getClip(int index) const
{
    if (index >= 0 && index < static_cast<int>(clips.size()))
        return clips[index];
    return nullptr;
}

std::shared_ptr<AudioClip> Track::getClipAtTime(double timeSeconds) const
{
    for (const auto& clip : clips)
    {
        if (clip->containsTime(timeSeconds))
            return clip;
    }
    return nullptr;
}

void Track::addMidiClip(std::shared_ptr<MidiClip> clip)
{
    midiClips.push_back(std::move(clip));
    if (onClipsChanged) onClipsChanged();
}

void Track::removeMidiClip(int index)
{
    if (index >= 0 && index < static_cast<int>(midiClips.size()))
    {
        midiClips.erase(midiClips.begin() + index);
        if (onClipsChanged) onClipsChanged();
    }
}

void Track::clearMidiClips()
{
    midiClips.clear();
    if (onClipsChanged) onClipsChanged();
}

int Track::getNumMidiClips() const
{
    return static_cast<int>(midiClips.size());
}

std::shared_ptr<MidiClip> Track::getMidiClip(int index) const
{
    if (index >= 0 && index < static_cast<int>(midiClips.size()))
        return midiClips[index];
    return nullptr;
}

void Track::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;

    outputBuffer.setSize(2, samplesPerBlock);
    tempClipBuffer.setSize(2, samplesPerBlock);

    if (synth != nullptr)
        synth->setCurrentPlaybackSampleRate(sampleRate);

    effectChain.prepareToPlay(sampleRate, samplesPerBlock);
}

void Track::processBlock(juce::AudioBuffer<float>& destBuffer,
                         juce::MidiBuffer& midiMessages,
                         double positionInSeconds)
{
    const int numSamples = destBuffer.getNumSamples();
    const int numChannels = destBuffer.getNumChannels();

    outputBuffer.setSize(numChannels, numSamples, false, false, true);
    outputBuffer.clear();

    if (atomicMuted.load())
    {
        updatePeakLevel(0.0f);
        return;
    }

    if (midiTrack && synth != nullptr)
    {
        processMidiBlock(outputBuffer, midiMessages, positionInSeconds);
    }
    else
    {
        for (auto& clip : clips)
        {
            if (clip == nullptr || clip->isMuted())
                continue;

            double clipStart = clip->getTimelineStart();
            double clipEnd = clip->getTimelineEnd();

            if (positionInSeconds + (static_cast<double>(numSamples) / currentSampleRate) < clipStart)
                continue;
            if (positionInSeconds > clipEnd)
                continue;

            renderClipBlock(*clip, outputBuffer, 0, numSamples, positionInSeconds);
        }
    }

    float currentGain = atomicGain.load();
    float currentPan = atomicPan.load();
    float leftGain = currentGain * std::sqrt(0.5f * (1.0f + currentPan));
    float rightGain = currentGain * std::sqrt(0.5f * (1.0f - currentPan));

    float maxPeak = 0.0f;

    if (numChannels > 0)
    {
        outputBuffer.applyGain(0, 0, numSamples, leftGain);
        maxPeak = std::max(maxPeak, outputBuffer.getMagnitude(0, 0, numSamples));
    }

    if (numChannels > 1)
    {
        outputBuffer.applyGain(1, 0, numSamples, rightGain);
        maxPeak = std::max(maxPeak, outputBuffer.getMagnitude(1, 0, numSamples));
    }

    updatePeakLevel(maxPeak);

    effectChain.processBlock(outputBuffer, numSamples);
}

EffectChain& Track::getEffectChain()
{
    return effectChain;
}

float Track::getBusSendLevel(int busIndex) const
{
    if (busIndex == 0) return busSendA;
    if (busIndex == 1) return busSendB;
    return 0.0f;
}

void Track::setBusSendLevel(int busIndex, float level)
{
    if (busIndex == 0) busSendA = juce::jlimit(0.0f, 1.0f, level);
    if (busIndex == 1) busSendB = juce::jlimit(0.0f, 1.0f, level);
    if (onTrackChanged) onTrackChanged();
}

void Track::processMidiBlock(juce::AudioBuffer<float>& outputBuffer,
                             juce::MidiBuffer& midiMessages,
                             double positionInSeconds)
{
    if (synth == nullptr) return;

    const int numSamples = outputBuffer.getNumSamples();
    juce::MidiBuffer trackMidi;

    for (int c = 0; c < static_cast<int>(midiClips.size()); ++c)
    {
        auto& midiClip = midiClips[c];
        if (midiClip == nullptr || midiClip->isMuted())
            continue;

        double clipStart = midiClip->getTimelineStart();
        double clipEnd = clipStart + midiClip->getClipDuration();

        if (positionInSeconds + (static_cast<double>(numSamples) / currentSampleRate) < clipStart)
            continue;
        if (positionInSeconds > clipEnd)
            continue;

        midiClip->renderMidiBuffer(trackMidi, positionInSeconds, numSamples, currentSampleRate);
    }

    synth->renderNextBlock(outputBuffer, trackMidi, 0, numSamples);
}

void Track::releaseResources()
{
    outputBuffer.setSize(0, 0);
    tempClipBuffer.setSize(0, 0);
    peakLevel = 0.0f;
}

const juce::AudioBuffer<float>& Track::getOutputBuffer() const
{
    return outputBuffer;
}

std::shared_ptr<AudioClip> Track::importAudioFile(const juce::File& file, double timelinePosition)
{
    if (!file.existsAsFile())
        return nullptr;

    if (sampleLoader != nullptr)
    {
        auto* info = sampleLoader->load(file);
        if (info == nullptr)
            return nullptr;

        double fileLength = static_cast<double>(info->lengthInSamples) / info->sampleRate;

        auto clip = std::make_shared<AudioClip>(
            file.getFileNameWithoutExtension(),
            file,
            timelinePosition,
            0.0,
            fileLength,
            info->sampleRate);

        clip->setColour(getTrackColourForNumber(trackNumber));
        addClip(clip);

        return clip;
    }

    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
        return nullptr;

    double fileLength = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    double sampleRate = reader->sampleRate;

    delete reader;

    auto clip = std::make_shared<AudioClip>(
        file.getFileNameWithoutExtension(),
        file,
        timelinePosition,
        0.0,
        fileLength,
        sampleRate);

    clip->setColour(getTrackColourForNumber(trackNumber));
    addClip(clip);

    return clip;
}

bool Track::canImportFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
        return false;
    delete reader;
    return true;
}

void Track::splitClip(int clipIndex, double splitTimeSeconds)
{
    if (clipIndex < 0 || clipIndex >= static_cast<int>(clips.size()))
        return;

    auto& clip = clips[clipIndex];
    double clipStart = clip->getTimelineStart();
    double clipDuration = clip->getClipDuration();
    double clipEnd = clipStart + clipDuration;

    if (splitTimeSeconds <= clipStart || splitTimeSeconds >= clipEnd)
        return;

    double splitOffset = splitTimeSeconds - clipStart;

    auto rightClip = std::make_shared<AudioClip>(
        clip->getName() + " (R)",
        clip->getSourceFile(),
        splitTimeSeconds,
        clip->getSourceOffset() + splitOffset,
        clipDuration - splitOffset,
        44100.0);
    rightClip->setGain(static_cast<float>(clip->getGain()));
    rightClip->setColour(clip->getColour());

    clip->setClipDuration(splitOffset);

    clips.insert(clips.begin() + clipIndex + 1, std::move(rightClip));

    if (onClipsChanged) onClipsChanged();
}

void Track::deleteClip(int clipIndex)
{
    if (clipIndex >= 0 && clipIndex < static_cast<int>(clips.size()))
    {
        clips.erase(clips.begin() + clipIndex);
        if (onClipsChanged) onClipsChanged();
    }
}

void Track::moveClip(int clipIndex, double newStartTime)
{
    if (clipIndex >= 0 && clipIndex < static_cast<int>(clips.size()))
    {
        clips[clipIndex]->setTimelineStart(juce::jmax(0.0, newStartTime));
        if (onClipsChanged) onClipsChanged();
    }
}

void Track::renderClipBlock(AudioClip& clip, juce::AudioBuffer<float>& destBuffer,
                            int destStartSample, int numSamples, double positionInSeconds)
{
    SampleLoader::SampleInfo* sampleInfo = nullptr;

    if (sampleLoader != nullptr)
    {
        sampleInfo = sampleLoader->find(clip.getSourceFile());
    }

    if (sampleInfo != nullptr)
    {
        double clipStart = clip.getTimelineStart();
        double sourceOffset = clip.getSourceOffset();

        double startInClip = positionInSeconds - clipStart;
        double sourcePosition = sourceOffset + startInClip;

        juce::int64 startSample = static_cast<juce::int64>(sourcePosition * sampleInfo->sampleRate);
        int samplesToRead = numSamples;

        if (startSample < 0)
        {
            samplesToRead += static_cast<int>(startSample);
            startSample = 0;
        }

        if (samplesToRead <= 0 || startSample >= sampleInfo->lengthInSamples)
            return;

        samplesToRead = juce::jmin(samplesToRead,
                                   static_cast<int>(sampleInfo->lengthInSamples - startSample));

        double clipGain = clip.getGain();
        const int destChannels = destBuffer.getNumChannels();
        const int srcChannels = sampleInfo->buffer.getNumChannels();

        for (int ch = 0; ch < destChannels && ch < srcChannels; ++ch)
        {
            const float* src = sampleInfo->buffer.getReadPointer(ch) + startSample;
            destBuffer.addFrom(ch, destStartSample, src, samplesToRead,
                               static_cast<float>(clipGain));
        }

        return;
    }

    auto* reader = formatManager.createReaderFor(clip.getSourceFile());
    if (reader == nullptr)
        return;

    double clipStart = clip.getTimelineStart();
    double sourceOffset = clip.getSourceOffset();

    double startInClip = positionInSeconds - clipStart;
    double sourcePosition = sourceOffset + startInClip;

    juce::int64 startSample = static_cast<juce::int64>(sourcePosition * reader->sampleRate);
    int samplesToRead = numSamples;

    if (startSample < 0)
    {
        samplesToRead += static_cast<int>(startSample);
        startSample = 0;
    }

    if (samplesToRead <= 0 || startSample >= reader->lengthInSamples)
    {
        delete reader;
        return;
    }

    samplesToRead = juce::jmin(samplesToRead, static_cast<int>(reader->lengthInSamples - startSample));

    juce::AudioBuffer<float> readBuffer(reader->numChannels, samplesToRead);
    reader->read(&readBuffer, 0, samplesToRead, startSample, true, true);

    double clipGain = clip.getGain();
    const int destChannels = destBuffer.getNumChannels();
    const int readChannels = readBuffer.getNumChannels();

    for (int ch = 0; ch < destChannels && ch < readChannels; ++ch)
    {
        destBuffer.addFrom(ch, destStartSample, readBuffer, ch, 0, samplesToRead, static_cast<float>(clipGain));
    }

    delete reader;
}

juce::Colour Track::getTrackColourForNumber(int number)
{
    const juce::Colour colours[] = {
        juce::Colour(0xff4a90d9),
        juce::Colour(0xffe74c3c),
        juce::Colour(0xff2ecc71),
        juce::Colour(0xfff39c12),
        juce::Colour(0xff9b59b6),
        juce::Colour(0xff1abc9c),
        juce::Colour(0xffe67e22),
        juce::Colour(0xff3498db),
    };

    return colours[number % 8];
}

} // namespace harmonic_engine
