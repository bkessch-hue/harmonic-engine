#include "harmonic_engine/AudioEngine/InstrumentManager.h"

namespace harmonic_engine
{

InstrumentManager::InstrumentManager()
{
    addInstrument("Sine",            InstrumentType::Sine);
    addInstrument("Saw",             InstrumentType::Saw);
    addInstrument("Square",          InstrumentType::Square);
    addInstrument("Triangle",        InstrumentType::Triangle);
    addInstrument("FM Synth",        InstrumentType::FM);
    addInstrument("Electric Piano",  InstrumentType::ElectricPiano);
    addInstrument("Organ",           InstrumentType::Organ);
    addInstrument("Pluck",           InstrumentType::Pluck);
    addInstrument("Wavetable",       InstrumentType::Wavetable);
    addInstrument("Subtractive",     InstrumentType::Subtractive);
    addInstrument("Sampler",         InstrumentType::Sampler);
    addInstrument("Drum",            InstrumentType::Drum);
}

InstrumentManager::~InstrumentManager()
{
}

InstrumentManager& InstrumentManager::getInstance()
{
    static InstrumentManager instance;
    return instance;
}

int InstrumentManager::getNumInstruments() const
{
    return static_cast<int>(instruments.size());
}

InstrumentInfo InstrumentManager::getInstrumentInfo(int index) const
{
    if (index >= 0 && index < static_cast<int>(instruments.size()))
        return instruments[index];
    return { "None", InstrumentType::Sine };
}

std::unique_ptr<juce::Synthesiser> InstrumentManager::createInstrument(InstrumentType type) const
{
    auto synth = std::make_unique<juce::Synthesiser>();
    synth->addSound(new HarmonicSynthSound());

    const int maxVoices = 16;

    switch (type)
    {
        case InstrumentType::Sine:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new SineVoice());
            break;
        case InstrumentType::Saw:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new SawVoice());
            break;
        case InstrumentType::Square:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new SquareVoice());
            break;
        case InstrumentType::Triangle:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new TriangleVoice());
            break;
        case InstrumentType::FM:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new FMSynthVoice());
            break;
        case InstrumentType::ElectricPiano:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new ElectricPianoVoice());
            break;
        case InstrumentType::Organ:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new OrganVoice());
            break;
        case InstrumentType::Pluck:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new PluckVoice());
            break;
        case InstrumentType::Wavetable:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new WavetableSynthVoice());
            break;
        case InstrumentType::Subtractive:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new SubtractiveSynthVoice());
            break;
        case InstrumentType::Sampler:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new SamplerSynthVoice());
            break;
        case InstrumentType::Drum:
            for (int i = 0; i < maxVoices; ++i) synth->addVoice(new DrumSynthVoice());
            break;
    }

    return synth;
}

juce::StringArray InstrumentManager::getInstrumentNames() const
{
    juce::StringArray names;
    for (const auto& inst : instruments)
        names.add(inst.name);
    return names;
}

int InstrumentManager::getInstrumentIndexByName(const juce::String& name) const
{
    for (int i = 0; i < static_cast<int>(instruments.size()); ++i)
    {
        if (instruments[i].name == name)
            return i;
    }
    return -1;
}

const std::vector<InstrumentInfo>& InstrumentManager::getAvailableInstruments()
{
    static InstrumentManager instance;
    return instance.instruments;
}

void InstrumentManager::addInstrument(const juce::String& name, InstrumentType type)
{
    instruments.push_back({ name, type });
}

} // namespace harmonic_engine
