#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <memory>
#include <functional>
#include "harmonic_engine/AudioEngine/Track.h"
#include "harmonic_engine/AudioEngine/EffectBase.h"

namespace harmonic_engine
{

struct AutomationPoint
{
    double time = 0.0;
    float value = 0.0f;
    int curve = 0; // 0=step, 1=linear, 2=bezier
};

class AutomationLane
{
public:
    juce::String paramPath;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    bool armed = false;
    bool visible = true;

    void addPoint(double time, float value, int curve = 1);
    void removePoint(int index);
    void clear();
    int getNumPoints() const;
    const AutomationPoint& getPoint(int index) const;
    AutomationPoint& getPointRef(int index);
    float getValueAt(double time) const;
    void sort();

    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);

private:
    std::vector<AutomationPoint> points;
};

struct AuxBus
{
    juce::String name;
    float level = 0.0f;
    bool postFader = true;
    EffectChain chain;
    juce::AudioBuffer<float> buffer;
};

class MultiTrackMixer
{
public:
    MultiTrackMixer();
    ~MultiTrackMixer();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& outputBuffer,
                      std::vector<Track*>& tracks,
                      int numSamples);

    // Master
    void setMasterGain(float gain);
    float getMasterGain() const;
    float getMasterPeak(int channel) const;
    EffectChain& getMasterChain();

    // Aux buses
    int addAuxBus(const juce::String& name);
    void removeAuxBus(int index);
    int getNumAuxBuses() const;
    AuxBus& getAuxBus(int index);
    void setTrackAuxSend(int trackIndex, int busIndex, float level);
    float getTrackAuxSend(int trackIndex, int busIndex) const;

    // Track insert effects
    EffectChain& getTrackInsertChain(int trackIndex);
    bool hasTrackInsertChain(int trackIndex) const;

    // Automation
    int addAutomationLane(const juce::String& paramPath,
                          float minValue = 0.0f, float maxValue = 1.0f);
    void removeAutomationLane(int index);
    int getNumAutomationLanes() const;
    AutomationLane* getAutomationLane(int index);
    void applyAutomation(double time);

    // State
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);

    std::function<void(int trackIndex, float level)> onTrackPeakChanged;

private:
    double currentSampleRate = 44100.0;
    int currentSamplesPerBlock = 512;

    float masterGain = 1.0f;
    std::atomic<float> masterPeakLeft{ 0.0f };
    std::atomic<float> masterPeakRight{ 0.0f };
    EffectChain masterChain;

    std::vector<std::unique_ptr<AuxBus>> auxBuses;

    struct TrackInserts
    {
        EffectChain preChain;
        EffectChain postChain;
    };
    std::map<int, std::unique_ptr<TrackInserts>> trackInsertMap;

    std::vector<AutomationLane> automationLanes;

    juce::AudioBuffer<float> masterBuffer;
    juce::AudioBuffer<float> tempBuffer;

    void updatePeakLevels(const juce::AudioBuffer<float>& buffer, int numSamples);
};

} // namespace harmonic_engine
