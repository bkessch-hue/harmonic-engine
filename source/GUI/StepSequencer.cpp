#include "harmonic_engine/GUI/StepSequencer.h"
#include <cstdlib>
#include <algorithm>
#include <ctime>

namespace harmonic_engine
{

// ============================================================================
// Drum map:  MIDI note → name → colour
// ============================================================================
static constexpr int NoteKick    = 36;
static constexpr int NoteSnare   = 38;
static constexpr int NoteHihatCH = 42;
static constexpr int NoteHihatOH = 46;
static constexpr int NoteClap    = 39;
static constexpr int NoteTomHi   = 48;
static constexpr int NoteTomMid  = 45;
static constexpr int NoteTomLo   = 41;
static constexpr int NoteRim     = 37;
static constexpr int NoteCrash   = 49;
static constexpr int NoteRide    = 51;
static constexpr int NoteCowbell = 56;

StepSequencer::StepSequencer(Transport& t, DrumSequencer& drumSeq)
    : transport(t), drumSequencer(drumSeq)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    initDrumRows();
    createControls();

    const int numRows = static_cast<int>(drumRows.size());
    pattern.assign(numRows, std::vector<bool>(numSteps, false));
    accent.assign(numRows * numSteps, 0.0f);
    muteButtons.resize(numRows);

    for (int r = 0; r < numRows; ++r)
    {
        muteButtons[r] = std::make_unique<juce::TextButton>("M");
        muteButtons[r]->setClickingTogglesState(true);
        muteButtons[r]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333355));
        muteButtons[r]->setColour(juce::TextButton::buttonOnColourId, drumRows[r].colour.withAlpha(0.5f));
        muteButtons[r]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xff888888));
        muteButtons[r]->setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        muteButtons[r]->setToggleState(false, juce::dontSendNotification);
        addAndMakeVisible(*muteButtons[r]);
    }

    startTimerHz(30);
}

StepSequencer::~StepSequencer()
{
    stopTimer();
}

void StepSequencer::initDrumRows()
{
    drumRows = {
        { NoteKick,    "Kick",     juce::Colour(0xffe74c3c) },
        { NoteSnare,   "Snare",    juce::Colour(0xfff39c12) },
        { NoteHihatCH, "CH Hat",   juce::Colour(0xff9b59b6) },
        { NoteHihatOH, "OH Hat",   juce::Colour(0xff2ecc71) },
        { NoteClap,    "Clap",     juce::Colour(0xff1abc9c) },
        { NoteRim,     "Rim",      juce::Colour(0xffffd700) },
        { NoteTomHi,   "Tom Hi",   juce::Colour(0xff3498db) },
        { NoteTomMid,  "Tom Mid",  juce::Colour(0xff9b59b6) },
        { NoteTomLo,   "Tom Lo",   juce::Colour(0xffe91e63) },
        { NoteCrash,   "Crash",    juce::Colour(0xffff6b6b) },
        { NoteRide,    "Ride",     juce::Colour(0xffa0a0c0) },
        { NoteCowbell, "Cowbell",  juce::Colour(0xffc0c0c0) },
    };
}

