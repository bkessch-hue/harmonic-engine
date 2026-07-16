#include "harmonic_engine/GUI/VirtualKeyboard.h"
#include "harmonic_engine/AudioEngine/Engine.h"

namespace harmonic_engine
{

VirtualKeyboard::VirtualKeyboard(Engine& engine)
    : juce::MidiKeyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      audioEngine(engine),
      listener(*this)
{
    keyboardState.addListener(&listener);
    setOctaveForMiddleC(4);
    setAvailableRange(21, 108);
    setKeyWidth(16.0f);
    setScrollButtonWidth(20);
    setColour(juce::MidiKeyboardComponent::whiteNoteColourId,
              juce::Colour::fromString("0xFFF8F8F3"));
    setColour(juce::MidiKeyboardComponent::blackNoteColourId,
              juce::Colour::fromString("0xFF252A2E"));
    setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,
              juce::Colour::fromString("0xFF555555"));
    setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
              juce::Colour::fromString("0x00000000"));
}

VirtualKeyboard::~VirtualKeyboard()
{
    keyboardState.removeListener(&listener);
}

void VirtualKeyboard::InternalListener::handleNoteOn(juce::MidiKeyboardState*, int, int midiNoteNumber, float velocity)
{
    juce::MidiMessage msg = juce::MidiMessage::noteOn(1, midiNoteNumber, velocity);
    owner.audioEngine.addMidiInputMessage(msg);
}

void VirtualKeyboard::InternalListener::handleNoteOff(juce::MidiKeyboardState*, int, int midiNoteNumber, float velocity)
{
    juce::MidiMessage msg = juce::MidiMessage::noteOff(1, midiNoteNumber, velocity);
    owner.audioEngine.addMidiInputMessage(msg);
}

} // namespace harmonic_engine
