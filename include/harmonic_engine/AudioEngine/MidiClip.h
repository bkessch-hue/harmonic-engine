#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>

namespace harmonic_engine
{

struct MidiNote
{
    int noteNumber = 60;
    double startTime = 0.0;
    double duration = 0.25;
    float velocity = 0.8f;

    double getEndTime() const { return startTime + duration; }
    bool overlaps(double time) const { return time >= startTime && time < getEndTime(); }
    bool overlapsRange(double rangeStart, double rangeEnd) const
    {
        return startTime < rangeEnd && getEndTime() > rangeStart;
    }
};

class MidiClip
{
public:
    MidiClip(const juce::String& clipName, double timelineStart, double bpm = 120.0);
    ~MidiClip();

    juce::String getName() const;
    void setName(const juce::String& newName);

    double getTimelineStart() const;
    void setTimelineStart(double startSeconds);

    double getClipDuration() const;
    void setClipDuration(double durationSeconds);

    double getBpm() const;
    void setBpm(double bpm);

    bool isMuted() const;
    void setMuted(bool muted);

    juce::Colour getColour() const;
    void setColour(juce::Colour colour);

    void addNote(int noteNumber, double startTime, double duration, float velocity = 0.8f);
    void removeNote(int index);
    void clearNotes();
    int getNumNotes() const;
    MidiNote getNote(int index) const;
    void setNote(int index, const MidiNote& note);

    MidiNote getNoteAtTime(double time, int noteNumber) const;
    std::vector<MidiNote> getNotesInRange(double startTime, double endTime) const;

    void renderMidiBuffer(juce::MidiBuffer& midiBuffer, double clipStartTime,
                          int numSamples, double sampleRate) const;

    juce::MidiMessageSequence toMidiSequence() const;
    void fromMidiSequence(const juce::MidiMessageSequence& seq, double bpm);

    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& tree);

    static const juce::Identifier clipId;
    static const juce::Identifier nameProperty;
    static const juce::Identifier timelineStartProperty;
    static const juce::Identifier clipLengthProperty;
    static const juce::Identifier bpmProperty;
    static const juce::Identifier mutedProperty;
    static const juce::Identifier colourProperty;
    static const juce::Identifier notesId;

private:
    juce::ValueTree state;
    std::vector<MidiNote> notes;

    void syncNotesToState();
    void syncStateToNotes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiClip)
};

} // namespace harmonic_engine
