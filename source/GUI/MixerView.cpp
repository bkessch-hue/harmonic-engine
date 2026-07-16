#include "harmonic_engine/GUI/MixerView.h"
#include "harmonic_engine/GUI/LookAndFeel.h"

namespace harmonic_engine
{

ChannelStrip::ChannelStrip(Track& t)
    : track(t)
{
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setRange(0.0, 2.0, 0.01);
    gainSlider.setValue(track.getGain(), juce::dontSendNotification);
    gainSlider.onValueChange = [this]() {
        track.setGain(static_cast<float>(gainSlider.getValue()));
    };
    addAndMakeVisible(gainSlider);

    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    panSlider.setRange(-1.0, 1.0, 0.01);
    panSlider.setValue(track.getPan(), juce::dontSendNotification);
    panSlider.onValueChange = [this]() {
        track.setPan(static_cast<float>(panSlider.getValue()));
    };
    addAndMakeVisible(panSlider);

    muteButton.setButtonText("M");
    muteButton.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe0e0e0));
    muteButton.onClick = [this]() {
        track.setMuted(muteButton.getToggleState());
    };
    addAndMakeVisible(muteButton);

    soloButton.setButtonText("S");
    soloButton.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe0e0e0));
    soloButton.onClick = [this]() {
        track.setSoloed(soloButton.getToggleState());
    };
    addAndMakeVisible(soloButton);

    sendASlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    sendASlider.setRange(0.0, 1.0, 0.01);
    sendASlider.setValue(track.getBusSendLevel(0), juce::dontSendNotification);
    sendASlider.onValueChange = [this]() {
        track.setBusSendLevel(0, static_cast<float>(sendASlider.getValue()));
    };
    sendASlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(sendASlider);

    sendBSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    sendBSlider.setRange(0.0, 1.0, 0.01);
    sendBSlider.setValue(track.getBusSendLevel(1), juce::dontSendNotification);
    sendBSlider.onValueChange = [this]() {
        track.setBusSendLevel(1, static_cast<float>(sendBSlider.getValue()));
    };
    sendBSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(sendBSlider);

    nameLabel.setText(track.getName(), juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(nameLabel);
}

ChannelStrip::~ChannelStrip()
{
}

void ChannelStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff1a1a2e));
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(track.getTrackColour().withAlpha(0.3f));
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour(0xff333355));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    g.setColour(juce::Colour(0xff888888));
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    auto sendLabelBounds = bounds.reduced(4.0f).removeFromBottom(40.0f);
    g.drawText("A", sendLabelBounds.getX(), sendLabelBounds.getY(), 10, 12,
               juce::Justification::centred);
    g.drawText("B", sendLabelBounds.getX(), sendLabelBounds.getY() + 14, 10, 12,
               juce::Justification::centred);

    auto meterX = bounds.getRight() - 8.0f;
    auto meterHeight = bounds.getHeight() - 20.0f;
    auto meterY = bounds.getY() + 10.0f;

    g.setColour(juce::Colour(0xff333355));
    g.fillRect(meterX, meterY, 4.0f, meterHeight);

    float meterLevel = currentPeakLevel;
    float filledHeight = meterHeight * meterLevel;

    juce::Colour meterColour;
    if (meterLevel < 0.6f)
        meterColour = HarmonicLookAndFeel::getMeterGreen();
    else if (meterLevel < 0.85f)
        meterColour = HarmonicLookAndFeel::getMeterYellow();
    else
        meterColour = HarmonicLookAndFeel::getMeterRed();

    g.setColour(meterColour);
    g.fillRect(meterX, meterY + meterHeight - filledHeight, 4.0f, filledHeight);
}

