#include "harmonic_engine/AudioEngine/MixerGraph.h"
#include <algorithm>

namespace harmonic_engine
{

MixerGraph::MixerGraph()
{
}

MixerGraph::~MixerGraph()
{
}

void MixerGraph::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;

    busABuffer.setSize(2, samplesPerBlock);
    busBBuffer.setSize(2, samplesPerBlock);

    busAChain.prepareToPlay(sampleRate, samplesPerBlock);
    busBChain.prepareToPlay(sampleRate, samplesPerBlock);
}

void MixerGraph::processBlock(juce::AudioBuffer<float>& outputBuffer,
                              juce::MidiBuffer& midiMessages,
                              std::vector<Track*>& tracks,
                              int numSamples)
{
    juce::ignoreUnused(midiMessages);

    const int numChannels = outputBuffer.getNumChannels();
    outputBuffer.clear(0, numSamples);

    busABuffer.clear(0, numSamples);
    busBBuffer.clear(0, numSamples);

    for (auto* track : tracks)
    {
        if (track == nullptr || track->isMuted())
            continue;

        bool anySoloed = false;
        for (auto* otherTrack : tracks)
        {
            if (otherTrack != nullptr && otherTrack->isSoloed())
            {
                anySoloed = true;
                break;
            }
        }

        if (anySoloed && !track->isSoloed())
            continue;

        float trackGain = track->getGain();
        float trackPan = track->getPan();

        float leftGain = trackGain * std::sqrt(0.5f * (1.0f + trackPan));
        float rightGain = trackGain * std::sqrt(0.5f * (1.0f - trackPan));

        const auto& trackBuffer = track->getOutputBuffer();

        if (numChannels > 0)
            outputBuffer.addFrom(0, 0, trackBuffer, 0, 0, numSamples, leftGain);
        if (numChannels > 1 && trackBuffer.getNumChannels() > 1)
            outputBuffer.addFrom(1, 0, trackBuffer, 1, 0, numSamples, rightGain);

        float sendA = track->getBusSendLevel(0);
        float sendB = track->getBusSendLevel(1);
        if (sendA > 0.001f)
        {
            busABuffer.addFrom(0, 0, trackBuffer, 0, 0, numSamples, sendA);
            if (trackBuffer.getNumChannels() > 1)
                busABuffer.addFrom(1, 0, trackBuffer, 1, 0, numSamples, sendA);
        }
        if (sendB > 0.001f)
        {
            busBBuffer.addFrom(0, 0, trackBuffer, 0, 0, numSamples, sendB);
            if (trackBuffer.getNumChannels() > 1)
                busBBuffer.addFrom(1, 0, trackBuffer, 1, 0, numSamples, sendB);
        }
    }

    busAChain.processBlock(busABuffer, numSamples);
    busBChain.processBlock(busBBuffer, numSamples);

    if (numChannels > 0)
        outputBuffer.addFrom(0, 0, busABuffer, 0, 0, numSamples, busAGain);
    if (numChannels > 1)
        outputBuffer.addFrom(1, 0, busABuffer, 1, 0, numSamples, busAGain);
    if (numChannels > 0)
        outputBuffer.addFrom(0, 0, busBBuffer, 0, 0, numSamples, busBGain);
    if (numChannels > 1)
        outputBuffer.addFrom(1, 0, busBBuffer, 1, 0, numSamples, busBGain);

    float currentMasterGain = masterGain;
    outputBuffer.applyGain(0, 0, numSamples, currentMasterGain);
    if (numChannels > 1)
        outputBuffer.applyGain(1, 0, numSamples, currentMasterGain);

    updatePeakLevels(outputBuffer, numSamples);
}

float MixerGraph::getMasterPeakLevel(int channel) const
{
    if (channel == 0) return masterPeakLeft.load();
    if (channel == 1) return masterPeakRight.load();
    return 0.0f;
}

void MixerGraph::setMasterGain(float gain)
{
    masterGain = juce::jlimit(0.0f, 2.0f, gain);
}

float MixerGraph::getMasterGain() const
{
    return masterGain;
}

EffectChain& MixerGraph::getBusAEffectChain() { return busAChain; }
EffectChain& MixerGraph::getBusBEffectChain() { return busBChain; }
void MixerGraph::setBusAGain(float gain) { busAGain = juce::jlimit(0.0f, 2.0f, gain); }
void MixerGraph::setBusBGain(float gain) { busBGain = juce::jlimit(0.0f, 2.0f, gain); }
float MixerGraph::getBusAGain() const { return busAGain; }
float MixerGraph::getBusBGain() const { return busBGain; }

void MixerGraph::updatePeakLevels(const juce::AudioBuffer<float>& buffer, int numSamples)
{
    const int numChannels = buffer.getNumChannels();

    float peakL = 0.0f;
    float peakR = 0.0f;

    if (numChannels > 0)
        peakL = buffer.getMagnitude(0, 0, numSamples);
    if (numChannels > 1)
        peakR = buffer.getMagnitude(1, 0, numSamples);

    masterPeakLeft.store(peakL);
    masterPeakRight.store(peakR);
}

} // namespace harmonic_engine
