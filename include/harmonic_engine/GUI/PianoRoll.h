#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/MidiClip.h"
#include "harmonic_engine/AudioEngine/Transport.h"

namespace harmonic_engine {

class PianoRoll : public juce::Component,
                  public juce::ScrollBar::Listener,
                  private juce::Timer
{
public:
    PianoRoll(Transport& transport);
    ~PianoRoll() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override;

    void setMidiClip(MidiClip* clip);
    void setPixelsPerSecond(double pps);
    void setSnapEnabled(bool snap);
    void setSnapDivision(double divisionBeats);

    std::function<void()> onNotesChanged;

private:
    Transport& transport;
    MidiClip* currentClip = nullptr;

    double pixelsPerSecond = 100.0;
    int noteHeight = 14;
    int keyboardWidth = 60;
    int numOctaves = 5;
    int lowestNote = 36;
    int highestNote = 96;

    bool snapEnabled = true;
    double snapDivision = 0.25;

    double scrollX = 0.0;
    double scrollY = 0.0;

    enum class DragMode { None, Creating, Moving, Resizing, Selecting };
    DragMode dragMode = DragMode::None;
    int dragNoteIndex = -1;
    double dragStartTime = 0.0;
    int dragStartNote = 0;
    double dragOriginalDuration = 0.0;
    double dragOriginalStartTime = 0.0;
    juce::Rectangle<int> dragStartBounds;

    juce::Array<int> selectedNotes;
    int hoveredNoteIndex = -1;
    int hoveredKeyNote = -1;

    int yToNoteNumber(int y) const;
    int noteNumberToY(int noteNum) const;
    double xToTime(int x) const;
    int timeToX(double time) const;
    void drawPianoKeyboard(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawGrid(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawNotes(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawPlayhead(juce::Graphics& g, juce::Rectangle<int> bounds);
    double snapTime(double time) const;
    bool isBlackKey(int noteNum) const;
    juce::Colour getNoteColour(int noteNum) const;
    juce::String getNoteName(int noteNum) const;
    int getNoteAtPosition(int x, int y) const;
    bool isOnResizeEdge(int x, int y, int noteIndex) const;

    void timerCallback() override;
    void scrollBarMoved(juce::ScrollBar* scrollBar, double newRangeStart) override;

    int getGridWidth() const;
    int getGridHeight() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRoll)
};

} // namespace harmonic_engine
