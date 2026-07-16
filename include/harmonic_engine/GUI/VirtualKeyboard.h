#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "harmonic_engine/AudioEngine/Engine.h"

namespace harmonic_engine
{

class VirtualKeyboard : public juce::MidiKeyboardComponent
{
public:
    VirtualKeyboard(Engine& engine);
    ~VirtualKeyboard() override;

    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

private:
    class InternalListener : public juce::MidiKeyboardState::Listener
    {
    public:
        InternalListener(VirtualKeyboard& parent) : owner(parent) {}
        void handleNoteOn(juce::MidiKeyboardState*, int channel, int note, float velocity) override;
        void handleNoteOff(juce::MidiKeyboardState*, int channel, int note, float velocity) override;
    private:
        VirtualKeyboard& owner;
    };

    Engine& audioEngine;
    juce::MidiKeyboardState keyboardState;
    InternalListener listener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualKeyboard)
};

} // namespace harmonic_engine
