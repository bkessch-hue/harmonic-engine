#include "harmonic_engine/AudioEngine/MultiTrackMixer.h"
#include <algorithm>
#include <cmath>

namespace harmonic_engine
{

// ============================================================================
// AutomationLane
// ============================================================================

void AutomationLane::addPoint(double time, float value, int curveType)
{
    points.push_back({ time, juce::jlimit(minValue, maxValue, value),
                       juce::jlimit(0, 2, curveType) });
    sort();
}

void AutomationLane::removePoint(int index)
{
    if (index >= 0 && index < static_cast<int>(points.size()))
        points.erase(points.begin() + index);
}

void AutomationLane::clear()
{
    points.clear();
}

int AutomationLane::getNumPoints() const
{
    return static_cast<int>(points.size());
}

const AutomationPoint& AutomationLane::getPoint(int index) const
{
    return points[index];
}

AutomationPoint& AutomationLane::getPointRef(int index)
{
    return points[index];
}

void AutomationLane::sort()
{
    std::sort(points.begin(), points.end(),
              [](const AutomationPoint& a, const AutomationPoint& b) {
                  return a.time < b.time;
              });
}

float AutomationLane::getValueAt(double time) const
{
    if (points.empty())
        return 0.0f;

    if (time <= points.front().time)
        return points.front().value;

    if (time >= points.back().time)
        return points.back().value;

    for (int i = 0; i < static_cast<int>(points.size()) - 1; ++i)
    {
        const auto& p0 = points[i];
        const auto& p1 = points[i + 1];

        if (time >= p0.time && time <= p1.time)
        {
            double range = p1.time - p0.time;
            if (range <= 0.0) return p0.value;

            float t = static_cast<float>((time - p0.time) / range);
            t = juce::jlimit(0.0f, 1.0f, t);

            switch (p0.curve)
            {
                case 0: return p0.value;                                              // step
                case 1: return p0.value + (p1.value - p0.value) * t;                 // linear
                case 2: return p0.value + (p1.value - p0.value) * (t * t * (3.0f - 2.0f * t)); // smooth
                default: return p0.value + (p1.value - p0.value) * t;
            }
        }
    }

    return points.back().value;
}

juce::ValueTree AutomationLane::getState() const
{
    juce::ValueTree state("AutomationLane");
    state.setProperty("ParamPath", paramPath, nullptr);
    state.setProperty("MinValue", minValue, nullptr);
    state.setProperty("MaxValue", maxValue, nullptr);
    state.setProperty("Armed", armed, nullptr);
    state.setProperty("Visible", visible, nullptr);

    juce::ValueTree pointsState("Points");
    for (const auto& p : points)
    {
        juce::ValueTree pt("Point");
        pt.setProperty("Time", p.time, nullptr);
        pt.setProperty("Value", p.value, nullptr);
        pt.setProperty("Curve", p.curve, nullptr);
        pointsState.addChild(pt, -1, nullptr);
    }
    state.addChild(pointsState, -1, nullptr);

    return state;
}

void AutomationLane::setState(const juce::ValueTree& state)
{
    if (!state.hasType("AutomationLane")) return;

    paramPath = state.getProperty("ParamPath", paramPath).toString();
    minValue = state.getProperty("MinValue", minValue);
    maxValue = state.getProperty("MaxValue", maxValue);
    armed = state.getProperty("Armed", false);
    visible = state.getProperty("Visible", true);

    points.clear();
    auto ptsState = state.getChildWithName("Points");
    if (ptsState.isValid())
    {
        for (int i = 0; i < ptsState.getNumChildren(); ++i)
        {
            auto pt = ptsState.getChild(i);
            points.push_back({
                pt.getProperty("Time", 0.0),
                pt.getProperty("Value", 0.0f),
                pt.getProperty("Curve", 1)
            });
        }
    }
    sort();
}

// ============================================================================
// MultiTrackMixer
// ============================================================================

MultiTrackMixer::MultiTrackMixer()
{
    addAuxBus("Reverb");
    addAuxBus("Delay");
}

MultiTrackMixer::~MultiTrackMixer() = default;

void MultiTrackMixer::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;

    masterBuffer.setSize(2, samplesPerBlock);
    tempBuffer.setSize(2, samplesPerBlock);

    masterChain.prepareToPlay(sampleRate, samplesPerBlock);

