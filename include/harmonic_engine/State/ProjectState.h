#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include "harmonic_engine/AudioEngine/TrackManager.h"
#include "harmonic_engine/AudioEngine/Transport.h"

namespace harmonic_engine
{

class ProjectState
{
public:
    ProjectState(TrackManager& trackManager, Transport& transport);
    ~ProjectState();

    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& newState);

    bool saveToFile(const juce::File& file);
    bool loadFromFile(const juce::File& file);

    juce::String getProjectName() const;
    void setProjectName(const juce::String& name);

    bool hasUnsavedChanges() const;
    void markAsSaved();

    std::function<void()> onStateChanged;

private:
    TrackManager& trackManager;
    Transport& transport;

    juce::ValueTree state;
    juce::String projectName = "Untitled";
    bool unsavedChanges = false;

    static const juce::Identifier projectId;
    static const juce::Identifier trackListId;
    static const juce::Identifier trackId;
    static const juce::Identifier transportId;

    void trackStateFromTrack(juce::ValueTree& trackState, Track* track);
    void trackFromTrackState(const juce::ValueTree& trackState);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProjectState)
};

} // namespace harmonic_engine
