#include "harmonic_engine/GUI/ZoomControls.h"

namespace harmonic_engine
{

ZoomControls::ZoomControls()
{
    zoomSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    zoomSlider.setRange(minPixelsPerSecond, maxPixelsPerSecond, 1.0);
    zoomSlider.setValue(currentPixelsPerSecond, juce::dontSendNotification);
    zoomSlider.addListener(this);
    addAndMakeVisible(zoomSlider);

    zoomLabel.setText("Zoom:", juce::dontSendNotification);
    addAndMakeVisible(zoomLabel);
}

ZoomControls::~ZoomControls()
{
}

void ZoomControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff16213e));
    g.fillRoundedRectangle(bounds, 4.0f);
}

void ZoomControls::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(8, 4);

    zoomLabel.setBounds(bounds.getX(), bounds.getY(), 45, bounds.getHeight());
    bounds.removeFromLeft(50);

    zoomSlider.setBounds(bounds);
}

void ZoomControls::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &zoomSlider)
    {
        currentPixelsPerSecond = zoomSlider.getValue();

        if (onZoomChanged)
            onZoomChanged(currentPixelsPerSecond);
    }
}

double ZoomControls::getPixelsPerSecond() const
{
    return currentPixelsPerSecond;
}

void ZoomControls::setZoomRange(double minPPS, double maxPPS)
{
    minPixelsPerSecond = minPPS;
    maxPixelsPerSecond = maxPPS;
    zoomSlider.setRange(minPPS, maxPPS, 1.0);
}

void ZoomControls::setPixelsPerSecond(double pps)
{
    currentPixelsPerSecond = juce::jlimit(minPixelsPerSecond, maxPixelsPerSecond, pps);
    zoomSlider.setValue(currentPixelsPerSecond, juce::dontSendNotification);
}

void ZoomControls::zoomIn()
{
    setPixelsPerSecond(currentPixelsPerSecond + 20.0);
}

void ZoomControls::zoomOut()
{
    setPixelsPerSecond(currentPixelsPerSecond - 20.0);
}

} // namespace harmonic_engine