void StepSequencer::createControls()
{
    auto makeButton = [this](const juce::String& text)
    {
        auto btn = std::make_unique<juce::TextButton>(text);
        btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
        btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc0c0d0));
        addAndMakeVisible(*btn);
        btn->addListener(this);
        return btn;
    };

    genreCombo = std::make_unique<juce::ComboBox>();
    genreCombo->addItem("Four on Floor", 1);
    genreCombo->addItem("Rock Beat",     2);
    genreCombo->addItem("Breakbeat",     3);
    genreCombo->addItem("Half Time",     4);
    genreCombo->addItem("Shuffle",       5);
    genreCombo->addItem("DnB",           6);
    genreCombo->addItem("House",         7);
    genreCombo->addItem("Trap",          8);
    genreCombo->addItem("Jazz Brush",    9);
    genreCombo->addItem("Funk",         10);
    genreCombo->addItem("Punk",         11);
    genreCombo->addItem("Reggae",       12);
    genreCombo->setSelectedId(1, juce::dontSendNotification);
    genreCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a4a));
    genreCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xffc0c0d0));
    genreCombo->addListener(this);
    addAndMakeVisible(*genreCombo);

    generateBtn = makeButton("Generate");
    fillBtn     = makeButton("Fill");
    mutateBtn   = makeButton("Mutate");
    randomBtn   = makeButton("Randomize");
    clearBtn    = makeButton("Clear");

    swingSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    swingSlider->setRange(0.0, 1.0, 0.01);
    swingSlider->setValue(0.0, juce::dontSendNotification);
    swingSlider->setColour(juce::Slider::trackColourId, juce::Colour(0xff4a90d9));
    swingSlider->setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1a2e));
    swingSlider->addListener(this);
    addAndMakeVisible(*swingSlider);

    swingLabel = std::make_unique<juce::Label>(juce::String(), "Swing");
    swingLabel->setColour(juce::Label::textColourId, juce::Colour(0xff8888aa));
    swingLabel->setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(*swingLabel);

    densitySlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    densitySlider->setRange(0.05, 1.0, 0.01);
    densitySlider->setValue(0.5, juce::dontSendNotification);
    densitySlider->setColour(juce::Slider::trackColourId, juce::Colour(0xff4a90d9));
    densitySlider->setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1a2e));
    densitySlider->addListener(this);
    addAndMakeVisible(*densitySlider);

    densityLabel = std::make_unique<juce::Label>(juce::String(), "Density");
    densityLabel->setColour(juce::Label::textColourId, juce::Colour(0xff8888aa));
    densityLabel->setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(*densityLabel);
}

// ============================================================================
// Pattern helper methods
// ============================================================================

void StepSequencer::placeNote(int row, int step, float vel)
{
    if (row >= 0 && row < static_cast<int>(pattern.size()) &&
        step >= 0 && step < static_cast<int>(pattern[0].size()))
    {
        pattern[row][step] = true;
        accent[row * numSteps + step] = vel;
    }
}

void StepSequencer::removeNote(int row, int step)
{
    if (row >= 0 && row < static_cast<int>(pattern.size()) &&
        step >= 0 && step < static_cast<int>(pattern[0].size()))
    {
        pattern[row][step] = false;
        accent[row * numSteps + step] = 0.0f;
    }
}

bool StepSequencer::hasNote(int row, int step) const
{
    if (row >= 0 && row < static_cast<int>(pattern.size()) &&
        step >= 0 && step < static_cast<int>(pattern[0].size()))
        return pattern[row][step];
    return false;
}

void StepSequencer::rebuildPattern()
{
    const int numRows = static_cast<int>(drumRows.size());
    pattern.assign(numRows, std::vector<bool>(numSteps, false));
    accent.assign(numRows * numSteps, 0.0f);

    if (currentClip == nullptr) return;

    for (int r = 0; r < numRows; ++r)
    {
        int noteNum = drumRows[r].noteNumber;
        double beatDuration = 60.0 / transport.getTempo();
        double stepDuration = beatDuration / static_cast<double>(stepsPerBeat);

        for (int n = 0; n < currentClip->getNumNotes(); ++n)
        {
            auto note = currentClip->getNote(n);
            if (note.noteNumber != noteNum) continue;

            double relativeTime = note.startTime - currentClip->getTimelineStart();
            int step = static_cast<int>((relativeTime / stepDuration) + 0.5);

            if (step >= 0 && step < numSteps)
            {
                pattern[r][step] = true;
                accent[r * numSteps + step] = note.velocity;
            }
        }
    }
}

void StepSequencer::syncClipToPattern()
{
    if (currentClip == nullptr) return;

    currentClip->clearNotes();

    const int numRows = static_cast<int>(drumRows.size());
    double beatDuration = 60.0 / transport.getTempo();
    double stepDuration = beatDuration / static_cast<double>(stepsPerBeat);
    double noteDuration = stepDuration * 0.85;

    for (int r = 0; r < numRows; ++r)
    {
        if (drumRows[r].muted) continue;

        int noteNum = drumRows[r].noteNumber;
        for (int s = 0; s < numSteps; ++s)
        {
            if (pattern[r][s])
            {
                float vel = accent[r * numSteps + s];
                if (vel <= 0.0f) vel = 0.8f;

                double noteTime = currentClip->getTimelineStart() + s * stepDuration;
                currentClip->addNote(noteNum, noteTime, noteDuration, vel);
            }
        }
    }

    syncToDrumSequencer();

    if (onPatternChanged) onPatternChanged();
}

