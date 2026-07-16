#include "harmonic_engine/GUI/TimelineView.h"
#include "harmonic_engine/GUI/LookAndFeel.h"
#include "harmonic_engine/GUI/DesignTokens.h"

namespace harmonic_engine
{

TimelineView::TimelineView(Transport& t, TrackManager& tm, AudioFileLoader& fl)
    : transport(t),
      trackManager(tm),
      fileLoader(fl)
{
    formatManager.registerBasicFormats();

    addAndMakeVisible(viewport);

    viewport.setViewedComponent(&timelineContent, false);
    timelineContent.setOpaque(false);

    viewport.setScrollBarThickness(10);
    viewport.setColour(juce::ScrollBar::thumbColourId, Tokens::Colours::bgOverlay());
    viewport.setColour(juce::ScrollBar::trackColourId, Tokens::Colours::bgBase());

    trackManager.onTracksChanged = [this]() { rebuildClipComponents(); };
}

TimelineView::~TimelineView()
{
}

void TimelineView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.setColour(Tokens::Colours::bgDarkest());
    g.fillRect(bounds);

    const int rulerHeight = 30;
    auto rulerBounds = bounds.removeFromTop(rulerHeight);
    drawRuler(g, rulerBounds);

    const int headerWidth = 120;
    auto headerBounds = bounds.removeFromLeft(headerWidth);
    drawTrackHeaders(g, headerBounds);

    drawTrackLanes(g, bounds);
}

void TimelineView::resized()
{
    auto bounds = getLocalBounds();

    const int rulerHeight = 30;
    bounds.removeFromTop(rulerHeight);

    const int headerWidth = 120;
    bounds.removeFromLeft(headerWidth);

    viewport.setBounds(bounds);

    int contentWidth = static_cast<int>(getTotalTimelineLength() * pixelsPerSecond) + 200;
    int contentHeight = trackManager.getNumTracks() * static_cast<int>(trackHeight);
    contentWidth = juce::jmax(contentWidth, bounds.getWidth());
    contentHeight = juce::jmax(contentHeight, bounds.getHeight());

    timelineContent.setSize(contentWidth, contentHeight);
}

void TimelineView::drawRuler(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(bounds);

    if (transport.isLooping())
    {
        int loopStartX = timeToX(transport.getLoopStart()) + 120 - viewport.getViewPositionX();
        int loopEndX = timeToX(transport.getLoopEnd()) + 120 - viewport.getViewPositionX();

        g.setColour(Tokens::Colours::accent().withAlpha(0.12f));
        g.fillRect(loopStartX, bounds.getY(), loopEndX - loopStartX, bounds.getHeight());

        g.setColour(Tokens::Colours::accent().withAlpha(0.6f));
        g.drawVerticalLine(loopStartX, static_cast<float>(bounds.getY()),
                           static_cast<float>(bounds.getBottom()));
        g.drawVerticalLine(loopEndX, static_cast<float>(bounds.getY()),
                           static_cast<float>(bounds.getBottom()));
    }

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(bounds.getBottom() - 1, 0.0f, static_cast<float>(getWidth()));

    g.setColour(Tokens::Colours::textSecondary());
    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));

    double tempo = transport.getTempo();
    double pixelsPerBeat = pixelsPerSecond * 60.0 / tempo;

    double scrollTime = viewport.getViewPositionX() / pixelsPerSecond;
    int startBeat = static_cast<int>(scrollTime * tempo / 60.0);

    for (int beat = startBeat; beat < startBeat + (getWidth() / pixelsPerBeat) + 2; ++beat)
    {
        int x = static_cast<int>(beat * pixelsPerBeat) - viewport.getViewPositionX();
        bool isBar = (beat % 4 == 0);

        int lineY = isBar ? bounds.getY() : bounds.getBottom() - 8;
        int lineHeight = isBar ? bounds.getHeight() : 8;

        g.drawVerticalLine(x + 120, static_cast<float>(lineY),
                           static_cast<float>(lineY + lineHeight));

        if (isBar)
        {
            int barNumber = (beat / 4) + 1;
            g.drawText(juce::String(barNumber), x + 124, bounds.getY(),
                       40, bounds.getHeight(), juce::Justification::centredLeft);
        }
    }
}

