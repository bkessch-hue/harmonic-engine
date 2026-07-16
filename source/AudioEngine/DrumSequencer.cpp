#include "harmonic_engine/AudioEngine/DrumSequencer.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

namespace harmonic_engine
{

const DrumMapEntry DrumSequencer::DrumMap[NumDrumTypes] = {
    { 0, "Kick",       36 },
    { 1, "Snare",      38 },
    { 2, "HihatClosed",42 },
    { 3, "HihatOpen",  46 },
    { 4, "TomLow",     41 },
    { 5, "TomMid",     45 },
    { 6, "TomHigh",    48 },
    { 7, "Clap",       39 },
    { 8, "Rimshot",    37 },
    { 9, "Crash",      49 },
    { 10,"Ride",       51 },
    { 11,"Shaker",     70 }
};

DrumSequencer::DrumSequencer()
{
    setNumSteps(DefaultSteps);
}

DrumSequencer::~DrumSequencer() = default;

void DrumSequencer::setNumSteps(int num)
{
    num = juce::jmax(1, num);
    steps.resize(num);
    for (auto& step : steps)
    {
        step.active = false;
        step.notes.clear();
        step.swingOffset = 0.0f;
    }
}

int DrumSequencer::getNumSteps() const
{
    return static_cast<int>(steps.size());
}

void DrumSequencer::setStepActive(int index, bool active)
{
    if (index >= 0 && index < static_cast<int>(steps.size()))
        steps[index].active = active;
}

bool DrumSequencer::isStepActive(int index) const
{
    return (index >= 0 && index < static_cast<int>(steps.size())) && steps[index].active;
}

void DrumSequencer::addNoteToStep(int stepIndex, const DrumStepNote& note)
{
    if (stepIndex >= 0 && stepIndex < static_cast<int>(steps.size()))
    {
        steps[stepIndex].active = true;
        steps[stepIndex].notes.push_back(note);
    }
}

void DrumSequencer::clearStep(int stepIndex)
{
    if (stepIndex >= 0 && stepIndex < static_cast<int>(steps.size()))
    {
        steps[stepIndex].notes.clear();
        steps[stepIndex].active = false;
        steps[stepIndex].swingOffset = 0.0f;
    }
}

void DrumSequencer::clearAllSteps()
{
    for (auto& step : steps)
    {
        step.notes.clear();
        step.active = false;
        step.swingOffset = 0.0f;
    }
}

void DrumSequencer::clearPattern()
{
    clearAllSteps();
}

int DrumSequencer::getNumNotesInStep(int stepIndex) const
{
    if (stepIndex >= 0 && stepIndex < static_cast<int>(steps.size()))
        return static_cast<int>(steps[stepIndex].notes.size());
    return 0;
}

DrumStepNote DrumSequencer::getNoteInStep(int stepIndex, int noteIndex) const
{
    if (stepIndex >= 0 && stepIndex < static_cast<int>(steps.size()) &&
        noteIndex >= 0 && noteIndex < static_cast<int>(steps[stepIndex].notes.size()))
        return steps[stepIndex].notes[noteIndex];
    return DrumStepNote();
}

void DrumSequencer::setResolution(int res) { resolution = juce::jmax(1, res); }
int DrumSequencer::getResolution() const { return resolution; }

void DrumSequencer::setSwing(float amount) { swing = juce::jlimit(0.0f, 1.0f, amount); }
float DrumSequencer::getSwing() const { return swing; }

void DrumSequencer::setStepSwing(int stepIndex, float offset)
{
    if (stepIndex >= 0 && stepIndex < static_cast<int>(steps.size()))
        steps[stepIndex].swingOffset = juce::jlimit(-0.5f, 0.5f, offset);
}

float DrumSequencer::getStepSwing(int stepIndex) const
{
    if (stepIndex >= 0 && stepIndex < static_cast<int>(steps.size()))
        return steps[stepIndex].swingOffset;
    return 0.0f;
}

void DrumSequencer::setPatternLength(int beats) { patternLength = juce::jmax(1, beats); }
int DrumSequencer::getPatternLength() const { return patternLength; }

void DrumSequencer::randomizePattern(float density)
{
    density = juce::jlimit(0.0f, 1.0f, density);
    for (auto& step : steps)
    {
        step.notes.clear();
        step.active = (static_cast<float>(std::rand() / static_cast<double>(RAND_MAX))) < density;
        if (step.active)
        {
            // Pick a random drum type
            int drumIdx = std::rand() % NumDrumTypes;
            DrumStepNote note;
            note.noteNumber = DrumMap[drumIdx].defaultNote;
            note.velocity = 0.5f + static_cast<float>(std::rand() / static_cast<double>(RAND_MAX)) * 0.5f;
            note.probability = 0.7f + static_cast<float>(std::rand() / static_cast<double>(RAND_MAX)) * 0.3f;
            note.accent = (std::rand() % 100) < 15;
            step.notes.push_back(note);
        }
    }
}

void DrumSequencer::fillPattern(int noteNumber, float velocity)
{
    for (int i = 0; i < static_cast<int>(steps.size()); i += 2)
    {
        steps[i].active = true;
        steps[i].notes.clear();
        DrumStepNote note;
        note.noteNumber = noteNumber;
        note.velocity = velocity;
        note.probability = 1.0f;
        steps[i].notes.push_back(note);
    }
}

void DrumSequencer::fillPatternWithVariation(int noteNumber, float velocity, float variation)
{
    for (int i = 0; i < static_cast<int>(steps.size()); ++i)
    {
        float roll = static_cast<float>(std::rand() / static_cast<double>(RAND_MAX));
        if (roll < variation)
        {
            steps[i].active = true;
            steps[i].notes.clear();
            DrumStepNote note;
            note.noteNumber = noteNumber;
            note.velocity = velocity * (0.6f + 0.4f * static_cast<float>(std::rand() / static_cast<double>(RAND_MAX)));
            note.probability = 0.8f + 0.2f * static_cast<float>(std::rand() / static_cast<double>(RAND_MAX));
            note.accent = (std::rand() % 100) < 20;
            steps[i].notes.push_back(note);
        }
    }
}

void DrumSequencer::generateBeat(int genre)
{
    clearAllSteps();
    int numS = static_cast<int>(steps.size());

    switch (genre)
    {
        case 0: // Four on the floor
            for (int i = 0; i < numS; i += 4)
            {
                DrumStepNote kick;
                kick.noteNumber = DrumMap[0].defaultNote;
                kick.velocity = 0.9f;
                steps[i].notes.push_back(kick);
                steps[i].active = true;
            }
            for (int i = 4; i < numS; i += 8)
            {
                DrumStepNote snare;
                snare.noteNumber = DrumMap[1].defaultNote;
                snare.velocity = 0.85f;
                steps[i].notes.push_back(snare);
                steps[i].active = true;
            }
            for (int i = 2; i < numS; i += 4)
            {
                DrumStepNote hh;
                hh.noteNumber = DrumMap[2].defaultNote;
                hh.velocity = 0.5f;
                steps[i].notes.push_back(hh);
                steps[i].active = true;
            }
            break;

        case 1: // Rock
            for (int i = 0; i < numS; i += 4)
            {
                DrumStepNote kick;
                kick.noteNumber = DrumMap[0].defaultNote;
                kick.velocity = (i % 8 == 0) ? 0.9f : 0.7f;
                steps[i].notes.push_back(kick);
                steps[i].active = true;
            }
            for (int i = 4; i < numS; i += 8)
            {
                DrumStepNote snare;
                snare.noteNumber = DrumMap[1].defaultNote;
                snare.velocity = 0.85f;
                steps[i].notes.push_back(snare);
                steps[i].active = true;
            }
            for (int i = 0; i < numS; i += 2)
            {
                DrumStepNote hh;
                hh.noteNumber = DrumMap[2].defaultNote;
                hh.velocity = (i % 4 == 0) ? 0.6f : 0.4f;
                steps[i].notes.push_back(hh);
                steps[i].active = true;
            }
            break;

        case 2: // Breakbeat
            {
                static const int breakPattern[] = { 0, 1, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 1, 0, 0, 0, 0, 0 };
                for (int i = 0; i < numS && i < 16; ++i)
                {
                    DrumStepNote kick;
                    kick.noteNumber = DrumMap[0].defaultNote;
                    kick.velocity = 0.9f;
                    steps[i].notes.push_back(kick);
                    steps[i].active = true;

                    if (breakPattern[i])
                    {
                        DrumStepNote snare;
                        snare.noteNumber = DrumMap[1].defaultNote;
                        snare.velocity = 0.85f;
                        steps[i].notes.push_back(snare);
                    }
                }
                for (int i = 0; i < numS; i += 4)
                {
                    DrumStepNote hh;
                    hh.noteNumber = DrumMap[2].defaultNote;
                    hh.velocity = (i % 8 == 0) ? 0.6f : 0.4f;
                    steps[i].notes.push_back(hh);
                    steps[i].active = true;
                }
                // Ghost notes on offbeats
                for (int i = 3; i < numS; i += 8)
                {
                    DrumStepNote ghost;
                    ghost.noteNumber = DrumMap[8].defaultNote;
                    ghost.velocity = 0.3f;
                    ghost.probability = 0.5f;
                    steps[i].notes.push_back(ghost);
                }
            }
            break;

        case 3: // Half-time
            for (int i = 0; i < numS; i += 8)
            {
                DrumStepNote kick;
                kick.noteNumber = DrumMap[0].defaultNote;
                kick.velocity = 0.9f;
                steps[i].notes.push_back(kick);
                steps[i].active = true;
            }
            steps[4].notes.push_back({ DrumMap[0].defaultNote, 0.7f });
            steps[4].active = true;
            for (int i = 4; i < numS; i += 16)
            {
                DrumStepNote snare;
                snare.noteNumber = DrumMap[1].defaultNote;
                snare.velocity = 0.9f;
                steps[i].notes.push_back(snare);
                steps[i].active = true;
                steps[12].notes.push_back(snare);
                steps[12].active = true;
            }
            for (int i = 2; i < numS; i += 4)
            {
                DrumStepNote hh;
                hh.noteNumber = DrumMap[2].defaultNote;
                hh.velocity = 0.5f;
                steps[i].notes.push_back(hh);
                steps[i].active = true;
            }
            break;

        case 4: // Latin
            {
                static const int latinKicks[] = { 1, 0, 0, 1, 0, 1, 0, 1,
                                                  1, 0, 0, 1, 0, 1, 0, 0 };
                for (int i = 0; i < numS && i < 16; ++i)
                {
                    if (latinKicks[i])
                    {
                        DrumStepNote kick;
                        kick.noteNumber = DrumMap[0].defaultNote;
                        kick.velocity = 0.8f;
                        steps[i].notes.push_back(kick);
                        steps[i].active = true;
                    }
                }
                for (int i = 4; i < numS; i += 8)
                {
                    DrumStepNote snare;
                    snare.noteNumber = DrumMap[1].defaultNote;
                    snare.velocity = 0.7f;
                    steps[i].notes.push_back(snare);
                    steps[i].active = true;
                }
                for (int i = 1; i < numS; i += 2)
                {
                    DrumStepNote hh;
                    hh.noteNumber = DrumMap[3].defaultNote;
                    hh.velocity = 0.4f;
                    steps[i].notes.push_back(hh);
                    steps[i].active = true;
                }
            }
            break;

        case 5: // Garage / 2-step
            for (int i = 0; i < numS; i += 8)
            {
                DrumStepNote kick;
                kick.noteNumber = DrumMap[0].defaultNote;
                kick.velocity = 0.85f;
                steps[i].notes.push_back(kick);
                steps[i].active = true;

                DrumStepNote hh;
                hh.noteNumber = DrumMap[2].defaultNote;
                hh.velocity = 0.5f;
                hh.accent = true;
                steps[i + 6].notes.push_back(hh);
                steps[i + 6].active = true;
            }
            for (int i = 4; i < numS; i += 8)
            {
                DrumStepNote snare;
                snare.noteNumber = DrumMap[1].defaultNote;
                snare.velocity = 0.75f;
                steps[i].notes.push_back(snare);
                steps[i].active = true;
            }
            // Shuffle hats
            for (int i = 2; i < numS; i += 4)
            {
                DrumStepNote sh;
                sh.noteNumber = DrumMap[11].defaultNote;
                sh.velocity = 0.3f;
                steps[i + 1].notes.push_back(sh);
                steps[i + 1].active = true;
                if (i % 8 == 2)
                {
                    steps[i + 1].swingOffset = 0.15f;
                }
            }
            break;
    }
}

int DrumSequencer::getDefaultNoteForDrumType(int drumTypeIndex)
{
    return (drumTypeIndex >= 0 && drumTypeIndex < NumDrumTypes)
        ? DrumMap[drumTypeIndex].defaultNote : 36;
}

int DrumSequencer::getNumDrumTypes()
{
    return NumDrumTypes;
}

DrumMapEntry DrumSequencer::getDrumMapEntry(int index)
{
    if (index >= 0 && index < NumDrumTypes)
        return DrumMap[index];
    return { 0, "Unknown", 36 };
}

int DrumSequencer::noteToDrumTypeIndex(int noteNumber)
{
    for (int i = 0; i < NumDrumTypes; ++i)
    {
        if (DrumMap[i].defaultNote == noteNumber)
            return i;
    }
    return -1;
}

void DrumSequencer::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    reset();
}

