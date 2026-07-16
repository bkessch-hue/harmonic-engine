#include "harmonic_engine/GUI/LookAndFeel.h"

namespace harmonic_engine
{

HarmonicLookAndFeel::HarmonicLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, Tokens::Colours::bgDarkest());
    setColour(juce::PopupMenu::backgroundColourId, Tokens::Colours::bgBase());
    setColour(juce::PopupMenu::textColourId, Tokens::Colours::textPrimary());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Tokens::Colours::accent());
}

HarmonicLookAndFeel::~HarmonicLookAndFeel()
{
}

void HarmonicLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool isMouseOver,
                                               bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto colour = backgroundColour;

    if (isButtonDown)
        colour = colour.brighter(0.3f);
    else if (isMouseOver)
        colour = colour.brighter(0.1f);

    g.setColour(colour);
    g.fillRoundedRectangle(bounds, Tokens::kCornerRadiusSmall);
}

void HarmonicLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                           int x, int y, int width, int height,
                                           float sliderPos,
                                           float minSliderPos,
                                           float maxSliderPos,
                                           const juce::Slider::SliderStyle style,
                                           juce::Slider& slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos, style, slider);

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();

    if (style == juce::Slider::SliderStyle::LinearVertical)
    {
        auto trackWidth = 4.0f;
        auto thumbWidth = 12.0f;
        auto thumbHeight = 8.0f;

        auto trackX = bounds.getCentreX() - trackWidth / 2.0f;
        auto trackBounds = juce::Rectangle<float>(trackX, bounds.getY() + thumbHeight / 2.0f,
                                                  trackWidth, bounds.getHeight() - thumbHeight);

        g.setColour(Tokens::Colours::bgOverlay());
        g.fillRoundedRectangle(trackBounds, trackWidth / 2.0f);

        auto normalisedPos = (sliderPos - minSliderPos) / (maxSliderPos - minSliderPos);
        if (maxSliderPos == minSliderPos)
            normalisedPos = 0.5f;

        auto thumbY = bounds.getY() + thumbHeight / 2.0f +
                      (1.0f - normalisedPos) * (bounds.getHeight() - thumbHeight);

        auto thumbBounds = juce::Rectangle<float>(
            bounds.getCentreX() - thumbWidth / 2.0f,
            thumbY - thumbHeight / 2.0f,
            thumbWidth, thumbHeight);

        g.setColour(Tokens::Colours::accent());
        g.fillRoundedRectangle(thumbBounds, Tokens::kCornerRadiusSmall);
    }
}

void HarmonicLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                           int x, int y, int width, int height,
                                           float sliderPos,
                                           float rotaryStartAngle,
                                           float rotaryEndAngle,
                                           juce::Slider& slider)
{
    juce::ignoreUnused(slider);

    auto radius = static_cast<float>(juce::jmin(width, height)) / 2.0f - 4.0f;
    auto centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    auto centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;

    juce::Path track;
    track.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);

    g.setColour(Tokens::Colours::bgOverlay());
    g.strokePath(track, juce::PathStrokeType(3.0f));

    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Path thumb;
    auto tw = radius * 0.15f;
    thumb.addRectangle(-tw / 2.0f, -radius, tw, radius * 0.4f);

    g.setColour(Tokens::Colours::accent());

    {
        juce::Graphics::ScopedSaveState saved(g);
        g.setOrigin(juce::Point<int>(static_cast<int>(centreX), static_cast<int>(centreY)));
        g.addTransform(juce::AffineTransform::rotation(angle));
        g.fillPath(thumb);
    }
}

void HarmonicLookAndFeel::drawComboBox(juce::Graphics& g,
                                       int width, int height,
                                       bool isButtonDown,
                                       int buttonX, int buttonY,
                                       int buttonW, int buttonH,
                                       juce::ComboBox& box)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, box);

    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

    g.setColour(Tokens::Colours::bgRaised());
    g.fillRoundedRectangle(bounds, Tokens::kCornerRadiusSmall);

    g.setColour(Tokens::Colours::borderDefault());
    g.drawRoundedRectangle(bounds, Tokens::kCornerRadiusSmall, 1.0f);

    auto arrowArea = juce::Rectangle<int>(width - 20, 0, 20, height).toFloat();
    auto path = juce::Path();
    path.addTriangle(arrowArea.getCentreX() - 4.0f, arrowArea.getCentreY() - 2.0f,
                     arrowArea.getCentreX() + 4.0f, arrowArea.getCentreY() - 2.0f,
                     arrowArea.getCentreX(), arrowArea.getCentreY() + 3.0f);

    g.setColour(Tokens::Colours::textPrimary());
    g.fillPath(path);
}

void HarmonicLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(Tokens::Colours::bgBase());
    g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width),
                           static_cast<float>(height), Tokens::kCornerRadiusMedium);

    g.setColour(Tokens::Colours::borderDefault());
    g.drawRoundedRectangle(0.0f, 0.0f, static_cast<float>(width),
                           static_cast<float>(height), Tokens::kCornerRadiusMedium, 1.0f);
}

juce::Colour HarmonicLookAndFeel::getDarkBackground() const
{
    return Tokens::Colours::bgDarkest();
}

juce::Colour HarmonicLookAndFeel::getMediumBackground() const
{
    return Tokens::Colours::bgBase();
}

juce::Colour HarmonicLookAndFeel::getLightBackground() const
{
    return Tokens::Colours::bgRaised();
}

juce::Colour HarmonicLookAndFeel::getAccentColour() const
{
    return Tokens::Colours::accent();
}

juce::Colour HarmonicLookAndFeel::getMutedTextColour() const
{
    return Tokens::Colours::textSecondary();
}

juce::Colour HarmonicLookAndFeel::getMeterGreen()
{
    return Tokens::Colours::meterLow();
}

juce::Colour HarmonicLookAndFeel::getMeterYellow()
{
    return Tokens::Colours::meterMid();
}

juce::Colour HarmonicLookAndFeel::getMeterRed()
{
    return Tokens::Colours::meterPeak();
}

} // namespace harmonic_engine
