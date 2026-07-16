#include "harmonic_engine/State/ProjectState.h"

namespace harmonic_engine
{

const juce::Identifier ProjectState::projectId("HarmonicEngineProject");
const juce::Identifier ProjectState::trackListId("TrackList");
const juce::Identifier ProjectState::trackId("Track");
const juce::Identifier ProjectState::transportId("Transport");

ProjectState::ProjectState(TrackManager& tm, Transport& t)
    : trackManager(tm),
      transport(t),
      state(projectId)
{
    state.setProperty("projectName", projectName, nullptr);
    state.addChild(juce::ValueTree(trackListId), -1, nullptr);
    state.addChild(juce::ValueTree(transportId), -1, nullptr);
}

ProjectState::~ProjectState()
{
}

juce::ValueTree ProjectState::getState() const
{
    return state;
}

void ProjectState::setState(const juce::ValueTree& newState)
{
    if (newState.isValid() && newState.hasType(projectId))
    {
        state = newState;
        projectName = state.getProperty("projectName", "Untitled");

        trackManager.clearAllTracks();

        auto trackListNode = state.getChildWithName(trackListId);
        for (int i = 0; i < trackListNode.getNumChildren(); ++i)
        {
            auto trackState = trackListNode.getChild(i);
            trackFromTrackState(trackState);
        }

        unsavedChanges = false;

        if (onStateChanged)
            onStateChanged();
    }
}

bool ProjectState::saveToFile(const juce::File& file)
{
    juce::XmlElement xml("HarmonicEngineProject");
    xml.setAttribute("projectName", projectName);

    auto trackListNode = xml.createNewChildElement("TrackList");
    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track != nullptr)
        {
            auto trackElement = trackListNode->createNewChildElement("Track");
            trackElement->setAttribute("number", track->getTrackNumber());
            trackElement->setAttribute("name", track->getName());
            trackElement->setAttribute("gain", static_cast<double>(track->getGain()));
            trackElement->setAttribute("pan", static_cast<double>(track->getPan()));
            trackElement->setAttribute("muted", track->isMuted());
            trackElement->setAttribute("soloed", track->isSoloed());
        }
    }

    auto transportElement = xml.createNewChildElement("Transport");
    transportElement->setAttribute("tempo", transport.getTempo());

    auto text = xml.toString();
    bool success = file.replaceWithText(text);

    if (success)
        unsavedChanges = false;

    return success;
}

bool ProjectState::loadFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
        return false;

    if (xml->getTagName() != "HarmonicEngineProject")
        return false;

    projectName = xml->getStringAttribute("projectName", "Untitled");

    trackManager.clearAllTracks();

    auto* trackList = xml->getChildByName("TrackList");
    if (trackList != nullptr)
    {
        for (auto* trackElement : trackList->getChildIterator())
        {
            auto* track = trackManager.addTrack(trackElement->getStringAttribute("name", "Track"));
            track->setGain(static_cast<float>(trackElement->getDoubleAttribute("gain", 1.0)));
            track->setPan(static_cast<float>(trackElement->getDoubleAttribute("pan", 0.0)));
            track->setMuted(trackElement->getBoolAttribute("muted", false));
            track->setSoloed(trackElement->getBoolAttribute("soloed", false));
        }
    }

    auto* transportElement = xml->getChildByName("Transport");
    if (transportElement != nullptr)
    {
        transport.setTempo(transportElement->getDoubleAttribute("tempo", 120.0));
    }

    unsavedChanges = false;

    if (onStateChanged)
        onStateChanged();

    return true;
}

juce::String ProjectState::getProjectName() const
{
    return projectName;
}

void ProjectState::setProjectName(const juce::String& name)
{
    projectName = name;
    state.setProperty("projectName", projectName, nullptr);
    unsavedChanges = true;
}

bool ProjectState::hasUnsavedChanges() const
{
    return unsavedChanges;
}

void ProjectState::markAsSaved()
{
    unsavedChanges = false;
}

void ProjectState::trackStateFromTrack(juce::ValueTree& trackState, Track* track)
{
    trackState.setProperty("number", track->getTrackNumber(), nullptr);
    trackState.setProperty("name", track->getName(), nullptr);
    trackState.setProperty("gain", track->getGain(), nullptr);
    trackState.setProperty("pan", track->getPan(), nullptr);
    trackState.setProperty("muted", track->isMuted(), nullptr);
    trackState.setProperty("soloed", track->isSoloed(), nullptr);
}

void ProjectState::trackFromTrackState(const juce::ValueTree& trackState)
{
    auto* track = trackManager.addTrack(trackState.getProperty("name", "Track").toString());
    track->setGain(static_cast<float>(trackState.getProperty("gain", 1.0)));
    track->setPan(static_cast<float>(trackState.getProperty("pan", 0.0)));
    track->setMuted(trackState.getProperty("muted", false));
    track->setSoloed(trackState.getProperty("soloed", false));
}

} // namespace harmonic_engine