void StepSequencer::syncToDrumSequencer()
{
    const int numRows = static_cast<int>(drumRows.size());

    drumSequencer.setNumSteps(numSteps);
    drumSequencer.setResolution(stepsPerBeat);
    drumSequencer.setSwing(static_cast<float>(swingSlider->getValue()));
    drumSequencer.clearAllSteps();

    for (int s = 0; s < numSteps; ++s)
    {
        bool anyActive = false;
        for (int r = 0; r < numRows; ++r)
        {
            if (pattern[r][s] && !drumRows[r].muted)
            {
                DrumStepNote note;
                note.noteNumber = drumRows[r].noteNumber;
                float vel = accent[r * numSteps + s];
                note.velocity = (vel <= 0.0f) ? 0.8f : vel;
                note.probability = 1.0f;
                note.accent = (vel >= 0.9f);
                drumSequencer.addNoteToStep(s, note);
                anyActive = true;
            }
        }
        drumSequencer.setStepActive(s, anyActive);
    }
}

// ============================================================================
// Programmatic Rhythm Generation
// ============================================================================

void StepSequencer::generatePattern(Genre genre)
{
    clearPattern();

    const int numRows = static_cast<int>(drumRows.size());
    const float density = static_cast<float>(densitySlider->getValue());

    auto roll = []() -> bool { return (std::rand() % 100) < 50; };
    auto chance = [](int pct) -> bool { return (std::rand() % 100) < pct; };

    // Helper: place a row across a vector of step indices
    auto place = [&](int row, const std::vector<int>& steps, float vel = 0.8f)
    {
        for (int s : steps)
            if (s >= 0 && s < numSteps) placeNote(row, s, vel);
    };

    // Helper: ghost note (lower velocity)
    auto ghost = [&](int row, int step)
    {
        if (step >= 0 && step < numSteps && chance(static_cast<int>(density * 45)))
            placeNote(row, step, 0.4f + density * 0.2f);
    };

    switch (genre)
    {
        case Genre::FourOnFloor:
        {
            // Kick on every beat
            for (int s = 0; s < numSteps; s += stepsPerBeat)
                placeNote(0, s, 0.9f);
            // Snare on 2 & 4
            int beat2 = stepsPerBeat * 1;
            int beat4 = stepsPerBeat * 3;
            if (beat2 < numSteps) placeNote(1, beat2, 0.85f);
            if (beat4 < numSteps) placeNote(1, beat4, 0.85f);
            // Closed hat 8th notes
            for (int s = 0; s < numSteps; s += std::max(1, stepsPerBeat / 2))
                placeNote(2, s, 0.5f);
            // Open hat on off-beats occasionally
            for (int s = 1; s < numSteps; s += stepsPerBeat / 2)
                if (chance(30)) placeNote(3, s, 0.6f);
            break;
        }

        case Genre::RockBeat:
        {
            // Kick: 1, and-of-2, 3
            place(0, { 0, 2, stepsPerBeat * 2 });
            // Kick ghost
            if (chance(40)) ghost(0, 1);
            if (chance(40)) ghost(0, stepsPerBeat * 2 + 1);
            // Snare on 2 & 4
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.85f);
            // Hat: 8th notes
            for (int s = 0; s < numSteps; s += std::max(1, stepsPerBeat / 2))
                placeNote(2, s, s % stepsPerBeat == 0 ? 0.7f : 0.5f);
            // Ride pattern
            for (int s = 0; s < numSteps; s += std::max(1, stepsPerBeat / 4))
                if (chance(25)) place(10, { s }, 0.4f);
            break;
        }

        case Genre::Breakbeat:
        {
            // Classic break: kick on 1, 3, 5, 13 (swung feel)
            place(0, { 0, stepsPerBeat * 1, stepsPerBeat * 2, stepsPerBeat * 3 + 1 }, 0.9f);
            // Ghost kick
            ghost(0, 3); ghost(0, stepsPerBeat * 2 - 1);
            // Snare on 2 & 4
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.85f);
            // Hat: busy 16th pattern
            for (int s = 0; s < numSteps; s++)
                if (chance(60)) placeNote(2, s, 0.35f);
            for (int s = 0; s < numSteps; s += 2)
                placeNote(2, s, 0.55f);
            // Open hat accents
            for (int s = 1; s < numSteps; s += 4)
                if (chance(50)) placeNote(3, s, 0.5f);
            // Rimshots on backbeats
            for (int s = 0; s < numSteps; s += 2)
                if (chance(30)) place(5, { s }, 0.5f);
            break;
        }

        case Genre::HalfTime:
        {
            // Kick: 1, 3, and-of-3
            place(0, { 0, stepsPerBeat * 2, stepsPerBeat * 2 + stepsPerBeat / 2 }, 0.9f);
            // Snare on 3
            place(1, { stepsPerBeat * 2 }, 0.9f);
            if (chance(40)) place(1, { stepsPerBeat * 2 + stepsPerBeat / 2 }, 0.6f);
            // Hat: slow
            for (int s = 0; s < numSteps; s += stepsPerBeat)
                placeNote(2, s, 0.6f);
            // Tom fills
            for (int s = stepsPerBeat * 3; s < numSteps; ++s)
                if (chance(20))
                {
                    int tomRow = 6 + (std::rand() % 3);
                    placeNote(tomRow, s, 0.7f);
                }
            break;
        }

        case Genre::Shuffle:
        {
            // Swing will be set by the slider
            // Kick: 1, and-of-2, 3
            place(0, { 0, 2, stepsPerBeat * 2, stepsPerBeat * 2 + 1 }, 0.9f);
            // Snare on 2 & 4
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.85f);
            // Hat: shuffle pattern (triplet feel)
            int hatStep = std::max(1, stepsPerBeat / 4);
            for (int s = 0; s < numSteps; s += hatStep)
                if (chance(70)) placeNote(2, s, (s % stepsPerBeat == 0) ? 0.7f : 0.4f);
            break;
        }

        case Genre::DnB:
        {
            // Kick on 1, 3, 5, 7 (every 2 beats)
            for (int s = 0; s < numSteps; s += stepsPerBeat * 2)
                placeNote(0, s, 0.9f);
            // Kick ghost notes
            for (int s = 0; s < numSteps; ++s)
                if (chance(15)) ghost(0, s);
            // Snare on 2 & 4 (with ghost notes)
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.85f);
            for (int s = stepsPerBeat * 1; s < stepsPerBeat * 2; s++)
                if (chance(20)) ghost(1, s);
            // Hat: rapid 16th/32nd pattern
            for (int s = 0; s < numSteps; s++)
                placeNote(2, s, chance(80) ? 0.3f : (chance(50) ? 0.2f : 0.0f));
            for (int s = 0; s < numSteps; s += 2)
                placeNote(2, s, 0.5f);
            // Ride cymbal
            for (int s = 0; s < numSteps; s += stepsPerBeat)
                if (chance(40)) placeNote(10, s, 0.3f);
            // Rimshots
            for (int s = 0; s < numSteps; s += 4)
                if (chance(40)) place(5, { s }, 0.5f);
            break;
        }

        case Genre::House:
        {
            // Four on floor kick
            for (int s = 0; s < numSteps; s += stepsPerBeat)
                placeNote(0, s, 0.9f);
            // Kick ghost on offbeats
            for (int s = 2; s < numSteps; s += stepsPerBeat)
                if (chance(30)) ghost(0, s);
            // Clap/snare on 2 & 4
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.8f);
            place(4, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.7f);
            // Hat: 8th note
            for (int s = 0; s < numSteps; s += std::max(1, stepsPerBeat / 2))
                placeNote(2, s, 0.5f);
            // Open hat: every 2nd beat off
            for (int s = 1; s < numSteps; s += stepsPerBeat)
                placeNote(3, s, 0.5f);
            // Tom fills at end of phrase
            for (int s = numSteps - 4; s < numSteps; ++s)
                if (chance(35))
                {
                    int tomRow = 6 + (std::rand() % 3);
                    placeNote(tomRow, s, 0.65f);
                }
            break;
        }

        case Genre::Trap:
        {
            // Kick: triplets with heavy 808 feel
            place(0, { 0, 2, 4, 6, 8, 10, 11, 13, 14 }, 0.9f);
            // Snare/clap on 3 & 7
            place(1, { 4, 12 }, 0.85f);
            place(4, { 4, 12 }, 0.8f);
            // Hat: rapid 16th rolls
            for (int s = 0; s < numSteps; s++)
                placeNote(2, s, chance(70) ? 0.25f : 0.0f);
            // Open hat
            for (int s = 2; s < numSteps; s += 8)
                placeNote(3, s, 0.5f);
            // Cowbell for flavor
            for (int s = 0; s < numSteps; s += 6)
                if (chance(50)) place(11, { s }, 0.5f);
            break;
        }

        case Genre::JazzBrush:
        {
            // Brush kick: softer, less frequent
            for (int s = 0; s < numSteps; s += stepsPerBeat * 2)
                placeNote(0, s, 0.5f);
            if (chance(40)) place(0, { stepsPerBeat * 2 + 1 }, 0.35f);
            // Brush snare: swish pattern
            for (int s = 0; s < numSteps; s += 2)
                placeNote(1, s, 0.3f + density * 0.2f);
            // Ride cymbal: swing pattern
            for (int s = 0; s < numSteps; s += 3)
                if (chance(60)) placeNote(10, s, 0.3f);
            // Hihat: very sparse
            for (int s = 0; s < numSteps; s += stepsPerBeat * 3)
                placeNote(2, s, 0.2f);
            break;
        }

        case Genre::Funk:
        {
            // Kick: syncopated
            place(0, { 0, 3, stepsPerBeat * 1 + 1, stepsPerBeat * 2, stepsPerBeat * 3 - 1 }, 0.85f);
            ghost(0, 1); ghost(0, stepsPerBeat * 2 - 2);
            // Snare: 2 & 4 with ghost notes
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.8f);
            for (int s = stepsPerBeat * 1 + 1; s < stepsPerBeat * 2; s++)
                if (chance(25)) ghost(1, s);
            // Hat: 16th note with ghost notes
            for (int s = 0; s < numSteps; s++)
                if (s % 2 == 0) placeNote(2, s, 0.6f);
                else if (chance(40)) placeNote(2, s, 0.25f);
            // Rimshots on 2 & 4
            place(5, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.7f);
            // Cowbell on 1 & 3
            place(11, { 0, stepsPerBeat * 2 }, 0.5f);
            break;
        }

        case Genre::Punk:
        {
            // Kick: busy
            for (int s = 0; s < numSteps; s++)
                if (chance(50 + static_cast<int>(density * 30)))
                    placeNote(0, s, 0.85f);
            // Snare: 2 & 4
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.9f);
            // Crash on 1
            place(9, { 0 }, 0.8f);
            // Hat: fast
            for (int s = 0; s < numSteps; s++)
                if (chance(70)) placeNote(2, s, 0.45f);
            // Toms at end
            for (int s = numSteps - 8; s < numSteps; s++)
                if (chance(30))
                {
                    int tomRow = 6 + (std::rand() % 3);
                    placeNote(tomRow, s, 0.7f);
                }
            break;
        }

        case Genre::Reggae:
        {
            // Kick on 1 and 3
            place(0, { 0, stepsPerBeat * 2 }, 0.85f);
            ghost(0, stepsPerBeat * 2 - 2);
            // Snare on 2 & 4
            place(1, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.7f);
            // Hat: off-beat emphasis (one drop)
            for (int s = 2; s < numSteps; s += stepsPerBeat)
                placeNote(2, s, 0.5f);
            for (int s = 0; s < numSteps; s++)
                if (chance(20)) placeNote(2, s, 0.2f);
            // Rimshot on 2 & 4
            place(5, { stepsPerBeat * 1, stepsPerBeat * 3 }, 0.6f);
            // Cowbell
            for (int s = 0; s < numSteps; s += stepsPerBeat * 2)
                place(11, { s }, 0.4f);
            break;
        }

        default:
            break;
    }

    syncClipToPattern();
    repaint();
}

