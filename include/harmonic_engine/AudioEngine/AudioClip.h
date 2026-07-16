#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_graphics/juce_graphics.h>

namespace harmonic_engine
{

class AudioClip
{
public:
    AudioClip(const juce::String& clipName,
              const juce::File& sourceFile,
              double timelineStartSeconds,
              double sourceOffsetSeconds,
              double clipLengthSeconds,
              double sourceSampleRate);

    AudioClip(const juce::ValueTree& clipState);
    ~AudioClip();

    juce::ValueTree getState() const;

    juce::String getName() const;
    void setName(const juce::String& newName);

    juce::File getSourceFile() const;

    double getTimelineStart() const;
    void setTimelineStart(double startSeconds);

    double getClipDuration() const;
    void setClipDuration(double durationSeconds);

    double getSourceOffset() const;
    void setSourceOffset(double offsetSeconds);

    double getSourceLength() const;

    double getGain() const;
    void setGain(float newGain);

    double getFadeIn() const;
    void setFadeIn(double fadeInSeconds);

    double getFadeOut() const;
    void setFadeOut(double fadeOutSeconds);

    juce::Colour getColour() const;
    void setColour(juce::Colour colour);

    bool isMuted() const;
    void setMuted(bool muted);

    double getTimelineEnd() const;
    double getAvailableSourceLength() const;

    bool containsTime(double timeSeconds) const;
    double timeToSourcePosition(double timeSeconds) const;

    bool isValid() const;

    double getSourceSampleRate() const;

    static const juce::Identifier clipId;
    static const juce::Identifier nameProperty;
    static const juce::Identifier sourceFileProperty;
    static const juce::Identifier timelineStartProperty;
    static const juce::Identifier sourceOffsetProperty;
    static const juce::Identifier clipLengthProperty;
    static const juce::Identifier sourceSampleRateProperty;
    static const juce::Identifier gainProperty;
    static const juce::Identifier fadeInProperty;
    static const juce::Identifier fadeOutProperty;
    static const juce::Identifier colourProperty;
    static const juce::Identifier mutedProperty;

private:
    juce::ValueTree state;

    void initialiseDefaults();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioClip)
};

} // namespace harmonic_engine
