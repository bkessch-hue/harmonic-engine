#include "harmonic_engine/GUI/InstrumentSelector.h"

namespace harmonic_engine
{

InstrumentSelector::InstrumentSelector()
{
    instrumentLabel.setText("Instrument:", juce::dontSendNotification);
    instrumentLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
    instrumentLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    addAndMakeVisible(instrumentLabel);

    populateCombo();
    instrumentCombo.addListener(this);
    addAndMakeVisible(instrumentCombo);
}

InstrumentSelector::~InstrumentSelector()
{
    instrumentCombo.removeListener(this);
}

void InstrumentSelector::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff16213e));
    g.fillRoundedRectangle(bounds, 4.0f);
}

void InstrumentSelector::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(8, 4);

    instrumentLabel.setBounds(bounds.getX(), bounds.getY(), 65, bounds.getHeight());
    bounds.removeFromLeft(70);
    instrumentCombo.setBounds(bounds);
}

void InstrumentSelector::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &instrumentCombo)
    {
        int idx = instrumentCombo.getSelectedId() - 1;
        auto& manager = InstrumentManager::getInstance();

        if (idx >= 0 && idx < manager.getNumInstruments())
        {
            auto info = manager.getInstrumentInfo(idx);
            if (onInstrumentChanged)
                onInstrumentChanged(info.type);
        }
    }
}

void InstrumentSelector::populateCombo()
{
    instrumentCombo.clear();
    auto& manager = InstrumentManager::getInstance();

    for (int i = 0; i < manager.getNumInstruments(); ++i)
    {
        auto info = manager.getInstrumentInfo(i);
        instrumentCombo.addItem(info.name, i + 1);
    }

    instrumentCombo.setSelectedId(1, juce::dontSendNotification);
}

void InstrumentSelector::setSelectedInstrument(InstrumentType type)
{
    auto& manager = InstrumentManager::getInstance();

    for (int i = 0; i < manager.getNumInstruments(); ++i)
    {
        auto info = manager.getInstrumentInfo(i);
        if (info.type == type)
        {
            instrumentCombo.setSelectedId(i + 1, juce::dontSendNotification);
            return;
        }
    }
}

InstrumentType InstrumentSelector::getSelectedInstrument() const
{
    int idx = instrumentCombo.getSelectedId() - 1;
    auto& manager = InstrumentManager::getInstance();

    if (idx >= 0 && idx < manager.getNumInstruments())
        return manager.getInstrumentInfo(idx).type;

    return InstrumentType::Sine;
}

} // namespace harmonic_engine
