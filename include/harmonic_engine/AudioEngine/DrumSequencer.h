#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>
#include <array>
#include <functional>

namespace harmonic_engine
{

struct DrumStepNote
{
    int noteNumber = 36;
    float velocity = 0.8f;
    float probability = 1.0f;
    bool accent = false;
    int flam = 0;
};

struct DrumStep
{
    std::vector<DrumStepNote> notes;
    bool active = false;
    float swingOffset = 0.0f;
};

struct DrumMapEntry
{
    int noteNumber;
    const char* name;
    int defaultNote;
};

class DrumSequencer
{
public:
    DrumSequencer();
    ~DrumSequencer();

    // Step management
    void setNumSteps(int steps);
    int getNumSteps() const;

    void setStepActive(int index, bool active);
    bool isStepActive(int index) const;

    void addNoteToStep(int stepIndex, const DrumStepNote& note);
    void clearStep(int stepIndex);
    void clearAllSteps();
    void clearPattern();

    int getNumNotesInStep(int stepIndex) const;
    DrumStepNote getNoteInStep(int stepIndex, int noteIndex) const;

    // Timing
    void setResolution(int resolution);
    int getResolution() const;

    void setSwing(float amount);
    float getSwing() const;

    void setStepSwing(int stepIndex, float offset);
    float getStepSwing(int stepIndex) const;

    void setPatternLength(int beats);
    int getPatternLength() const;

    // Pattern generation
    void randomizePattern(float density = 0.4f);
    void fillPattern(int noteNumber, float velocity);
    void fillPatternWithVariation(int noteNumber, float velocity, float variation);
    void generateBeat(int genre); // 0=fourOnFloor, 1=rock, 2=breakbeat, 3=halfTime, 4=latin, 5=garage

    // Drum type mapping
    static int getDefaultNoteForDrumType(int drumTypeIndex);
    static int getNumDrumTypes();
    static DrumMapEntry getDrumMapEntry(int index);
    static int noteToDrumTypeIndex(int noteNumber);

    // Callbacks
    std::function<void(int stepIndex, int noteNumber, float velocity)> onStepTriggered;

    // Audio
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::MidiBuffer& midiBuffer,
                      double positionInSeconds,
                      double tempo,
                      double sampleRate,
                      bool transportPlaying);

    void reset();

    // Serialization
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& tree);

    static const int DefaultSteps = 16;
    static const int NumDrumTypes = 12;
    static const DrumMapEntry DrumMap[NumDrumTypes];

private:
    std::vector<DrumStep> steps;
    int resolution = 4;
    float swing = 0.0f;
    int patternLength = 4;

    double currentSampleRate = 44100.0;

    int lastTriggeredStep = -1;
    int currentStep = 0;
    double lastStepTime = 0.0;

    double getStepDuration(double tempo) const;
};

} // namespace harmonic_engine