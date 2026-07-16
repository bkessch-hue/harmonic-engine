#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/Transport.h"
#include <vector>

namespace harmonic_engine
{

struct Marker
{
    juce::String name;
    double timeSeconds;
    juce::Colour colour;
};

struct TempoEvent
{
    double timeSeconds;
    double bpm;
};

struct TimeSignature
{
    double timeSeconds;
    int numerator;
    int denominator;
};

class GlobalTracks : public juce::Component,
                     private juce::Timer
{
public:
    GlobalTracks(Transport& transport);
    ~GlobalTracks() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;

    void setPixelsPerSecond(double pps);
    void setViewStartTime(double seconds);

    void addMarker(const juce::String& name, double timeSeconds, juce::Colour colour);
    void removeMarker(int index);
    int getNumMarkers() const;
    Marker getMarker(int index) const;

    void addTempoEvent(double timeSeconds, double bpm);
    void removeTempoEvent(int index);
    int getNumTempoEvents() const;
    TempoEvent getTempoEvent(int index) const;

    void addTimeSignature(double timeSeconds, int num, int den);
    void removeTimeSignature(int index);
    int getNumTimeSignatures() const;
    TimeSignature getTimeSignature(int index) const;

    std::function<void()> onMarkerAdded;
    std::function<void()> onTempoChanged;
    std::function<void()> onTimeSignatureChanged;

private:
    Transport& transport;

    double pixelsPerSecond = 100.0;
    double viewStartTime = 0.0;

    int markerTrackHeight = 30;
    int tempoTrackHeight = 40;
    int timeSigTrackHeight = 30;
    int labelWidth = 80;

    std::vector<Marker> markers;
    std::vector<TempoEvent> tempoEvents;
    std::vector<TimeSignature> timeSignatures;

    int selectedMarkerIndex = -1;

    void timerCallback() override;
    int timeToX(double time) const;
    double xToTime(int x) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalTracks)
};

} // namespace harmonic_engine