void StepSequencer::generateFill()
{
    const int numRows = static_cast<int>(drumRows.size());

    for (int s = numSteps - 8; s < numSteps; ++s)
    {
        // Snare roll
        if (s % 2 == 0)
            placeNote(1, s, 0.5f + (static_cast<float>(s - (numSteps - 8)) / 8.0f) * 0.4f);

        // Tom fill
        if ((std::rand() % 100) < 60)
        {
            int tomRow = 6 + (std::rand() % 3);
            placeNote(tomRow, s, 0.6f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.3f);
        }

        // Crash on last beat
        if (s == numSteps - 2 || s == numSteps - 1)
            placeNote(9, s, 0.7f);
    }

    syncClipToPattern();
    repaint();
}

void StepSequencer::generateBrushPattern(int density)
{
    const int numRows = static_cast<int>(drumRows.size());

    for (int r = 0; r < numRows; ++r)
    {
        for (int s = 0; s < numSteps; ++s)
        {
            if ((std::rand() % 100) < density)
            {
                float vel = 0.3f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.5f;
                placeNote(r, s, vel);
            }
        }
    }

    syncClipToPattern();
    repaint();
}

void StepSequencer::mutatePattern(float amount)
{
    const int numRows = static_cast<int>(drumRows.size());
    const int mutateChance = static_cast<int>(amount * 100);

    for (int r = 0; r < numRows; ++r)
    {
        for (int s = 0; s < numSteps; ++s)
        {
            if ((std::rand() % 100) < mutateChance)
            {
                bool wasActive = pattern[r][s];
                if (wasActive)
                {
                    if ((std::rand() % 100) < 50)
                        removeNote(r, s);
                    else
                    {
                        float newVel = 0.4f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.5f;
                        placeNote(r, s, newVel);
                    }
                }
                else
                {
                    placeNote(r, s, 0.5f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.4f);
                }
            }
        }

        // Swap between adjacent rows occasionally
        if (r > 0 && (std::rand() % 100) < mutateChance / 2)
        {
            for (int s = 0; s < numSteps; ++s)
                std::swap(pattern[r][s], pattern[r - 1][s]);
        }
    }

    syncClipToPattern();
    repaint();
}

