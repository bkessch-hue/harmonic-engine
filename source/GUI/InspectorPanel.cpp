#include "harmonic_engine/GUI/InspectorPanel.h"

namespace harmonic_engine
{

InspectorPanel::InspectorPanel()
{
    // Track name
    nameLabel.setText("No Track Selected", juce::dontSendNotification);
    nameLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeLarge))));
    nameLabel.setColour(juce::Label::textColourId, Tokens::Colours::textPrimary());
    addAndMakeVisible(nameLabel);

    // Track type
    typeLabel.setText("", juce::dontSendNotification);
    typeLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    typeLabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    addAndMakeVisible(typeLabel);

    // Volume
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    volumeLabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    volumeLabel.setTooltip("Track volume level");
    addAndMakeVisible(volumeLabel);

    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 2.0, 0.01);
    volumeSlider.setValue(1.0, juce::dontSendNotification);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, Tokens::kButtonHeight);
    volumeSlider.onValueChange = [this]() {
        if (currentTrack != nullptr)
        {
            currentTrack->setGain(static_cast<float>(volumeSlider.getValue()));
            if (onVolumeChanged) onVolumeChanged(static_cast<float>(volumeSlider.getValue()));
        }
    };
    addAndMakeVisible(volumeSlider);

    // Pan
    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    panLabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    addAndMakeVisible(panLabel);

    panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    panSlider.setRange(-1.0, 1.0, 0.01);
    panSlider.setValue(0.0, juce::dontSendNotification);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, Tokens::kButtonHeight);
    panSlider.onValueChange = [this]() {
        if (currentTrack != nullptr)
        {
            currentTrack->setPan(static_cast<float>(panSlider.getValue()));
            if (onPanChanged) onPanChanged(static_cast<float>(panSlider.getValue()));
        }
    };
    addAndMakeVisible(panSlider);

    // Send A
    sendALabel.setText("Send A", juce::dontSendNotification);
    sendALabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    sendALabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    addAndMakeVisible(sendALabel);

    sendASlider.setSliderStyle(juce::Slider::LinearHorizontal);
    sendASlider.setRange(0.0, 1.0, 0.01);
    sendASlider.setValue(0.0, juce::dontSendNotification);
    sendASlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    sendASlider.onValueChange = [this]() {
        if (currentTrack != nullptr)
            currentTrack->setBusSendLevel(0, static_cast<float>(sendASlider.getValue()));
    };
    addAndMakeVisible(sendASlider);

    // Send B
    sendBLabel.setText("Send B", juce::dontSendNotification);
    sendBLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    sendBLabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    addAndMakeVisible(sendBLabel);

    sendBSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    sendBSlider.setRange(0.0, 1.0, 0.01);
    sendBSlider.setValue(0.0, juce::dontSendNotification);
    sendBSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    sendBSlider.onValueChange = [this]() {
        if (currentTrack != nullptr)
            currentTrack->setBusSendLevel(1, static_cast<float>(sendBSlider.getValue()));
    };
    addAndMakeVisible(sendBSlider);

    // Output
    outputLabel.setText("Output", juce::dontSendNotification);
    outputLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    outputLabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    addAndMakeVisible(outputLabel);

    outputCombo.addItem("Master Out", 1);
    outputCombo.addItem("Bus A", 2);
    outputCombo.addItem("Bus B", 3);
    outputCombo.setSelectedId(1, juce::dontSendNotification);
    outputCombo.setColour(juce::ComboBox::backgroundColourId, Tokens::Colours::bgRaised());
    outputCombo.setColour(juce::ComboBox::textColourId, Tokens::Colours::textPrimary());
    addAndMakeVisible(outputCombo);

    // Effect Rack (inserts)
    addAndMakeVisible(effectRack);
}

InspectorPanel::~InspectorPanel() = default;

void InspectorPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(Tokens::Colours::bgDark());

    // Title bar
    auto titleBounds = bounds.removeFromTop(Tokens::kSpace24);
    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(titleBounds);
    g.setColour(Tokens::Colours::textPrimary());
    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeMedium))));
    g.drawText("Inspector", titleBounds.reduced(Tokens::kSpace8, 0),
               juce::Justification::centredLeft);

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(Tokens::kSpace24, 0.0f, static_cast<float>(getWidth()));

    // Section separators
    int y = Tokens::kSpace24 + Tokens::kSpace8;
    auto remaining = bounds.withTop(y);

    // Channel section separator
    int channelSepY = y + 70;
    if (channelSepY < bounds.getBottom())
    {
        g.setColour(Tokens::Colours::borderSubtle());
        g.drawHorizontalLine(channelSepY, Tokens::kSpace8,
                             static_cast<float>(getWidth() - Tokens::kSpace8));
    }

    // Sends section separator
    int sendsSepY = channelSepY + 70;
    if (sendsSepY < bounds.getBottom())
    {
        g.setColour(Tokens::Colours::borderSubtle());
        g.drawHorizontalLine(sendsSepY, Tokens::kSpace8,
                             static_cast<float>(getWidth() - Tokens::kSpace8));
    }

    // Output section separator
    int outputSepY = sendsSepY + 30;
    if (outputSepY < bounds.getBottom())
    {
        g.setColour(Tokens::Colours::borderSubtle());
        g.drawHorizontalLine(outputSepY, Tokens::kSpace8,
                             static_cast<float>(getWidth() - Tokens::kSpace8));
    }

    // Section labels
    g.setColour(Tokens::Colours::textDisabled());
    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    g.drawText("CHANNEL", Tokens::kSpace8, y, 100, Tokens::kSpace16,
               juce::Justification::centredLeft);
    g.drawText("SENDS", Tokens::kSpace8, channelSepY, 100, Tokens::kSpace16,
               juce::Justification::centredLeft);
    g.drawText("OUTPUT", Tokens::kSpace8, sendsSepY, 100, Tokens::kSpace16,
               juce::Justification::centredLeft);
    g.drawText("INSERTS", Tokens::kSpace8, outputSepY, 100, Tokens::kSpace16,
               juce::Justification::centredLeft);
}

void InspectorPanel::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(Tokens::kSpace24); // title bar
    bounds.reduce(Tokens::kSpace8, Tokens::kSpace4);

    int y = bounds.getY();
    int w = bounds.getWidth();
    const int labelH = Tokens::kSpace16;
    const int sliderH = Tokens::kSliderHeight;
    const int gap = Tokens::kSpace4;

    // Track name
    nameLabel.setBounds(bounds.getX(), y, w, labelH + 4);
    y += labelH + 4;
    typeLabel.setBounds(bounds.getX(), y, w, labelH);
    y += labelH + Tokens::kSpace8;

    // Volume
    volumeLabel.setBounds(bounds.getX(), y, 50, labelH);
    volumeSlider.setBounds(bounds.getX() + 52, y, w - 52, sliderH);
    y += sliderH + gap;

    // Pan
    panLabel.setBounds(bounds.getX(), y, 50, labelH);
    panSlider.setBounds(bounds.getX() + 52, y, w - 52, sliderH);
    y += sliderH + Tokens::kSpace12;

    // Sends
    sendALabel.setBounds(bounds.getX(), y, 50, labelH);
    sendASlider.setBounds(bounds.getX() + 52, y, w - 52, sliderH);
    y += sliderH + gap;
    sendBLabel.setBounds(bounds.getX(), y, 50, labelH);
    sendBSlider.setBounds(bounds.getX() + 52, y, w - 52, sliderH);
    y += sliderH + Tokens::kSpace12;

    // Output
    outputLabel.setBounds(bounds.getX(), y, 50, labelH);
    outputCombo.setBounds(bounds.getX() + 52, y, w - 52, Tokens::kComboHeight);
    y += Tokens::kComboHeight + Tokens::kSpace12;

    // Inserts (EffectRack fills remaining space)
    effectRack.setBounds(bounds.getX(), y, w, bounds.getBottom() - y);
}

void InspectorPanel::setTrack(Track* track)
{
    currentTrack = track;
    updateFromTrack();
}

Track* InspectorPanel::getTrack() const
{
    return currentTrack;
}

void InspectorPanel::setEffectChain(EffectChain* chain)
{
    effectRack.setEffectChain(chain);
}

void InspectorPanel::updateFromTrack()
{
    if (currentTrack != nullptr)
    {
        nameLabel.setText(currentTrack->getName(), juce::dontSendNotification);
        typeLabel.setText(currentTrack->isMidiTrack() ? "MIDI Track" : "Audio Track",
                          juce::dontSendNotification);
        volumeSlider.setValue(currentTrack->getGain(), juce::dontSendNotification);
        panSlider.setValue(currentTrack->getPan(), juce::dontSendNotification);
        sendASlider.setValue(currentTrack->getBusSendLevel(0), juce::dontSendNotification);
        sendBSlider.setValue(currentTrack->getBusSendLevel(1), juce::dontSendNotification);
        setEffectChain(&currentTrack->getEffectChain());
    }
    else
    {
        nameLabel.setText("No Track Selected", juce::dontSendNotification);
        typeLabel.setText("", juce::dontSendNotification);
        volumeSlider.setValue(1.0, juce::dontSendNotification);
        panSlider.setValue(0.0, juce::dontSendNotification);
        sendASlider.setValue(0.0, juce::dontSendNotification);
        sendBSlider.setValue(0.0, juce::dontSendNotification);
        setEffectChain(nullptr);
    }
    repaint();
}

} // namespace harmonic_engine
