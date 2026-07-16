#include "harmonic_engine/AudioEngine/TrackManager.h"
#include "harmonic_engine/AudioEngine/SampleLoader.h"

namespace harmonic_engine
{

TrackManager::TrackManager()
{
    addTrack();
}

TrackManager::~TrackManager()
{
    clearAllTracks();
}

Track* TrackManager::addTrack()
{
    return addTrack("Track " + juce::String(nextTrackNumber));
}

Track* TrackManager::addTrack(const juce::String& name)
{
    auto track = std::make_unique<Track>(nextTrackNumber++);
    track->setName(name);
    track->setSampleLoader(sampleLoader);
    track->prepareToPlay(currentSampleRate, currentSamplesPerBlock);

    Track* trackPtr = track.get();
    tracks.push_back(std::move(track));

    if (onTracksChanged)
        onTracksChanged();

    return trackPtr;
}

void TrackManager::removeTrack(int trackNumber)
{
    auto it = std::find_if(tracks.begin(), tracks.end(),
                           [trackNumber](const std::unique_ptr<Track>& t) {
                               return t->getTrackNumber() == trackNumber;
                           });

    if (it != tracks.end())
    {
        tracks.erase(it);

        if (selectedTrackIndex >= static_cast<int>(tracks.size()))
            selectedTrackIndex = static_cast<int>(tracks.size()) - 1;

        if (onTracksChanged)
            onTracksChanged();
    }
}

void TrackManager::clearAllTracks()
{
    tracks.clear();
    selectedTrackIndex = 0;
    nextTrackNumber = 1;

    if (onTracksChanged)
        onTracksChanged();
}

void TrackManager::setSampleLoader(SampleLoader* loader)
{
    sampleLoader = loader;
    for (auto& track : tracks)
        track->setSampleLoader(loader);
}

SampleLoader* TrackManager::getSampleLoader() const
{
    return sampleLoader;
}

int TrackManager::getNumTracks() const
{
    return static_cast<int>(tracks.size());
}

Track* TrackManager::getTrack(int index) const
{
    if (index >= 0 && index < static_cast<int>(tracks.size()))
        return tracks[index].get();

    return nullptr;
}

Track* TrackManager::getTrackByNumber(int trackNumber) const
{
    for (const auto& track : tracks)
    {
        if (track->getTrackNumber() == trackNumber)
            return track.get();
    }

    return nullptr;
}

int TrackManager::getSelectedTrackIndex() const
{
    return selectedTrackIndex;
}

void TrackManager::setSelectedTrackIndex(int index)
{
    if (index >= 0 && index < static_cast<int>(tracks.size()))
    {
        selectedTrackIndex = index;

        if (onTrackSelectionChanged)
            onTrackSelectionChanged(index);
    }
}

Track* TrackManager::getSelectedTrack() const
{
    return getTrack(selectedTrackIndex);
}

void TrackManager::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;

    for (auto& track : tracks)
        track->prepareToPlay(sampleRate, samplesPerBlock);
}

void TrackManager::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(buffer, midiMessages);
}

void TrackManager::releaseResources()
{
    for (auto& track : tracks)
        track->releaseResources();
}

} // namespace harmonic_engine
