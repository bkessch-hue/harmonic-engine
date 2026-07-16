#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "harmonic_engine/AudioEngine/Transport.h"
#include "harmonic_engine/AudioEngine/TrackManager.h"
#include "harmonic_engine/AudioEngine/MultiTrackMixer.h"
#include "harmonic_engine/AudioEngine/MixerGraph.h"
#include "harmonic_engine/AudioEngine/AudioRecorder.h"
#include "harmonic_engine/AudioEngine/AudioFileLoader.h"
#include "harmonic_engine/AudioEngine/SampleLoader.h"
#include "harmonic_engine/AudioEngine/DrumSequencer.h"

namespace harmonic_engine
{

class Engine : public juce::AudioIODeviceCallback
{
public:
    Engine();
    ~Engine() override;

    Transport& getTransport();
    TrackManager& getTrackManager();
    MultiTrackMixer& getMultiTrackMixer();
    MixerGraph& getMixerGraph();
    AudioFileLoader& getFileLoader();
    SampleLoader& getSampleLoader();
    juce::AudioDeviceManager& getDeviceManager();

    DrumSequencer& getDrumSequencer();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void releaseResources();

    double getSampleRate() const;
    int getSamplesPerBlock() const;

    void startRecording();
    void stopRecording();
    bool isRecording() const;

    void setMetronomeEnabled(bool enabled);
    bool isMetronomeEnabled() const;

    void setCountInEnabled(bool enabled);
    bool isCountInEnabled() const;

    void setInputMonitoringEnabled(bool enabled);
    bool isInputMonitoringEnabled() const;

    void setLoopRecordEnabled(bool enabled);
    bool isLoopRecordEnabled() const;

    bool isLoopRecording() const;
    int getLoopPassRecordings() const;

    void addMidiInputMessage(const juce::MidiMessage& msg);
    juce::MidiBuffer& getInputMidiBuffer();
    void exportAudio(const juce::File& outputFile, double sampleRate, int bitDepth);
    void freezeTrack(int trackIndex, const juce::File& outputFile);
    double getProjectLengthInSeconds() const;

private:
    juce::AudioDeviceManager deviceManager;
    Transport transport;
    TrackManager trackManager;
    MultiTrackMixer multiTrackMixer;
    MixerGraph mixerGraph;
    AudioFileLoader fileLoader;
    SampleLoader sampleLoader;
    DrumSequencer drumSequencer;

    juce::TimeSliceThread backgroundThread{ "Harmonic Engine Background" };
    std::unique_ptr<AudioRecorder> recorder;

    double currentSampleRate = 44100.0;
    int currentSamplesPerBlock = 512;

    bool metronomeEnabled = true;
    bool countInEnabled = false;
    bool inputMonitoringEnabled = false;
    bool loopRecordEnabled = false;
    int loopRecordPassCount = 0;
    int lastLoopPassCount = 0;

    void finaliseLoopPass();

    int metronomeSamplePosition = 0;
    int countInSamplesRemaining = 0;
    bool countInActive = false;

    juce::MidiBuffer inputMidiBuffer;
    juce::CriticalSection inputMidiLock;

    void renderMetronome(juce::AudioBuffer<float>& buffer, int numSamples, int numChannels);
    void renderClick(juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                     int numChannels, bool isAccent);
    int getBeatLength() const;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                         int numInputChannels,
                                         float* const* outputChannelData,
                                         int numOutputChannels,
                                         int numSamples,
                                         const juce::AudioIODeviceCallbackContext& context) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Engine)
};

} // namespace harmonic_engine
