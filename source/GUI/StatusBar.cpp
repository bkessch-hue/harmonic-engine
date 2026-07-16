#include "harmonic_engine/GUI/StatusBar.h"

namespace harmonic_engine
{

StatusBar::StatusBar(Engine& engine)
    : audioEngine(engine)
{
    startTimerHz(10);
}

StatusBar::~StatusBar()
{
    stopTimer();
}

void StatusBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(Tokens::Colours::bgDark());

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));

    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    g.setColour(Tokens::Colours::textSecondary());

    // Left section — audio device info
    int x = Tokens::kSpace12;
    const int h = getHeight();
    const int segW = 140;

    auto drawSegment = [&](const juce::String& text)
    {
        g.drawText(text, x, 0, segW, h, juce::Justification::centredLeft);
        x += segW;

        // Separator dot
        g.setColour(Tokens::Colours::borderStrong());
        g.fillEllipse(static_cast<float>(x) - 3.0f, static_cast<float>(h) / 2.0f - 1.0f, 2.0f, 2.0f);
        g.setColour(Tokens::Colours::textSecondary());
        x += Tokens::kSpace8;
    };

    drawSegment(getAudioDeviceString());
    drawSegment(getSampleRateString());
    drawSegment(getBufferSizeString());
    drawSegment(getCpuUsageString());

    // Right section — MIDI activity
    int rightX = getWidth() - Tokens::kSpace12;
    g.setColour(midiActivity ? Tokens::Colours::accent() : Tokens::Colours::textDisabled());
    g.drawText("MIDI", rightX - 40, 0, 40, h, juce::Justification::centredRight);

    // MIDI activity indicator dot
    g.fillEllipse(static_cast<float>(rightX - 14), static_cast<float>(h) / 2.0f - 3.0f, 6.0f, 6.0f);
}

void StatusBar::resized()
{
}

void StatusBar::setMidiActivity(bool active)
{
    midiActivity = active;
    if (active)
        midiActivityTimeout = 10; // ~1 second at 10Hz timer
    repaint();
}

void StatusBar::timerCallback()
{
    // Decay MIDI activity indicator
    if (midiActivityTimeout > 0)
    {
        --midiActivityTimeout;
        if (midiActivityTimeout <= 0)
        {
            midiActivity = false;
            repaint();
        }
    }

    repaint();
}

juce::String StatusBar::getAudioDeviceString() const
{
    auto* device = audioEngine.getDeviceManager().getCurrentAudioDevice();
    if (device != nullptr)
        return device->getTypeName() + ": " + device->getName().upToFirstOccurrenceOf(":", false, false);
    return "No Device";
}

juce::String StatusBar::getSampleRateString() const
{
    auto* device = audioEngine.getDeviceManager().getCurrentAudioDevice();
    if (device != nullptr)
        return juce::String(device->getCurrentSampleRate() / 1000.0, 1) + " kHz";
    return "-- kHz";
}

juce::String StatusBar::getBufferSizeString() const
{
    auto* device = audioEngine.getDeviceManager().getCurrentAudioDevice();
    if (device != nullptr)
        return juce::String(device->getCurrentBufferSizeSamples()) + " samples";
    return "-- samples";
}

juce::String StatusBar::getCpuUsageString() const
{
    auto usage = audioEngine.getDeviceManager().getCpuUsage() * 100.0;
    return "CPU: " + juce::String(usage, 1) + "%";
}

} // namespace harmonic_engine
