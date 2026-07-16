#include "harmonic_engine/AudioEngine/SampleLoader.h"

namespace harmonic_engine
{

SampleLoader::SampleLoader()
{
    formatManager.registerBasicFormats();
}

SampleLoader::~SampleLoader()
{
    clearAll();
}

juce::String SampleLoader::getKeyFor(const juce::File& file) const
{
    return file.getFullPathName();
}

SampleLoader::SampleInfo* SampleLoader::load(const juce::File& file)
{
    if (!file.existsAsFile())
        return nullptr;

    juce::String key = getKeyFor(file);

    {
        const juce::ScopedLock lock(mutex);
        auto it = cache.find(key);
        if (it != cache.end())
        {
            it->second.refCount++;
            return &it->second.info;
        }
    }

    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
    if (reader == nullptr)
        return nullptr;

    CachedSample cached;
    cached.info.sampleRate = reader->sampleRate;
    cached.info.lengthInSamples = reader->lengthInSamples;
    cached.info.buffer.setSize(static_cast<int>(reader->numChannels),
                               static_cast<int>(reader->lengthInSamples));
    reader->read(&cached.info.buffer, 0,
                 static_cast<int>(reader->lengthInSamples),
                 0, true, true);
    cached.refCount = 1;

    {
        const juce::ScopedLock lock(mutex);
        auto result = cache.insert({ key, std::move(cached) });
        return &result.first->second.info;
    }
}

SampleLoader::SampleInfo* SampleLoader::find(const juce::File& file)
{
    juce::String key = getKeyFor(file);
    const juce::ScopedLock lock(mutex);
    auto it = cache.find(key);
    if (it != cache.end())
        return &it->second.info;
    return nullptr;
}

void SampleLoader::release(const juce::File& file)
{
    juce::String key = getKeyFor(file);
    const juce::ScopedLock lock(mutex);

    auto it = cache.find(key);
    if (it != cache.end())
    {
        it->second.refCount--;
        if (it->second.refCount <= 0)
            cache.erase(it);
    }
}

void SampleLoader::clearUnused()
{
    const juce::ScopedLock lock(mutex);

    for (auto it = cache.begin(); it != cache.end();)
    {
        if (it->second.refCount <= 0)
            it = cache.erase(it);
        else
            ++it;
    }
}

void SampleLoader::clearAll()
{
    const juce::ScopedLock lock(mutex);
    cache.clear();
}

bool SampleLoader::isLoaded(const juce::File& file) const
{
    juce::String key = getKeyFor(file);
    const juce::ScopedLock lock(mutex);
    return cache.find(key) != cache.end();
}

} // namespace harmonic_engine
