#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/Transport.h"
#include "harmonic_engine/AudioEngine/TrackManager.h"
#include "harmonic_engine/AudioEngine/AudioFileLoader.h"
#include "harmonic_engine/GUI/AudioRegion.h"

namespace harmonic_engine
{

class TrackLaneComponent;

class TimelineView : public juce::Component,
                     public juce::FileDragAndDropTarget
{
public:
    TimelineView(Transport& transport, TrackManager& trackManager,
                 AudioFileLoader& fileLoader);
    ~TimelineView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPixelsPerSecond(double pps);
    double getPixelsPerSecond() const;

    void setTrackHeight(double height);
    double getTrackHeight() const;

    void updatePlayhead();
    void rebuildClipComponents();

    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override;

    // juce::FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    std::function<void(double newPixelsPerSecond)> onZoomChanged;

private:
    Transport& transport;
    TrackManager& trackManager;
    AudioFileLoader& fileLoader;

    juce::Viewport viewport;
    juce::Component trackHeaderComponent;
    juce::Component timelineContent;

    double pixelsPerSecond = 100.0;
    double trackHeight = 80.0;

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{ 10 };

    std::vector<std::unique_ptr<AudioRegion>> clipComponents;
    juce::Component playheadComponent;

    void drawRuler(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawTrackHeaders(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawTrackLanes(juce::Graphics& g, juce::Rectangle<int> bounds);

    int timeToX(double timeSeconds) const;
    double xToTime(int x) const;

    int getTrackIndexAtY(int y) const;
    double getTotalTimelineLength() const;

    void handleZoom(double zoomDelta, int mouseX);
    void centerOnPosition(double timeSeconds);

    int dragOverTrackIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineView)
};

} // namespace harmonic_engine
