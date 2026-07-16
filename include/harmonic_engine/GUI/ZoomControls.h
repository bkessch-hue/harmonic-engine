#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace harmonic_engine
{

class ZoomControls : public juce::Component,
                     public juce::Slider::Listener
{
public:
    ZoomControls();
    ~ZoomControls() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;

    double getPixelsPerSecond() const;

    std::function<void(double newPixelsPerSecond)> onZoomChanged;

    void setZoomRange(double minPPS, double maxPPS);
    void setPixelsPerSecond(double pps);
    void zoomIn();
    void zoomOut();

private:
    juce::Slider zoomSlider;
    juce::Label zoomLabel;

    double minPixelsPerSecond = 20.0;
    double maxPixelsPerSecond = 500.0;
    double currentPixelsPerSecond = 100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZoomControls)
};

} // namespace harmonic_engine