void TimelineView::drawTrackHeaders(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(bounds);

    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track == nullptr) continue;

        int y = i * static_cast<int>(trackHeight);
        auto headerBounds = juce::Rectangle<int>(
            bounds.getX(), y + viewport.getViewPositionY(),
            bounds.getWidth(), static_cast<int>(trackHeight));

        g.setColour(Tokens::Colours::bgDarkest());
        g.fillRect(headerBounds);

        g.setColour(track->getTrackColour().withAlpha(0.3f));
        g.fillRect(headerBounds);

        g.setColour(Tokens::Colours::borderDefault());
        g.drawHorizontalLine(headerBounds.getBottom() - 1, 0.0f,
                             static_cast<float>(bounds.getWidth()));

        g.setColour(Tokens::Colours::textPrimary());
        g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeNormal))));
        g.drawText(track->getName(), headerBounds.reduced(8, 0),
                   juce::Justification::centredLeft);
    }
}

void TimelineView::drawTrackLanes(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (transport.isLooping())
    {
        int loopStartX = timeToX(transport.getLoopStart()) + 120 - viewport.getViewPositionX();
        int loopEndX = timeToX(transport.getLoopEnd()) + 120 - viewport.getViewPositionX();

        g.setColour(Tokens::Colours::accent().withAlpha(0.04f));
        g.fillRect(loopStartX, bounds.getY(), loopEndX - loopStartX, bounds.getHeight());

        g.setColour(Tokens::Colours::accent().withAlpha(0.3f));
        g.drawVerticalLine(loopStartX, static_cast<float>(bounds.getY()),
                           static_cast<float>(bounds.getBottom()));
        g.drawVerticalLine(loopEndX, static_cast<float>(bounds.getY()),
                           static_cast<float>(bounds.getBottom()));
    }

    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track == nullptr) continue;

        int y = i * static_cast<int>(trackHeight) - viewport.getViewPositionY();
        auto laneBounds = juce::Rectangle<int>(
            bounds.getX(), y, bounds.getWidth(), static_cast<int>(trackHeight));

        g.setColour(Tokens::Colours::bgDarkest());
        g.fillRect(laneBounds);

        g.setColour(track->getTrackColour().withAlpha(0.1f));
        g.fillRect(laneBounds);

        g.setColour(Tokens::Colours::borderDefault());
        g.drawHorizontalLine(laneBounds.getBottom() - 1, 0.0f,
                             static_cast<float>(bounds.getWidth()));
    }

    const double playheadTime = transport.getPositionInSeconds();
    const int playheadX = timeToX(playheadTime) + 120 - viewport.getViewPositionX();

    if (playheadX >= bounds.getX() && playheadX <= bounds.getRight())
    {
        g.setColour(Tokens::Colours::playhead());
        g.drawVerticalLine(playheadX, static_cast<float>(bounds.getY()),
                           static_cast<float>(bounds.getBottom()));
    }

    if (dragOverTrackIndex >= 0 && dragOverTrackIndex < trackManager.getNumTracks())
    {
        int dy = dragOverTrackIndex * static_cast<int>(trackHeight) - viewport.getViewPositionY();
        auto dropBounds = juce::Rectangle<int>(
            bounds.getX(), dy, bounds.getWidth(), static_cast<int>(trackHeight));

        g.setColour(Tokens::Colours::accent().withAlpha(0.15f));
        g.fillRect(dropBounds);

        g.setColour(Tokens::Colours::accent().withAlpha(0.8f));
        g.drawRect(dropBounds, 2);
    }
}

void TimelineView::setPixelsPerSecond(double pps)
{
    double oldCenterTime = xToTime(viewport.getViewPositionX() + viewport.getWidth() / 2);

    pixelsPerSecond = juce::jlimit(20.0, 500.0, pps);

    viewport.setViewPosition(
        timeToX(oldCenterTime) - viewport.getWidth() / 2,
        viewport.getViewPositionY());

    rebuildClipComponents();
    resized();
    repaint();
}

double TimelineView::getPixelsPerSecond() const
{
    return pixelsPerSecond;
}

void TimelineView::setTrackHeight(double height)
{
    trackHeight = juce::jlimit(40.0, 200.0, height);
    resized();
    repaint();
}

double TimelineView::getTrackHeight() const
{
    return trackHeight;
}

void TimelineView::updatePlayhead()
{
    repaint();
}

