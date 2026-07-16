#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "harmonic_engine/AudioEngine/AudioClip.h"

namespace harmonic_engine
{

class AudioRegion : public juce::Component,
                    public juce::ChangeListener
{
public:
    AudioRegion(AudioClip& clip,
                juce::AudioFormatManager& formatManager,
                juce::AudioThumbnailCache& thumbnailCache);
    ~AudioRegion() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void setPixelsPerSecond(double pps);
    void setVisibleRange(juce::Range<double> range);

    AudioClip& getClip();

    std::function<void(double newStartTime)> onClipMoved;
    std::function<void()> onClipSelected;

private:
    AudioClip& clip;
    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnailCache& thumbnailCacheRef;
    juce::AudioThumbnail thumbnail;

    double pixelsPerSecond = 100.0;
    juce::Range<double> visibleRange;

    bool isDragging = false;
    double dragStartTime = 0.0;
    double dragOffset = 0.0;

    void updateThumbnail();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRegion)
};

} // namespace harmonic_engine