void StepSequencer::randomizePattern()
{
    const int numRows = static_cast<int>(drumRows.size());
    const float density = static_cast<float>(densitySlider->getValue());

    for (int r = 0; r < numRows; ++r)
    {
        for (int s = 0; s < numSteps; ++s)
        {
            int chancePct = static_cast<int>(density * 100);
            if ((std::rand() % 100) < chancePct)
                placeNote(r, s, 0.4f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.5f);
        }
    }

    syncClipToPattern();
    repaint();
}

void StepSequencer::clearPattern()
{
    const int numRows = static_cast<int>(drumRows.size());
    for (int r = 0; r < numRows; ++r)
        for (int s = 0; s < numSteps; ++s)
            removeNote(r, s);

    syncClipToPattern();
    repaint();
}

// ============================================================================
// MidiClip / config
// ============================================================================

void StepSequencer::setMidiClip(MidiClip* clip)
{
    currentClip = clip;
    rebuildPattern();
    repaint();
}

void StepSequencer::setNumSteps(int steps)
{
    numSteps = juce::jlimit(1, 64, steps);
    rebuildPattern();
    repaint();
}

void StepSequencer::setStepsPerBeat(int spb)
{
    stepsPerBeat = juce::jmax(1, spb);
    rebuildPattern();
    repaint();
}

