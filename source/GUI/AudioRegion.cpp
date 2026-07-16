#include "harmonic_engine/GUI/AudioRegion.h"
#include <juce_audio_formats/juce_audio_formats.h>

namespace harmonic_engine
{

AudioRegion::AudioRegion(AudioClip& clip,
                         juce::AudioFormatManager& fmtManager,
                         juce::AudioThumbnailCache& cache)
    : clip(clip),
      formatManager(fmtManager),
      thumbnailCacheRef(cache),
      thumbnail(256, formatManager, cache)
{
    thumbnail.addChangeListener(this);
    updateThumbnail();
}

AudioRegion::~AudioRegion()
{
    thumbnail.removeChangeListener(this);
}

void AudioRegion::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.setColour(clip.getColour().withAlpha(0.3f));
    g.fillRoundedRectangle(bounds.toFloat(), 3.0f);

    g.setColour(clip.getColour().withAlpha(0.8f));
    g.drawRoundedRectangle(bounds.toFloat(), 3.0f, 1.0f);

    if (thumbnail.getTotalLength() > 0.0)
    {
        double clipStart = clip.getTimelineStart();
        double clipDuration = clip.getClipDuration();

        g.setColour(clip.getColour());

        auto waveBounds = bounds.reduced(2);
        thumbnail.drawChannels(g, waveBounds,
                               clipStart, clipStart + clipDuration, 1.0f);
    }

    g.setColour(juce::Colour(0xffe0e0e0));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText(clip.getName(), bounds.reduced(4),
               juce::Justification::centredLeft, false);
}

void AudioRegion::mouseDown(const juce::MouseEvent& event)
{
    isDragging = true;
    dragStartTime = clip.getTimelineStart();
    dragOffset = event.position.getX() / pixelsPerSecond;

    if (onClipSelected)
        onClipSelected();
}

void AudioRegion::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        double dragDelta = event.position.getX() / pixelsPerSecond - dragOffset;
        double newTime = dragStartTime + dragDelta;

        newTime = juce::jmax(0.0, newTime);
        newTime = std::round(newTime * 100.0) / 100.0;

        clip.setTimelineStart(newTime);

        if (onClipMoved)
            onClipMoved(newTime);

        repaint();
    }
}

void AudioRegion::mouseUp(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    isDragging = false;
}

void AudioRegion::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
        repaint();
}

void AudioRegion::setPixelsPerSecond(double pps)
{
    pixelsPerSecond = pps;
    repaint();
}

void AudioRegion::setVisibleRange(juce::Range<double> range)
{
    visibleRange = range;
}

AudioClip& AudioRegion::getClip()
{
    return clip;
}

void AudioRegion::updateThumbnail()
{
    auto sourceFile = clip.getSourceFile();
    if (sourceFile.existsAsFile())
    {
        thumbnail.setSource(new juce::FileInputSource(sourceFile));
    }
}

} // namespace harmonic_engine
