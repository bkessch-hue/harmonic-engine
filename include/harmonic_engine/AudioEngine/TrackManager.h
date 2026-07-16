#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <memory>
#include "harmonic_engine/AudioEngine/Track.h"
#include "harmonic_engine/AudioEngine/SampleLoader.h"

namespace harmonic_engine
{

class TrackManager
{
public:
    TrackManager();
    ~TrackManager();

    Track* addTrack();
    Track* addTrack(const juce::String& name);
    void removeTrack(int trackNumber);
    void clearAllTracks();

    int getNumTracks() const;
    Track* getTrack(int index) const;
    Track* getTrackByNumber(int trackNumber) const;

    int getSelectedTrackIndex() const;
    void setSelectedTrackIndex(int index);
    Track* getSelectedTrack() const;

    void setSampleLoader(SampleLoader* loader);
    SampleLoader* getSampleLoader() const;

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void releaseResources();

    std::function<void()> onTracksChanged;
    std::function<void(int)> onTrackSelectionChanged;

private:
    std::vector<std::unique_ptr<Track>> tracks;
    int selectedTrackIndex = 0;
    int nextTrackNumber = 1;

    SampleLoader* sampleLoader = nullptr;

    double currentSampleRate = 44100.0;
    int currentSamplesPerBlock = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackManager)
};

} // namespace harmonic_engine