// ============================================================================
// Paint
// ============================================================================

void StepSequencer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colour(0xff0d1117));

    // Control bar at top
    auto controlBar = bounds.removeFromTop(36);
    g.setColour(juce::Colour(0xff121a2e));
    g.fillRect(controlBar);

    auto gridArea = bounds;

    // Row labels
    auto labelBounds = gridArea.removeFromLeft(labelWidth);
    g.setColour(juce::Colour(0xff121a2e));
    g.fillRect(labelBounds);

    int numRows = static_cast<int>(drumRows.size());

    // Draw rows
    for (int r = 0; r < numRows; ++r)
    {
        int rowY = r * rowHeight;
        if (rowY > bounds.getHeight()) break;

        // Label background
        g.setColour(juce::Colour(0xff121a2e));
        g.fillRect(labelBounds.getX(), rowY, labelWidth, rowHeight);

        // Row name
        g.setColour(drumRows[r].muted ? juce::Colour(0xff555555) : drumRows[r].colour);
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
        g.drawText(drumRows[r].name,
                   labelBounds.getX() + 4, rowY + 2, labelWidth - 8, rowHeight - 4,
                   juce::Justification::centredLeft);

        // Horizontal line
        g.setColour(juce::Colour(0xff333355));
        g.drawHorizontalLine(rowY, 0.0f, static_cast<float>(getWidth()));
    }

    // Draw grid
    auto gridBounds = gridArea;
    for (int s = 0; s < numSteps; ++s)
    {
        int stepX = gridBounds.getX() + s * stepWidth;
        if (stepX > getWidth()) break;

        bool isBeatStart = (s % stepsPerBeat == 0);
        bool isBarStart = (s % (stepsPerBeat * 4) == 0);

        for (int r = 0; r < numRows; ++r)
        {
            int rowY = r * rowHeight;
            if (rowY > gridBounds.getHeight()) break;

            auto cellBounds = juce::Rectangle<int>(stepX, rowY, stepWidth, rowHeight).reduced(1);

            juce::Colour bgColour(0xff1a1a2e);
            if (isBarStart) bgColour = bgColour.brighter(0.15f);
            else if (isBeatStart) bgColour = bgColour.brighter(0.07f);

            g.setColour(bgColour);
            g.fillRect(cellBounds);

            if (pattern[r][s])
            {
                float vel = accent[r * numSteps + s];
                if (vel <= 0.0f) vel = 0.8f;
                float alpha = 0.3f + vel * 0.6f;

                auto colour = drumRows[r].muted
                    ? juce::Colour(0xff444444)
                    : drumRows[r].colour.withAlpha(alpha);

                g.setColour(colour);
                g.fillRoundedRectangle(cellBounds.toFloat(), 3.0f);

                g.setColour(colour.brighter(0.3f));
                g.drawRoundedRectangle(cellBounds.toFloat(), 3.0f, 1.0f);
            }
            else
            {
                g.setColour(juce::Colour(0xff333355));
                g.drawRoundedRectangle(cellBounds.toFloat(), 2.0f, 0.5f);
            }
        }

        // Bar/beat separators
        if (isBarStart && s > 0)
        {
            g.setColour(juce::Colour(0xff555577));
            g.drawVerticalLine(gridBounds.getX() + s * stepWidth - 1,
                               0.0f, static_cast<float>(gridBounds.getHeight()));
        }
        else if (isBeatStart && s > 0)
        {
            g.setColour(juce::Colour(0xff444466));
            g.drawVerticalLine(gridBounds.getX() + s * stepWidth - 1,
                               0.0f, static_cast<float>(gridBounds.getHeight()));
        }
    }

    // Playhead
    if (currentPlayStep >= 0 && currentPlayStep < numSteps)
    {
        int stepX = gridBounds.getX() + currentPlayStep * stepWidth;
        g.setColour(juce::Colour(0x40ffffff));
        g.fillRect(stepX, 0, stepWidth, numRows * rowHeight);
    }

    // Bottom label
    g.setColour(juce::Colour(0xff555577));
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    g.drawText("Step: " + juce::String(currentPlayStep + 1) + " / " + juce::String(numSteps),
               getWidth() - 120, bounds.getHeight() - 16, 120, 14,
               juce::Justification::right);
}

