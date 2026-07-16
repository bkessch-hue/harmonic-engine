#include "harmonic_engine/AudioEngine/Engine.h"

namespace harmonic_engine
{

Engine::Engine()
    : recorder(std::make_unique<AudioRecorder>(backgroundThread))
{
    backgroundThread.startThread();
    deviceManager.initialiseWithDefaultDevices(2, 2);
    deviceManager.addAudioCallback(this);

    trackManager.setSampleLoader(&sampleLoader);
}

Engine::~Engine()
{
    deviceManager.removeAudioCallback(this);
    deviceManager.closeAudioDevice();
    recorder->stopRecording();
    backgroundThread.stopThread(2000);
}

Transport& Engine::getTransport()
{
    return transport;
}

TrackManager& Engine::getTrackManager()
{
    return trackManager;
}

MultiTrackMixer& Engine::getMultiTrackMixer()
{
    return multiTrackMixer;
}

MixerGraph& Engine::getMixerGraph()
{
    return mixerGraph;
}

AudioFileLoader& Engine::getFileLoader()
{
    return fileLoader;
}

SampleLoader& Engine::getSampleLoader()
{
    return sampleLoader;
}

DrumSequencer& Engine::getDrumSequencer()
{
    return drumSequencer;
}

juce::AudioDeviceManager& Engine::getDeviceManager()
{
    return deviceManager;
}

void Engine::addMidiInputMessage(const juce::MidiMessage& msg)
{
    const juce::ScopedLock lock(inputMidiLock);
    inputMidiBuffer.addEvent(msg, 0);
}

juce::MidiBuffer& Engine::getInputMidiBuffer()
{
    return inputMidiBuffer;
}

void Engine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;

    transport.prepareToPlay(sampleRate, samplesPerBlock);
    trackManager.prepareToPlay(sampleRate, samplesPerBlock);
    multiTrackMixer.prepareToPlay(sampleRate, samplesPerBlock);
    mixerGraph.prepareToPlay(sampleRate, samplesPerBlock);
    drumSequencer.prepareToPlay(sampleRate, samplesPerBlock);
}

void Engine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    buffer.clear();

    transport.processBlock(buffer, midiMessages);

    std::vector<Track*> activeTracks;
    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track != nullptr)
        {
            track->processBlock(buffer, midiMessages, transport.getPositionInSeconds());
            activeTracks.push_back(track);
        }
    }

    multiTrackMixer.processBlock(buffer, activeTracks, numSamples);

    if (metronomeEnabled || countInActive)
        renderMetronome(buffer, numSamples, numChannels);
}

void Engine::releaseResources()
{
    trackManager.releaseResources();
}

double Engine::getSampleRate() const
{
    return currentSampleRate;
}

int Engine::getSamplesPerBlock() const
{
    return currentSamplesPerBlock;
}

void Engine::setMetronomeEnabled(bool enabled)
{
    metronomeEnabled = enabled;
}

bool Engine::isMetronomeEnabled() const
{
    return metronomeEnabled;
}

void Engine::setCountInEnabled(bool enabled)
{
    countInEnabled = enabled;
}

bool Engine::isCountInEnabled() const
{
    return countInEnabled;
}

void Engine::setInputMonitoringEnabled(bool enabled)
{
    inputMonitoringEnabled = enabled;
}

bool Engine::isInputMonitoringEnabled() const
{
    return inputMonitoringEnabled;
}

void Engine::setLoopRecordEnabled(bool enabled)
{
    loopRecordEnabled = enabled;
    if (!enabled)
    {
        loopRecordPassCount = 0;
        lastLoopPassCount = 0;
    }

    if (enabled && transport.isPlaying())
    {
        transport.setLooping(true);
    }
}

bool Engine::isLoopRecordEnabled() const
{
    return loopRecordEnabled;
}

bool Engine::isLoopRecording() const
{
    return loopRecordEnabled && transport.isRecording() && transport.isLooping();
}

int Engine::getLoopPassRecordings() const
{
    return loopRecordPassCount;
}