double DrumSequencer::getStepDuration(double tempo) const
{
    double secondsPerBeat = 60.0 / tempo;
    return secondsPerBeat / static_cast<double>(resolution);
}

void DrumSequencer::processBlock(juce::MidiBuffer& midiBuffer,
                                  double positionInSeconds,
                                  double tempo,
                                  double sampleRate,
                                  bool transportPlaying)
{
    juce::ignoreUnused(sampleRate);

    if (!transportPlaying || steps.empty() || tempo <= 0.0)
        return;

    double stepDuration = getStepDuration(tempo);
    if (stepDuration <= 0.0) return;

    double totalPatternTime = stepDuration * steps.size();
    if (totalPatternTime <= 0.0) return;

    double posInPattern = std::fmod(positionInSeconds, totalPatternTime);
    if (posInPattern < 0.0) posInPattern += totalPatternTime;

    int currentStepIndex = static_cast<int>(posInPattern / stepDuration);
    currentStepIndex = juce::jlimit(0, static_cast<int>(steps.size()) - 1, currentStepIndex);

    double stepStartTime = currentStepIndex * stepDuration;

    for (int i = 0; i < static_cast<int>(steps.size()); ++i)
    {
        if (!steps[i].active)
            continue;

        double thisStepStart = i * stepDuration;

        // Global swing on even/odd steps
        if (i % 2 == 1 && swing > 0.0f)
            thisStepStart += swing * stepDuration * 0.5f;

        // Per-step swing offset
        thisStepStart += steps[i].swingOffset * stepDuration;

        if (thisStepStart < stepStartTime - (10.0 / sampleRate))
            continue;

        if (thisStepStart > posInPattern + (10.0 / sampleRate))
            continue;

        for (const auto& note : steps[i].notes)
        {
            if (note.probability < 1.0f)
            {
                float roll = static_cast<float>(std::rand() / static_cast<double>(RAND_MAX));
                if (roll > note.probability)
                    continue;
            }

            float vel = note.accent ? juce::jmin(1.0f, note.velocity * 1.3f) : note.velocity;

            auto noteOn = juce::MidiMessage::noteOn(1, note.noteNumber, vel);
            noteOn.setTimeStamp(thisStepStart);
            midiBuffer.addEvent(noteOn, static_cast<int>((thisStepStart - posInPattern) * sampleRate));

            if (onStepTriggered)
                onStepTriggered(i, note.noteNumber, vel);

            if (note.flam > 0)
            {
                auto flamMsg = juce::MidiMessage::noteOn(1, note.noteNumber, vel * 0.7f);
                double flamOffset = thisStepStart + note.flam * 0.01;
                midiBuffer.addEvent(flamMsg, static_cast<int>((flamOffset - posInPattern) * sampleRate));
            }

            double noteLength = stepDuration * 0.9;
            auto noteOff = juce::MidiMessage::noteOff(1, note.noteNumber);
            noteOff.setTimeStamp(thisStepStart + noteLength);
            midiBuffer.addEvent(noteOff, static_cast<int>((thisStepStart + noteLength - posInPattern) * sampleRate));
        }
    }
}