void ChannelStrip::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(4, 4);

    const int stripWidth = bounds.getWidth();
    const int sliderHeight = 100;
    const int rotarySize = 30;
    const int buttonHeight = 20;
    const int sendHeight = 16;
    const int spacing = 4;

    nameLabel.setBounds(bounds.getX(), bounds.getY(), stripWidth, 18);
    bounds.removeFromTop(20);

    gainSlider.setBounds(bounds.getX(), bounds.getY(), stripWidth - 10, sliderHeight);
    bounds.removeFromTop(sliderHeight + spacing);

    panSlider.setBounds(bounds.getX() + (stripWidth - rotarySize) / 2,
                        bounds.getY(), rotarySize, rotarySize);
    bounds.removeFromTop(rotarySize + spacing);

    muteButton.setBounds(bounds.getX(), bounds.getY(), stripWidth / 2 - 2, buttonHeight);
    soloButton.setBounds(bounds.getX() + stripWidth / 2 + 2, bounds.getY(),
                         stripWidth / 2 - 2, buttonHeight);
    bounds.removeFromTop(buttonHeight + spacing);

    sendASlider.setBounds(bounds.getX(), bounds.getY(), stripWidth, sendHeight);
    bounds.removeFromTop(sendHeight + 2);
    sendBSlider.setBounds(bounds.getX(), bounds.getY(), stripWidth, sendHeight);
    bounds.removeFromTop(sendHeight + 2);
}

void ChannelStrip::updateMeter()
{
    currentPeakLevel = track.getPeakLevel();
    repaint();
}

MixerView::MixerView(TrackManager& tm)
    : trackManager(tm)
{
    trackManager.onTracksChanged = [this]() { rebuildStrips(); };
    rebuildStrips();
}

MixerView::~MixerView()
{
}

void MixerView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff0f3460));
    g.fillRoundedRectangle(bounds, 0.0f);

    g.setColour(juce::Colour(0xff333355));
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));

    auto masterBounds = juce::Rectangle<float>(
        static_cast<float>(getWidth()) - 80.0f, 4.0f, 72.0f,
        static_cast<float>(getHeight()) - 8.0f);

    g.setColour(juce::Colour(0xff1a1a2e));
    g.fillRoundedRectangle(masterBounds, 4.0f);

    g.setColour(juce::Colour(0xffe94560).withAlpha(0.3f));
    g.fillRoundedRectangle(masterBounds, 4.0f);

    g.setColour(juce::Colour(0xff333355));
    g.drawRoundedRectangle(masterBounds, 4.0f, 1.0f);

    g.setColour(juce::Colour(0xffe0e0e0));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText("MASTER", masterBounds.reduced(4.0f),
               juce::Justification::centredTop);

    float masterHeight = masterBounds.getHeight() - 20.0f;
    float masterY = masterBounds.getY() + 16.0f;

    for (int ch = 0; ch < 2; ++ch)
    {
        float meterX = masterBounds.getX() + 4.0f + ch * 32.0f;
        float peakLevel = (ch == 0) ? masterPeakLeft : masterPeakRight;
        float filledHeight = masterHeight * peakLevel;

        g.setColour(juce::Colour(0xff333355));
        g.fillRect(meterX, masterY, 24.0f, masterHeight);

        juce::Colour meterColour;
        if (peakLevel < 0.6f)
            meterColour = HarmonicLookAndFeel::getMeterGreen();
        else if (peakLevel < 0.85f)
            meterColour = HarmonicLookAndFeel::getMeterYellow();
        else
            meterColour = HarmonicLookAndFeel::getMeterRed();

        g.setColour(meterColour);
        g.fillRect(meterX, masterY + masterHeight - filledHeight, 24.0f, filledHeight);
    }
}

void MixerView::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(8, 8);

    const int stripWidth = 80;
    const int spacing = 4;

    for (int i = 0; i < static_cast<int>(channelStrips.size()); ++i)
    {
        int x = bounds.getX() + i * (stripWidth + spacing);
        channelStrips[i]->setBounds(x, bounds.getY(), stripWidth, bounds.getHeight());
    }
}

void MixerView::updateMeters()
{
    for (auto& strip : channelStrips)
        strip->updateMeter();
}

void MixerView::setMasterPeakLevels(float left, float right)
{
    masterPeakLeft = left;
    masterPeakRight = right;
    repaint();
}

void MixerView::rebuildStrips()
{
    channelStrips.clear();

    for (int i = 0; i < trackManager.getNumTracks(); ++i)
    {
        auto* track = trackManager.getTrack(i);
        if (track != nullptr)
        {
            auto strip = std::make_unique<ChannelStrip>(*track);
            addAndMakeVisible(strip.get());
            channelStrips.push_back(std::move(strip));
        }
    }

    resized();
    repaint();
}

} // namespace harmonic_engine