// ============================================================================
// Resize
// ============================================================================

void StepSequencer::resized()
{
    auto bounds = getLocalBounds();

    // Control bar at top
    auto controlBar = bounds.removeFromTop(36);

    controlBar.removeFromLeft(Tokens::kSpace4);
    auto genreArea = controlBar.removeFromLeft(130);
    genreCombo->setBounds(genreArea.reduced(2));

    controlBar.removeFromLeft(Tokens::kSpace2);
    generateBtn->setBounds(controlBar.removeFromLeft(70).reduced(2));
    fillBtn->setBounds(controlBar.removeFromLeft(55).reduced(2));
    mutateBtn->setBounds(controlBar.removeFromLeft(60).reduced(2));
    randomBtn->setBounds(controlBar.removeFromLeft(80).reduced(2));
    clearBtn->setBounds(controlBar.removeFromLeft(55).reduced(2));

    controlBar.removeFromLeft(Tokens::kSpace4);
    swingLabel->setBounds(controlBar.removeFromLeft(40).reduced(2));
    swingSlider->setBounds(controlBar.removeFromLeft(70).reduced(2));

    densityLabel->setBounds(controlBar.removeFromLeft(45).reduced(2));
    densitySlider->setBounds(controlBar.removeFromLeft(70).reduced(2));

    // Grid area
    auto gridBounds = bounds;

    // Mute buttons on left of labels
    rowHeight = juce::jmin(28, (gridBounds.getHeight() - 16) / static_cast<int>(drumRows.size()));
    rowHeight = juce::jmax(16, rowHeight);

    for (int r = 0; r < static_cast<int>(drumRows.size()); ++r)
    {
        int rowY = r * rowHeight;
        muteButtons[r]->setBounds(0, rowY, 20, rowHeight);
    }

    // Recalculate step width to fit
    int availableWidth = gridBounds.getWidth() - labelWidth - 20;
    stepWidth = juce::jmin(36, availableWidth / numSteps);
    stepWidth = juce::jmax(10, stepWidth);
}

