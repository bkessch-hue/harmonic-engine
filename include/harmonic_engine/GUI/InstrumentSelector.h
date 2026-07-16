#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/Instrument.h"
#include "harmonic_engine/AudioEngine/InstrumentManager.h"

namespace harmonic_engine
{

class InstrumentSelector : public juce::Component,
                           public juce::ComboBox::Listener
{
public:
    InstrumentSelector();
    ~InstrumentSelector() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void setSelectedInstrument(InstrumentType type);
    InstrumentType getSelectedInstrument() const;

    std::function<void(InstrumentType newType)> onInstrumentChanged;

private:
    juce::ComboBox instrumentCombo;
    juce::Label instrumentLabel;

    void populateCombo();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentSelector)
};

} // namespace harmonic_engine