    for (auto& bus : auxBuses)
    {
        if (bus == nullptr) continue;
        bus->buffer.setSize(2, samplesPerBlock);
        bus->chain.prepareToPlay(sampleRate, samplesPerBlock);
    }

    for (auto& [idx, inserts] : trackInsertMap)
    {
        if (inserts == nullptr) continue;
        inserts->preChain.prepareToPlay(sampleRate, samplesPerBlock);
        inserts->postChain.prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void MultiTrackMixer::processBlock(juce::AudioBuffer<float>& outputBuffer,
                                    std::vector<Track*>& tracks,
                                    int numSamples)
{
    const int numChannels = outputBuffer.getNumChannels();
    outputBuffer.clear();

    for (auto& bus : auxBuses)
    {
        if (bus != nullptr)
            bus->buffer.clear(0, numSamples);
    }

    masterBuffer.clear(0, numSamples);
    tempBuffer.clear(0, numSamples);

    bool anySoloed = false;
    for (auto* track : tracks)
    {
        if (track != nullptr && track->isSoloed())
        {
            anySoloed = true;
            break;
        }
    }

    for (auto* track : tracks)
    {
        if (track == nullptr) continue;

        int tn = track->getTrackNumber();
        bool muted = track->isMuted();
        bool soloed = track->isSoloed();

        if (muted) continue;
        if (anySoloed && !soloed) continue;

        const auto& trackBuffer = track->getOutputBuffer();

        // Pre-fader insert effects
        auto it = trackInsertMap.find(tn);
        if (it != trackInsertMap.end() && it->second != nullptr)
        {
            tempBuffer.makeCopyOf(trackBuffer, true);
            it->second->preChain.processBlock(tempBuffer, numSamples);
        }

        const float* trackDataL = trackBuffer.getReadPointer(0);
        const float* trackDataR = trackBuffer.getNumChannels() > 1
                                  ? trackBuffer.getReadPointer(1)
                                  : trackDataL;

        // Aux sends
        for (int b = 0; b < static_cast<int>(auxBuses.size()); ++b)
        {
            auto& bus = auxBuses[b];
            if (bus == nullptr) continue;
            float sendLevel = bus->level;
            if (sendLevel > 0.001f)
            {
                for (int s = 0; s < numSamples; ++s)
                {
                    float sampleL = trackDataL[s] * sendLevel;
                    float sampleR = trackDataR[s] * sendLevel;
                    bus->buffer.addSample(0, s, sampleL);
                    if (bus->buffer.getNumChannels() > 1)
                        bus->buffer.addSample(1, s, sampleR);
                }
            }
        }

        // Sum to master
        float leftGain = track->getGain() * std::sqrt(0.5f * (1.0f + track->getPan()));
        float rightGain = track->getGain() * std::sqrt(0.5f * (1.0f - track->getPan()));

        float peak = 0.0f;
        for (int s = 0; s < numSamples; ++s)
        {
            masterBuffer.addSample(0, s, trackDataL[s] * leftGain);
            if (masterBuffer.getNumChannels() > 1)
                masterBuffer.addSample(1, s, trackDataR[s] * rightGain);
        }

        if (numChannels > 0)
            peak = std::max(peak, trackBuffer.getMagnitude(0, 0, numSamples));
        if (numChannels > 1 && trackBuffer.getNumChannels() > 1)
            peak = std::max(peak, trackBuffer.getMagnitude(1, 0, numSamples));
        track->updatePeakLevel(peak);
        if (onTrackPeakChanged)
            onTrackPeakChanged(tn, peak);

        // Post-fader insert effects
        if (it != trackInsertMap.end() && it->second != nullptr)
        {
            tempBuffer.makeCopyOf(trackBuffer, true);
            it->second->postChain.processBlock(tempBuffer, numSamples);
        }
    }

    // Process aux buses and sum to master
    for (auto& bus : auxBuses)
    {
        if (bus == nullptr) continue;
        if (bus->level > 0.001f)
        {
            bus->chain.processBlock(bus->buffer, numSamples);
            for (int s = 0; s < numSamples; ++s)
            {
                masterBuffer.addSample(0, s, bus->buffer.getSample(0, s) * bus->level);
                if (masterBuffer.getNumChannels() > 1)
                    masterBuffer.addSample(1, s, bus->buffer.getSample(1, s) * bus->level);
            }
        }
    }

    // Master insert effects
    masterChain.processBlock(masterBuffer, numSamples);

    // Master gain
    float currentMasterGain = masterGain;
    for (int s = 0; s < numSamples; ++s)
    {
        if (numChannels > 0)
            outputBuffer.addSample(0, s, masterBuffer.getSample(0, s) * currentMasterGain);
        if (numChannels > 1)
            outputBuffer.addSample(1, s, masterBuffer.getSample(1, s) * currentMasterGain);
    }

    updatePeakLevels(outputBuffer, numSamples);
}

void MultiTrackMixer::setMasterGain(float gain)
{
    masterGain = juce::jlimit(0.0f, 2.0f, gain);
}

float MultiTrackMixer::getMasterGain() const
{
    return masterGain;
}

float MultiTrackMixer::getMasterPeak(int channel) const
{
    if (channel == 0) return masterPeakLeft.load();
    if (channel == 1) return masterPeakRight.load();
    return 0.0f;
}

EffectChain& MultiTrackMixer::getMasterChain()
{
    return masterChain;
}

int MultiTrackMixer::addAuxBus(const juce::String& name)
{
    auto bus = std::make_unique<AuxBus>();
    bus->name = name;
    bus->level = 0.5f;
    bus->postFader = true;
    if (currentSampleRate > 0)
    {
        bus->buffer.setSize(2, currentSamplesPerBlock);
        bus->chain.prepareToPlay(currentSampleRate, currentSamplesPerBlock);
    }
    auxBuses.push_back(std::move(bus));
    return static_cast<int>(auxBuses.size()) - 1;
}

void MultiTrackMixer::removeAuxBus(int index)
{
    if (index >= 0 && index < static_cast<int>(auxBuses.size()))
        auxBuses.erase(auxBuses.begin() + index);
}

int MultiTrackMixer::getNumAuxBuses() const
{
    return static_cast<int>(auxBuses.size());
}

AuxBus& MultiTrackMixer::getAuxBus(int index)
{
    return *auxBuses[index];
}

void MultiTrackMixer::setTrackAuxSend(int trackIndex, int busIndex, float level)
{
    juce::ignoreUnused(trackIndex, busIndex, level);
}

float MultiTrackMixer::getTrackAuxSend(int trackIndex, int busIndex) const
{
    juce::ignoreUnused(trackIndex);
    if (busIndex >= 0 && busIndex < static_cast<int>(auxBuses.size()))
        return (busIndex == 0 || busIndex == 1) ? 1.0f : 0.0f;
    return 0.0f;
}

EffectChain& MultiTrackMixer::getTrackInsertChain(int trackIndex)
{
    auto it = trackInsertMap.find(trackIndex);
    if (it == trackInsertMap.end() || it->second == nullptr)
    {
        auto inserts = std::make_unique<TrackInserts>();
        if (currentSampleRate > 0)
        {
            inserts->preChain.prepareToPlay(currentSampleRate, currentSamplesPerBlock);
            inserts->postChain.prepareToPlay(currentSampleRate, currentSamplesPerBlock);
        }
        it = trackInsertMap.emplace(trackIndex, std::move(inserts)).first;
    }
    return it->second->preChain;
}

bool MultiTrackMixer::hasTrackInsertChain(int trackIndex) const
{
    return trackInsertMap.find(trackIndex) != trackInsertMap.end();
}

int MultiTrackMixer::addAutomationLane(const juce::String& paramPath,
                                        float minValue, float maxValue)
{
    AutomationLane lane;
    lane.paramPath = paramPath;
    lane.minValue = minValue;
    lane.maxValue = maxValue;
    automationLanes.push_back(std::move(lane));
    return static_cast<int>(automationLanes.size()) - 1;
}

void MultiTrackMixer::removeAutomationLane(int index)
{
    if (index >= 0 && index < static_cast<int>(automationLanes.size()))
        automationLanes.erase(automationLanes.begin() + index);
}

int MultiTrackMixer::getNumAutomationLanes() const
{
    return static_cast<int>(automationLanes.size());
}

AutomationLane* MultiTrackMixer::getAutomationLane(int index)
{
    if (index >= 0 && index < static_cast<int>(automationLanes.size()))
        return &automationLanes[index];
    return nullptr;
}

void MultiTrackMixer::applyAutomation(double time)
{
    for (const auto& lane : automationLanes)
    {
        float value = lane.getValueAt(time);
        auto path = lane.paramPath;

        auto parts = juce::StringArray::fromTokens(path, "/", "");
        if (parts.size() < 2) continue;

        if (parts[0] == "master")
        {
            if (parts[1] == "gain")
                setMasterGain(value);
        }
        else if (parts[0] == "track" && parts.size() >= 3)
        {
            int trackIdx = parts[1].getIntValue();
            if (parts[2] == "gain")
            {
                juce::ignoreUnused(trackIdx, value);
            }
            else if (parts[2] == "pan")
            {
                juce::ignoreUnused(trackIdx, value);
            }
        }
    }
}

juce::ValueTree MultiTrackMixer::getState() const
{
    juce::ValueTree state("MultiTrackMixer");
    state.setProperty("MasterGain", masterGain, nullptr);

    juce::ValueTree auxState("AuxBuses");
    for (int i = 0; i < static_cast<int>(auxBuses.size()); ++i)
    {
        if (auxBuses[i] == nullptr) continue;
        juce::ValueTree busState("AuxBus");
        busState.setProperty("Index", i, nullptr);
        busState.setProperty("Name", auxBuses[i]->name, nullptr);
        busState.setProperty("Level", auxBuses[i]->level, nullptr);
        busState.setProperty("PostFader", auxBuses[i]->postFader, nullptr);
        busState.addChild(auxBuses[i]->chain.getState(), -1, nullptr);
        auxState.addChild(busState, -1, nullptr);
    }
    state.addChild(auxState, -1, nullptr);

    juce::ValueTree autoState("AutomationLanes");
    for (const auto& lane : automationLanes)
        autoState.addChild(lane.getState(), -1, nullptr);
    state.addChild(autoState, -1, nullptr);

    juce::ValueTree insertsState("TrackInserts");
    for (const auto& [idx, inserts] : trackInsertMap)
    {
        if (inserts == nullptr) continue;
        juce::ValueTree trackState("Track");
        trackState.setProperty("Index", idx, nullptr);
        trackState.addChild(inserts->preChain.getState(), -1, nullptr);
        insertsState.addChild(trackState, -1, nullptr);
    }
    state.addChild(insertsState, -1, nullptr);

    state.addChild(masterChain.getState(), -1, nullptr);

    return state;
}

void MultiTrackMixer::setState(const juce::ValueTree& state)
{
    if (!state.hasType("MultiTrackMixer")) return;

    masterGain = state.getProperty("MasterGain", masterGain);

    auto auxState = state.getChildWithName("AuxBuses");
    if (auxState.isValid())
    {
        auxBuses.clear();
        for (int i = 0; i < auxState.getNumChildren(); ++i)
        {
            auto busState = auxState.getChild(i);
            auto bus = std::make_unique<AuxBus>();
            bus->name = busState.getProperty("Name", "Bus " + juce::String(i + 1)).toString();
            bus->level = busState.getProperty("Level", 0.5f);
            bus->postFader = busState.getProperty("PostFader", true);
            auto chainState = busState.getChildWithName("EffectChain");
            if (chainState.isValid())
                bus->chain.setState(chainState);
            auxBuses.push_back(std::move(bus));
        }
    }

    auto autoState = state.getChildWithName("AutomationLanes");
    if (autoState.isValid())
    {
        automationLanes.clear();
        for (int i = 0; i < autoState.getNumChildren(); ++i)
        {
            AutomationLane lane;
            lane.setState(autoState.getChild(i));
            automationLanes.push_back(std::move(lane));
        }
    }

    auto masterChainState = state.getChildWithName("EffectChain");
    if (masterChainState.isValid())
        masterChain.setState(masterChainState);
}

void MultiTrackMixer::updatePeakLevels(const juce::AudioBuffer<float>& buffer, int numSamples)
{
    float peakL = (buffer.getNumChannels() > 0) ? buffer.getMagnitude(0, 0, numSamples) : 0.0f;
    float peakR = (buffer.getNumChannels() > 1) ? buffer.getMagnitude(1, 0, numSamples) : 0.0f;
    masterPeakLeft.store(peakL);
    masterPeakRight.store(peakR);
}

} // namespace harmonic_engine