void Engine::finaliseLoopPass()
{
    auto* armedTrack = trackManager.getSelectedTrack();
    if (armedTrack == nullptr || !armedTrack->isRecordArmed())
        return;

    recorder->stopRecording();
    auto recordedFile = recorder->getCurrentFile();
    if (recordedFile.existsAsFile())
    {
        auto clip = armedTrack->importAudioFile(recordedFile, transport.getLoopStart());
        if (clip != nullptr)
        {
            clip->setName("Loop " + juce::String(loopRecordPassCount + 1));
            clip->setColour(armedTrack->getTrackColour().withBrightness(
                0.5f + (loopRecordPassCount % 5) * 0.1f));
        }
    }

    AudioFileLoader::ensureRecordingDirectoryExists();
    auto recDir = AudioFileLoader::getRecordingDirectory();
    auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
    auto newFile = recDir.getChildFile("Loop_" + timestamp + ".wav");
    recorder->startRecording(newFile, currentSampleRate, 2);

    loopRecordPassCount++;
}

void Engine::startRecording()
{
    if (countInEnabled && transport.isPlaying())
    {
        countInActive = true;
        int beatsPerBar = 4;
        double tempo = transport.getTempo();
        double secondsPerBeat = 60.0 / tempo;
        double countInSeconds = beatsPerBar * secondsPerBeat;
        countInSamplesRemaining = static_cast<int>(countInSeconds * currentSampleRate);
        metronomeSamplePosition = 0;
        return;
    }

    loopRecordPassCount = 0;
    lastLoopPassCount = 0;

    AudioFileLoader::ensureRecordingDirectoryExists();
    auto recDir = AudioFileLoader::getRecordingDirectory();
    auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");

    if (loopRecordEnabled && transport.isLooping())
    {
        transport.setPositionInSeconds(transport.getLoopStart());
        transport.resetLoopPassCount();

        transport.setLoopPassCallback([this](int pass)
        {
            if (isLoopRecording() && pass > lastLoopPassCount)
            {
                lastLoopPassCount = pass;
                finaliseLoopPass();
            }
        });

        auto file = recDir.getChildFile("Loop_" + timestamp + ".wav");
        recorder->startRecording(file, currentSampleRate, 2);
        transport.record();
    }
    else
    {
        transport.setLoopPassCallback(nullptr);
        auto file = recDir.getChildFile("Recording_" + timestamp + ".wav");
        recorder->startRecording(file, currentSampleRate, 2);
        transport.record();
    }
}

void Engine::stopRecording()
{
    recorder->stopRecording();
    transport.stop();
    transport.setLoopPassCallback(nullptr);
    countInActive = false;
    countInSamplesRemaining = 0;
    loopRecordPassCount = 0;
    lastLoopPassCount = 0;
}

bool Engine::isRecording() const
{
    return recorder->isRecording();
}

int Engine::getBeatLength() const
{
    double tempo = transport.getTempo();
    if (tempo <= 0.0) return static_cast<int>(currentSampleRate);
    double secondsPerBeat = 60.0 / tempo;
    return static_cast<int>(secondsPerBeat * currentSampleRate);
}

void Engine::renderClick(juce::AudioBuffer<float>& buffer, int startSample,
                         int numSamples, int numChannels, bool isAccent)
{
    const float frequency = isAccent ? 1000.0f : 800.0f;
    const float amplitude = isAccent ? 0.5f : 0.3f;
    const float duration = 0.01f;
    const int clickLength = juce::jmin(static_cast<int>(duration * currentSampleRate), numSamples);
    const int endSample = juce::jmin(startSample + clickLength, numSamples);

    for (int i = startSample; i < endSample; ++i)
    {
        float t = static_cast<float>(i - startSample) / static_cast<float>(currentSampleRate);
        float envelope = 1.0f - (t / duration);
        envelope = juce::jmax(0.0f, envelope);
        float sample = amplitude * envelope * std::sin(2.0f * 3.14159265f * frequency * t);

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addSample(ch, i, sample);
    }
}

