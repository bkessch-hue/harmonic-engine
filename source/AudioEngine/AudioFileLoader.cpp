#include "harmonic_engine/AudioEngine/AudioFileLoader.h"

namespace harmonic_engine
{

AudioFileLoader::AudioFileLoader()
{
    formatManager.registerBasicFormats();
    formatManager.registerFormat(new juce::FlacAudioFormat(), false);
}

AudioFileLoader::~AudioFileLoader()
{
}

juce::AudioFormatManager& AudioFileLoader::getFormatManager()
{
    return formatManager;
}

std::unique_ptr<juce::AudioFormatReader> AudioFileLoader::createReaderFor(const juce::File& file)
{
    return std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
}

bool AudioFileLoader::canUnderstandFile(const juce::File& file) const
{
    for (int i = 0; i < formatManager.getNumKnownFormats(); ++i)
    {
        if (formatManager.getKnownFormat(i)->canHandleFile(file))
            return true;
    }
    return false;
}

juce::StringArray AudioFileLoader::getSupportedFileExtensions() const
{
    juce::StringArray extensions;
    for (int i = 0; i < formatManager.getNumKnownFormats(); ++i)
    {
        auto* fmt = formatManager.getKnownFormat(i);
        for (const auto& ext : fmt->getFileExtensions())
            extensions.add(ext);
    }
    return extensions;
}

juce::File AudioFileLoader::getRecordingDirectory()
{
    auto docsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto recDir = docsDir.getChildFile("Harmonic Engine/Recordings");
    return recDir;
}

void AudioFileLoader::ensureRecordingDirectoryExists()
{
    getRecordingDirectory().createDirectory();
}

} // namespace harmonic_engine