void DrumSequencer::reset()
{
    lastTriggeredStep = -1;
    currentStep = 0;
    lastStepTime = 0.0;
}

juce::ValueTree DrumSequencer::getState() const
{
    juce::ValueTree state("DrumSequencer");
    state.setProperty("NumSteps", static_cast<int>(steps.size()), nullptr);
    state.setProperty("Resolution", resolution, nullptr);
    state.setProperty("Swing", swing, nullptr);
    state.setProperty("PatternLength", patternLength, nullptr);

    juce::ValueTree stepsState("Steps");
    for (int i = 0; i < static_cast<int>(steps.size()); ++i)
    {
        juce::ValueTree stepState("Step");
        stepState.setProperty("Index", i, nullptr);
        stepState.setProperty("Active", steps[i].active, nullptr);
        stepState.setProperty("SwingOffset", steps[i].swingOffset, nullptr);

        juce::ValueTree notesState("Notes");
        for (const auto& note : steps[i].notes)
        {
            juce::ValueTree noteState("Note");
            noteState.setProperty("NoteNumber", note.noteNumber, nullptr);
            noteState.setProperty("Velocity", note.velocity, nullptr);
            noteState.setProperty("Probability", note.probability, nullptr);
            noteState.setProperty("Accent", note.accent, nullptr);
            noteState.setProperty("Flam", note.flam, nullptr);
            notesState.addChild(noteState, -1, nullptr);
        }
        stepState.addChild(notesState, -1, nullptr);
        stepsState.addChild(stepState, -1, nullptr);
    }
    state.addChild(stepsState, -1, nullptr);

    return state;
}