void Engine::renderMetronome(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels)
{
    int beatLength = getBeatLength();
    if (beatLength <= 0) return;

    int samplesToProcess = countInActive ? countInSamplesRemaining : numSamples;
    samplesToProcess = juce::jmin(samplesToProcess, numSamples);

    for (int i = 0; i < samplesToProcess; ++i)
    {
        int posInBeat = metronomeSamplePosition % beatLength;
        if (posInBeat == 0)
        {
            int beatNumber = metronomeSamplePosition / beatLength;
            bool isAccent = (beatNumber % 4 == 0);
            renderClick(buffer, i, samplesToProcess - i, numChannels, isAccent);
        }
        ++metronomeSamplePosition;
    }

    if (countInActive)
    {
        countInSamplesRemaining -= samplesToProcess;
        if (countInSamplesRemaining <= 0)
        {
            countInActive = false;
            countInSamplesRemaining = 0;

            AudioFileLoader::ensureRecordingDirectoryExists();
            auto recDir = AudioFileLoader::getRecordingDirectory();
            auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
            auto file = recDir.getChildFile("Recording_" + timestamp + ".wav");

            recorder->startRecording(file, currentSampleRate, 2);
            transport.record();
        }
    }
}

double Engine::getProjectLengthInSeconds() const
{
    double maxEnd = 0.0;

    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track == nullptr) continue;

        for (int c = 0; c < track->getNumClips(); ++c)
        {
            auto clip = track->getClip(c);
            if (clip != nullptr)
            {
                double clipEnd = clip->getTimelineEnd();
                if (clipEnd > maxEnd)
                    maxEnd = clipEnd;
            }
        }
    }

    return juce::jmax(maxEnd, 10.0);
}

void Engine::exportAudio(const juce::File& outputFile, double sampleRate, int bitDepth)
{
    transport.stop();
    transport.setPositionInSeconds(0.0);

    const int exportChannels = 2;
    const int samplesPerBlock = 1024;
    double lengthSeconds = getProjectLengthInSeconds();
    juce::int64 totalSamples = static_cast<juce::int64>(lengthSeconds * sampleRate);

    juce::WavAudioFormat wavFormat;
    auto stream = outputFile.createOutputStream();
    if (stream == nullptr) return;

    auto options = juce::AudioFormatWriterOptions{}
        .withSampleRate(sampleRate)
        .withNumChannels(exportChannels)
        .withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);

    auto* writer = wavFormat.createWriterFor(stream.release(), sampleRate,
                                            exportChannels, bitDepth,
                                            {}, 0);
    if (writer == nullptr) return;

    juce::AudioBuffer<float> mixBuffer(exportChannels, samplesPerBlock);
    juce::MidiBuffer midiBuffer;

    TrackManager exportTrackManager;
    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* src = trackManager.getTrack(i);
        if (src == nullptr) continue;

        auto* dst = exportTrackManager.addTrack(src->getName());
        dst->setGain(src->getGain());
        dst->setPan(src->getPan());

        for (int c = 0; c < src->getNumClips(); ++c)
            dst->addClip(src->getClip(c));
    }

    exportTrackManager.prepareToPlay(sampleRate, samplesPerBlock);
    MixerGraph exportMixer;
    exportMixer.prepareToPlay(sampleRate, samplesPerBlock);

    juce::int64 currentSample = 0;

    while (currentSample < totalSamples)
    {
        int blockToRender = static_cast<int>(juce::jmin(
            static_cast<juce::int64>(samplesPerBlock), totalSamples - currentSample));

        mixBuffer.clear(0, blockToRender);
        double posSeconds = static_cast<double>(currentSample) / sampleRate;

        std::vector<Track*> activeTracks;
        for (int i = 0; i < exportTrackManager.getNumTracks(); ++i)
        {
            auto* track = exportTrackManager.getTrack(i);
            if (track != nullptr)
            {
                track->processBlock(mixBuffer, midiBuffer, posSeconds);
                activeTracks.push_back(track);
            }
        }

        exportMixer.processBlock(mixBuffer, midiBuffer, activeTracks, blockToRender);
        writer->writeFromAudioSampleBuffer(mixBuffer, 0, blockToRender);
        currentSample += blockToRender;
    }

    delete writer;
}

