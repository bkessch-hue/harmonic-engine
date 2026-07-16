#include "harmonic_engine/AudioEngine/AudioClip.h"
#include <juce_audio_formats/juce_audio_formats.h>

namespace harmonic_engine
{

const juce::Identifier AudioClip::clipId("AudioClip");
const juce::Identifier AudioClip::nameProperty("name");
const juce::Identifier AudioClip::sourceFileProperty("sourceFile");
const juce::Identifier AudioClip::timelineStartProperty("timelineStart");
const juce::Identifier AudioClip::sourceOffsetProperty("sourceOffset");
const juce::Identifier AudioClip::clipLengthProperty("clipLength");
const juce::Identifier AudioClip::sourceSampleRateProperty("sourceSampleRate");
const juce::Identifier AudioClip::gainProperty("gain");
const juce::Identifier AudioClip::fadeInProperty("fadeIn");
const juce::Identifier AudioClip::fadeOutProperty("fadeOut");
const juce::Identifier AudioClip::colourProperty("colour");
const juce::Identifier AudioClip::mutedProperty("muted");

AudioClip::AudioClip(const juce::String& clipName,
                     const juce::File& source,
                     double timelineStartSeconds,
                     double sourceOffsetSeconds,
                     double clipLengthSeconds,
                     double sourceSampleRate)
    : state(clipId)
{
    state.setProperty(nameProperty, clipName, nullptr);
    state.setProperty(sourceFileProperty, source.getFullPathName(), nullptr);
    state.setProperty(timelineStartProperty, timelineStartSeconds, nullptr);
    state.setProperty(sourceOffsetProperty, sourceOffsetSeconds, nullptr);
    state.setProperty(clipLengthProperty, clipLengthSeconds, nullptr);
    state.setProperty(sourceSampleRateProperty, sourceSampleRate, nullptr);
    initialiseDefaults();
}

AudioClip::AudioClip(const juce::ValueTree& clipState)
    : state(clipState)
{
}

AudioClip::~AudioClip()
{
}

juce::ValueTree AudioClip::getState() const
{
    return state;
}

juce::String AudioClip::getName() const
{
    return state.getProperty(nameProperty, "Clip").toString();
}

void AudioClip::setName(const juce::String& newName)
{
    state.setProperty(nameProperty, newName, nullptr);
}

juce::File AudioClip::getSourceFile() const
{
    return juce::File(state.getProperty(sourceFileProperty, "").toString());
}

double AudioClip::getTimelineStart() const
{
    return state.getProperty(timelineStartProperty, 0.0);
}

void AudioClip::setTimelineStart(double startSeconds)
{
    state.setProperty(timelineStartProperty, juce::jmax(0.0, startSeconds), nullptr);
}

double AudioClip::getClipDuration() const
{
    return state.getProperty(clipLengthProperty, 0.0);
}

void AudioClip::setClipDuration(double durationSeconds)
{
    state.setProperty(clipLengthProperty, juce::jmax(0.0, durationSeconds), nullptr);
}

double AudioClip::getSourceOffset() const
{
    return state.getProperty(sourceOffsetProperty, 0.0);
}

void AudioClip::setSourceOffset(double offsetSeconds)
{
    state.setProperty(sourceOffsetProperty, juce::jmax(0.0, offsetSeconds), nullptr);
}

double AudioClip::getSourceLength() const
{
    juce::File file = getSourceFile();
    if (file.existsAsFile())
    {
        return static_cast<double>(file.getSize()) / (44100.0 * 2.0 * 2.0);
    }
    return 0.0;
}

double AudioClip::getGain() const
{
    return state.getProperty(gainProperty, 1.0);
}

void AudioClip::setGain(float newGain)
{
    state.setProperty(gainProperty, static_cast<double>(newGain), nullptr);
}

double AudioClip::getFadeIn() const
{
    return state.getProperty(fadeInProperty, 0.0);
}

void AudioClip::setFadeIn(double fadeInSeconds)
{
    state.setProperty(fadeInProperty, juce::jmax(0.0, fadeInSeconds), nullptr);
}

double AudioClip::getFadeOut() const
{
    return state.getProperty(fadeOutProperty, 0.0);
}

void AudioClip::setFadeOut(double fadeOutSeconds)
{
    state.setProperty(fadeOutProperty, juce::jmax(0.0, fadeOutSeconds), nullptr);
}

juce::Colour AudioClip::getColour() const
{
    auto colourVar = state.getProperty(colourProperty, static_cast<int>(0xff4a90d9));
    auto colourInt = static_cast<juce::uint32>(static_cast<int>(colourVar));
    return juce::Colour(colourInt);
}

void AudioClip::setColour(juce::Colour colour)
{
    state.setProperty(colourProperty, static_cast<int>(colour.getARGB()), nullptr);
}

bool AudioClip::isMuted() const
{
    return state.getProperty(mutedProperty, false);
}

void AudioClip::setMuted(bool muted)
{
    state.setProperty(mutedProperty, muted, nullptr);
}

double AudioClip::getTimelineEnd() const
{
    return getTimelineStart() + getClipDuration();
}

double AudioClip::getAvailableSourceLength() const
{
    double sourceOffset = getSourceOffset();
    double available = getSourceLength() - sourceOffset;
    return juce::jmax(0.0, available);
}

bool AudioClip::containsTime(double timeSeconds) const
{
    return timeSeconds >= getTimelineStart() && timeSeconds < getTimelineEnd();
}

double AudioClip::timeToSourcePosition(double timeSeconds) const
{
    return getSourceOffset() + (timeSeconds - getTimelineStart());
}

bool AudioClip::isValid() const
{
    return state.isValid() && state.hasType(clipId);
}

double AudioClip::getSourceSampleRate() const
{
    return static_cast<double>(state.getProperty(sourceSampleRateProperty, 44100.0));
}

void AudioClip::initialiseDefaults()
{
    if (!state.hasProperty(gainProperty))
        state.setProperty(gainProperty, 1.0, nullptr);
    if (!state.hasProperty(fadeInProperty))
        state.setProperty(fadeInProperty, 0.0, nullptr);
    if (!state.hasProperty(fadeOutProperty))
        state.setProperty(fadeOutProperty, 0.0, nullptr);
    if (!state.hasProperty(colourProperty))
        state.setProperty(colourProperty, static_cast<int>(0xff4a90d9), nullptr);
    if (!state.hasProperty(mutedProperty))
        state.setProperty(mutedProperty, false, nullptr);
}

} // namespace harmonic_engine
