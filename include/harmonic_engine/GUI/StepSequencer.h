#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include "harmonic_engine/AudioEngine/Transport.h"
#include "harmonic_engine/AudioEngine/MidiClip.h"
#include "harmonic_engine/AudioEngine/DrumSequencer.h"
#include "harmonic_engine/GUI/DesignTokens.h"

namespace harmonic_engine
{

class StepSequencer : public juce::Component,
                      private juce::Timer,
                      public juce::Button::Listener,
                      public juce::ComboBox::Listener,
                      public juce::Slider::Listener
{
public:
    StepSequencer(Transport& transport, DrumSequencer& drumSeq);
    ~StepSequencer() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    void setMidiClip(MidiClip* clip);
    void setNumSteps(int steps);
    void setStepsPerBeat(int spb);

    enum class Genre
    {
        FourOnFloor,
        RockBeat,
        Breakbeat,
        HalfTime,
        Shuffle,
        DnB,
        House,
        Trap,
        JazzBrush,
        Funk,
        Punk,
        Reggae,
        NumberOfGenres
    };

    void generatePattern(Genre genre);
    void generateFill();
    void generateBrushPattern(int density = 40);
    void mutatePattern(float amount = 0.15f);
    void randomizePattern();
    void clearPattern();

    std::function<void()> onPatternChanged;

private:
    Transport& transport;
    DrumSequencer& drumSequencer;
    MidiClip* currentClip = nullptr;

    int numSteps = 16;
    int stepsPerBeat = 4;

    struct DrumRow
    {
        int noteNumber;
        juce::String name;
        juce::Colour colour;
        bool muted = false;
    };
    std::vector<DrumRow> drumRows;

    int labelWidth = 80;
    int stepWidth = 36;
    int rowHeight = 28;

    std::vector<std::vector<bool>> pattern;
    std::vector<float> accent;

    int currentPlayStep = -1;

    // Control buttons
    std::unique_ptr<juce::ComboBox> genreCombo;
    std::unique_ptr<juce::TextButton> generateBtn;
    std::unique_ptr<juce::TextButton> fillBtn;
    std::unique_ptr<juce::TextButton> mutateBtn;
    std::unique_ptr<juce::TextButton> randomBtn;
    std::unique_ptr<juce::TextButton> clearBtn;
    std::unique_ptr<juce::Slider> swingSlider;
    std::unique_ptr<juce::Label> swingLabel;
    std::unique_ptr<juce::Slider> densitySlider;
    std::unique_ptr<juce::Label> densityLabel;

    // Mute per row
    std::vector<std::unique_ptr<juce::TextButton>> muteButtons;

    void rebuildPattern();
    void syncClipToPattern();
    void syncToDrumSequencer();
    void initDrumRows();
    void createControls();

    void timerCallback() override;
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* combo) override;
    void sliderValueChanged(juce::Slider* slider) override;

    struct PatternStep
    {
        int step;
        int row;
        float velocity;
    };

    void placeNote(int row, int step, float velocity = 0.8f);
    void removeNote(int row, int step);
    bool hasNote(int row, int step) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepSequencer)
};

} // namespace harmonic_engine