void Engine::freezeTrack(int trackIndex, const juce::File& outputFile)
{
    auto* track = trackManager.getTrack(trackIndex);
    if (track == nullptr || track->getNumClips() == 0) return;

    transport.stop();
    transport.setPositionInSeconds(0.0);

    const int freezeChannels = 2;
    const int freezeRate = 44100;
    const int freezeBlock = 1024;

    double lengthSeconds = getProjectLengthInSeconds();
    juce::int64 totalSamples = static_cast<juce::int64>(lengthSeconds * freezeRate);

    juce::WavAudioFormat wavFormat;
    auto stream = outputFile.createOutputStream();
    if (stream == nullptr) return;

    auto* writer = wavFormat.createWriterFor(stream.release(), freezeRate,
                                            freezeChannels, 24,
                                            {}, 0);
    if (writer == nullptr) return;

    juce::AudioBuffer<float> trackBuffer(freezeChannels, freezeBlock);
    juce::MidiBuffer midiBuffer;

    TrackManager singleTrackManager;
    auto* frozen = singleTrackManager.addTrack("Frozen");
    frozen->setGain(1.0f);
    for (int c = 0; c < track->getNumClips(); ++c)
        frozen->addClip(track->getClip(c));
    singleTrackManager.prepareToPlay(freezeRate, freezeBlock);

    juce::int64 currentSample = 0;

    while (currentSample < totalSamples)
    {
        int blockToRender = static_cast<int>(juce::jmin(
            static_cast<juce::int64>(freezeBlock), totalSamples - currentSample));

        trackBuffer.clear(0, blockToRender);
        double posSeconds = static_cast<double>(currentSample) / freezeRate;

        auto* renderTrack = singleTrackManager.getTrack(0);
        if (renderTrack != nullptr)
            renderTrack->processBlock(trackBuffer, midiBuffer, posSeconds);

        writer->writeFromAudioSampleBuffer(trackBuffer, 0, blockToRender);
        currentSample += blockToRender;
    }

    delete writer;

    track->clearClips();
    auto frozenClip = std::make_shared<AudioClip>(
        track->getName() + " (Frozen)",
        outputFile,
        0.0, 0.0, lengthSeconds, freezeRate);
    track->addClip(frozenClip);
}

void Engine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    const double sampleRate = device->getCurrentSampleRate();
    const int bufferSize = device->getCurrentBufferSizeSamples();

    prepareToPlay(sampleRate, bufferSize);
}

void Engine::audioDeviceStopped()
{
    releaseResources();
}

void Engine::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                             int numInputChannels,
                                             float* const* outputChannelData,
                                             int numOutputChannels,
                                             int numSamples,
                                             const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(context);

    juce::AudioBuffer<float> outputBuffer(outputChannelData, numOutputChannels, numSamples);
    juce::MidiBuffer mergedMidi;

    {
        const juce::ScopedLock lock(inputMidiLock);
        mergedMidi = inputMidiBuffer;
        inputMidiBuffer.clear();
    }

    {
        juce::MidiBuffer drumMidi;
        drumSequencer.processBlock(drumMidi, transport.getPositionInSeconds(),
                                    transport.getTempo(), currentSampleRate,
                                    transport.isPlaying());
        for (const auto& msg : drumMidi)
            mergedMidi.addEvent(msg.getMessage(), msg.samplePosition);
    }

    if (transport.isRecording())
        recorder->writeBlock(inputChannelData, numInputChannels, numSamples);

    outputBuffer.clear();
    processBlock(outputBuffer, mergedMidi);

    if (inputMonitoringEnabled && numInputChannels > 0)
    {
        int chsToMix = juce::jmin(numInputChannels, numOutputChannels);
        for (int ch = 0; ch < chsToMix; ++ch)
        {
            for (int s = 0; s < numSamples; ++s)
                outputBuffer.addSample(ch, s, inputChannelData[ch][s] * 0.5f);
        }
    }
}

} // namespace harmonic_engine