// ============================================================================
// Mouse interaction
// ============================================================================

void StepSequencer::mouseDown(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds();
    auto controlBar = bounds.removeFromTop(36);
    auto gridArea = bounds;
    auto labelBounds = gridArea.removeFromLeft(labelWidth);
    auto gridBounds = gridArea;

    int mx = event.getPosition().x;
    int my = event.getPosition().y;

    // Check if in mute button area (first 20px)
    if (mx < 20)
    {
        int row = my / rowHeight;
        if (row >= 0 && row < static_cast<int>(drumRows.size()))
        {
            drumRows[row].muted = !drumRows[row].muted;
            muteButtons[row]->setToggleState(drumRows[row].muted, juce::dontSendNotification);
            repaint();
            syncClipToPattern();
        }
        return;
    }

    int gridX = mx - gridBounds.getX();
    int gridY = my;

    int col = gridX / stepWidth;
    int row = gridY / rowHeight;

    if (event.mods.isRightButtonDown())
    {
        // Right-click: remove
        if (col >= 0 && col < numSteps && row >= 0 && row < static_cast<int>(drumRows.size()))
        {
            removeNote(row, col);
            syncClipToPattern();
            repaint();
        }
        return;
    }

    if (col >= 0 && col < numSteps && row >= 0 && row < static_cast<int>(drumRows.size()))
    {
        if (event.mods.isShiftDown())
        {
            // Shift-click: set accent
            placeNote(row, col, 1.0f);
        }
        else if (pattern[row][col])
        {
            removeNote(row, col);
        }
        else
        {
            float vel = 0.6f + static_cast<float>(densitySlider->getValue()) * 0.3f;
            placeNote(row, col, vel);
        }

        syncClipToPattern();
        repaint();
    }
}

// ============================================================================
// Timer (playhead)
// ============================================================================

void StepSequencer::timerCallback()
{
    if (transport.isPlaying())
    {
        double pos = transport.getPositionInSeconds();
        double tempo = transport.getTempo();
        double beatDuration = 60.0 / tempo;
        double stepDuration = beatDuration / static_cast<double>(stepsPerBeat);

        double clipStart = 0.0;
        if (currentClip != nullptr)
            clipStart = currentClip->getTimelineStart();

        double relativePos = pos - clipStart;
        if (relativePos < 0.0) relativePos = 0.0;

        int step = static_cast<int>(relativePos / stepDuration) % numSteps;
        if (step != currentPlayStep)
        {
            currentPlayStep = step;
            repaint();
        }
    }
    else
    {
        if (currentPlayStep != -1)
        {
            currentPlayStep = -1;
            repaint();
        }
    }
}

// ============================================================================
// Button callbacks
// ============================================================================

void StepSequencer::buttonClicked(juce::Button* button)
{
    if (button == generateBtn.get())
    {
        int id = genreCombo->getSelectedId();
        if (id >= 1 && id <= static_cast<int>(Genre::NumberOfGenres))
            generatePattern(static_cast<Genre>(id - 1));
    }
    else if (button == fillBtn.get())
    {
        generateFill();
    }
    else if (button == mutateBtn.get())
    {
        mutatePattern(0.15f);
    }
    else if (button == randomBtn.get())
    {
        randomizePattern();
    }
    else if (button == clearBtn.get())
    {
        clearPattern();
    }
    else
    {
        // Mute button
        for (int r = 0; r < static_cast<int>(muteButtons.size()); ++r)
        {
            if (button == muteButtons[r].get())
            {
                drumRows[r].muted = button->getToggleState();
                syncClipToPattern();
                repaint();
                break;
            }
        }
    }
}

void StepSequencer::comboBoxChanged(juce::ComboBox* combo)
{
    if (combo == genreCombo.get())
    {
        // Just update - generation happens on button press
    }
}

void StepSequencer::sliderValueChanged(juce::Slider* slider)
{
    if (slider == swingSlider.get())
    {
        drumSequencer.setSwing(static_cast<float>(swingSlider->getValue()));
    }
}

} // namespace harmonic_engine