void TimelineView::rebuildClipComponents()
{
    clipComponents.clear();

    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track == nullptr) continue;

        for (int j = 0; j < track->getNumClips(); ++j)
        {
            auto clip = track->getClip(j);
            if (clip == nullptr) continue;

            auto region = std::make_unique<AudioRegion>(*clip, formatManager, thumbnailCache);
            region->setPixelsPerSecond(pixelsPerSecond);

            int x = timeToX(clip->getTimelineStart()) + 120;
            int w = static_cast<int>(clip->getClipDuration() * pixelsPerSecond);
            int y = i * static_cast<int>(trackHeight);
            int h = static_cast<int>(trackHeight);

            region->setBounds(x, y, juce::jmax(w, 20), h);

            region->onClipMoved = [this, i, j](double newTime) {
                trackManager.getTrack(i)->moveClip(j, newTime);
            };

            region->onClipSelected = [this, i]() {
                trackManager.setSelectedTrackIndex(i);
            };

            timelineContent.addAndMakeVisible(region.get());
            clipComponents.push_back(std::move(region));
        }
    }
}

void TimelineView::mouseWheelMove(const juce::MouseEvent& event,
                                   const juce::MouseWheelDetails& wheel)
{
    if (event.mods.isAltDown())
    {
        handleZoom(wheel.deltaY * 10.0, event.getPosition().getX());
    }
    else
    {
        int scrollAmount = wheel.deltaY > 0 ? -30 : 30;
        viewport.setViewPosition(
            viewport.getViewPositionX() + scrollAmount,
            viewport.getViewPositionY());
    }
}

int TimelineView::timeToX(double timeSeconds) const
{
    return static_cast<int>(timeSeconds * pixelsPerSecond);
}

double TimelineView::xToTime(int x) const
{
    return static_cast<double>(x) / pixelsPerSecond;
}

int TimelineView::getTrackIndexAtY(int y) const
{
    const int rulerHeight = 30;
    int contentY = y - rulerHeight;
    if (contentY < 0) return -1;
    int adjustedY = contentY + viewport.getViewPositionY();
    return adjustedY / static_cast<int>(trackHeight);
}

double TimelineView::getTotalTimelineLength() const
{
    double maxEnd = 30.0;

    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track == nullptr) continue;

        for (int j = 0; j < track->getNumClips(); ++j)
        {
            auto clip = track->getClip(j);
            if (clip != nullptr)
            {
                double clipEnd = clip->getTimelineEnd();
                if (clipEnd > maxEnd)
                    maxEnd = clipEnd;
            }
        }
    }

    return maxEnd + 10.0;
}

void TimelineView::handleZoom(double zoomDelta, int mouseX)
{
    double mouseTime = xToTime(mouseX + viewport.getViewPositionX() - 120);

    double newPPS = pixelsPerSecond + zoomDelta;
    newPPS = juce::jlimit(20.0, 500.0, newPPS);

    setPixelsPerSecond(newPPS);

    if (onZoomChanged)
        onZoomChanged(newPPS);
}

void TimelineView::centerOnPosition(double timeSeconds)
{
    int targetX = timeToX(timeSeconds) - viewport.getWidth() / 2;
    viewport.setViewPosition(targetX, viewport.getViewPositionY());
}

bool TimelineView::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        auto file = juce::File(f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" ||
            ext == ".flac" || ext == ".ogg" || ext == ".mp3")
            return true;
    }
    return false;
}

void TimelineView::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(files, x);
    dragOverTrackIndex = getTrackIndexAtY(y);
    repaint();
}

void TimelineView::fileDragExit(const juce::StringArray& files)
{
    juce::ignoreUnused(files);
    dragOverTrackIndex = -1;
    repaint();
}

void TimelineView::filesDropped(const juce::StringArray& files, int x, int y)
{
    int trackIndex = getTrackIndexAtY(y);
    if (trackIndex < 0 || trackIndex >= trackManager.getNumTracks())
    {
        dragOverTrackIndex = -1;
        repaint();
        return;
    }

    dragOverTrackIndex = -1;

    auto* track = trackManager.getTrack(trackIndex);
    if (track == nullptr)
    {
        repaint();
        return;
    }

    int contentX = x - 120 + viewport.getViewPositionX();
    double dropTime = static_cast<double>(contentX) / pixelsPerSecond;
    if (dropTime < 0.0) dropTime = 0.0;

    for (const auto& f : files)
    {
        auto file = juce::File(f);
        if (!file.existsAsFile())
            continue;

        auto clip = track->importAudioFile(file, dropTime);
        if (clip != nullptr)
            dropTime += clip->getClipDuration();
    }

    rebuildClipComponents();
    repaint();
}

} // namespace harmonic_engine
