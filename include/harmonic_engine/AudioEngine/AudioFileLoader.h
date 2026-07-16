#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>

namespace harmonic_engine
{

class AudioFileLoader
{
public:
    AudioFileLoader();
    ~AudioFileLoader();

    juce::AudioFormatManager& getFormatManager();

    std::unique_ptr<juce::AudioFormatReader> createReaderFor(const juce::File& file);
    bool canUnderstandFile(const juce::File& file) const;

    juce::StringArray getSupportedFileExtensions() const;

    static juce::File getRecordingDirectory();
    static void ensureRecordingDirectoryExists();

private:
    juce::AudioFormatManager formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFileLoader)
};

} // namespace harmonic_engine