void DrumSequencer::setState(const juce::ValueTree& tree)
{
    if (!tree.hasType("DrumSequencer")) return;

    resolution = tree.getProperty("Resolution", resolution);
    swing = tree.getProperty("Swing", swing);
    patternLength = tree.getProperty("PatternLength", patternLength);

    int numSteps = tree.getProperty("NumSteps", DefaultSteps);
    setNumSteps(numSteps);

    auto stepsState = tree.getChildWithName("Steps");
    if (stepsState.isValid())
    {
        for (int c = 0; c < stepsState.getNumChildren(); ++c)
        {
            auto stepState = stepsState.getChild(c);
            int idx = stepState.getProperty("Index", -1);
            if (idx >= 0 && idx < static_cast<int>(steps.size()))
            {
                steps[idx].active = stepState.getProperty("Active", false);
                steps[idx].swingOffset = stepState.getProperty("SwingOffset", 0.0f);
                steps[idx].notes.clear();

                auto notesState = stepState.getChildWithName("Notes");
                if (notesState.isValid())
                {
                    for (int n = 0; n < notesState.getNumChildren(); ++n)
                    {
                        auto noteState = notesState.getChild(n);
                        DrumStepNote note;
                        note.noteNumber = noteState.getProperty("NoteNumber", 36);
                        note.velocity = noteState.getProperty("Velocity", 0.8f);
                        note.probability = noteState.getProperty("Probability", 1.0f);
                        note.accent = noteState.getProperty("Accent", false);
                        note.flam = noteState.getProperty("Flam", 0);
                        steps[idx].notes.push_back(note);
                    }
                }
            }
        }
    }
}

} // namespace harmonic_engine
