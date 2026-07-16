#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/GUI/DesignTokens.h"

namespace harmonic_engine
{

class HarmonicLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HarmonicLookAndFeel();
    ~HarmonicLookAndFeel() override;

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOver,
                              bool isButtonDown) override;

    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g,
                      int width, int height,
                      bool isButtonDown,
                      int buttonX, int buttonY,
                      int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    juce::Colour getDarkBackground() const;
    juce::Colour getMediumBackground() const;
    juce::Colour getLightBackground() const;
    juce::Colour getAccentColour() const;
    juce::Colour getMutedTextColour() const;

    static juce::Colour getMeterGreen();
    static juce::Colour getMeterYellow();
    static juce::Colour getMeterRed();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonicLookAndFeel)
};

} // namespace harmonic_engine
