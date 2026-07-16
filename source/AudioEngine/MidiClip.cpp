#include "harmonic_engine/AudioEngine/MidiClip.h"

namespace harmonic_engine
{

const juce::Identifier MidiClip::clipId("MidiClip");
const juce::Identifier MidiClip::nameProperty("name");
const juce::Identifier MidiClip::timelineStartProperty("timelineStart");
const juce::Identifier MidiClip::clipLengthProperty("clipLength");
const juce::Identifier MidiClip::bpmProperty("bpm");
const juce::Identifier MidiClip::mutedProperty("muted");
const juce::Identifier MidiClip::colourProperty("colour");
const juce::Identifier MidiClip::notesId("Notes");

MidiClip::MidiClip(const juce::String& clipName, double timelineStart, double bpm)
    : state(clipId)
{
    state.setProperty(nameProperty, clipName, nullptr);
    state.setProperty(timelineStartProperty, timelineStart, nullptr);
    state.setProperty(clipLengthProperty, 4.0, nullptr);
    state.setProperty(bpmProperty, bpm, nullptr);
    state.setProperty(mutedProperty, false, nullptr);
    state.setProperty(colourProperty, static_cast<int>(0xff2ecc71), nullptr);
    state.addChild(juce::ValueTree(notesId), -1, nullptr);
}

MidiClip::~MidiClip()
{
}

juce::String MidiClip::getName() const
{
    return state.getProperty(nameProperty, "MIDI Clip");
}

void MidiClip::setName(const juce::String& newName)
{
    state.setProperty(nameProperty, newName, nullptr);
}

double MidiClip::getTimelineStart() const
{
    return static_cast<double>(state.getProperty(timelineStartProperty, 0.0));
}

void MidiClip::setTimelineStart(double startSeconds)
{
    state.setProperty(timelineStartProperty, juce::jmax(0.0, startSeconds), nullptr);
}

double MidiClip::getClipDuration() const
{
    return static_cast<double>(state.getProperty(clipLengthProperty, 4.0));
}

void MidiClip::setClipDuration(double durationSeconds)
{
    state.setProperty(clipLengthProperty, juce::jmax(0.01, durationSeconds), nullptr);
}

double MidiClip::getBpm() const
{
    return static_cast<double>(state.getProperty(bpmProperty, 120.0));
}

void MidiClip::setBpm(double bpm)
{
    state.setProperty(bpmProperty, juce::jmax(20.0, juce::jmin(300.0, bpm)), nullptr);
}

bool MidiClip::isMuted() const
{
    return static_cast<bool>(state.getProperty(mutedProperty, false));
}

void MidiClip::setMuted(bool muted)
{
    state.setProperty(mutedProperty, muted, nullptr);
}

juce::Colour MidiClip::getColour() const
{
    auto colourVar = state.getProperty(colourProperty, static_cast<juce::int32>(0xff2ecc71));
    auto colourInt = static_cast<juce::uint32>(static_cast<int>(colourVar));
    return juce::Colour(colourInt);
}

void MidiClip::setColour(juce::Colour colour)
{
    state.setProperty(colourProperty, static_cast<juce::int32>(colour.getARGB()), nullptr);
}

void MidiClip::addNote(int noteNumber, double startTime, double duration, float velocity)
{
    MidiNote note;
    note.noteNumber = juce::jlimit(0, 127, noteNumber);
    note.startTime = startTime;
    note.duration = juce::jmax(0.01, duration);
    note.velocity = juce::jlimit(0.0f, 1.0f, velocity);
    notes.push_back(note);
    syncNotesToState();
}

void MidiClip::removeNote(int index)
{
    if (index >= 0 && index < static_cast<int>(notes.size()))
    {
        notes.erase(notes.begin() + index);
        syncNotesToState();
    }
}

void MidiClip::clearNotes()
{
    notes.clear();
    syncNotesToState();
}

int MidiClip::getNumNotes() const
{
    return static_cast<int>(notes.size());
}

MidiNote MidiClip::getNote(int index) const
{
    if (index >= 0 && index < static_cast<int>(notes.size()))
        return notes[index];
    return {};
}

void MidiClip::setNote(int index, const MidiNote& note)
{
    if (index >= 0 && index < static_cast<int>(notes.size()))
    {
        notes[index] = note;
        syncNotesToState();
    }
}

MidiNote MidiClip::getNoteAtTime(double time, int noteNumber) const
{
    for (const auto& note : notes)
    {
        if (note.noteNumber == noteNumber && note.overlaps(time))
            return note;
    }
    return { -1, 0.0, 0.0, 0.0f };
}

std::vector<MidiNote> MidiClip::getNotesInRange(double startTime, double endTime) const
{
    std::vector<MidiNote> result;
    for (const auto& note : notes)
    {
        if (note.overlapsRange(startTime, endTime))
            result.push_back(note);
    }
    return result;
}

void MidiClip::renderMidiBuffer(juce::MidiBuffer& midiBuffer, double clipStartTime,
                                int numSamples, double sampleRate) const
{
    if (static_cast<bool>(state.getProperty(mutedProperty, false)))
        return;

    double clipDuration = static_cast<double>(numSamples) / sampleRate;
    double clipStart = static_cast<double>(state.getProperty(timelineStartProperty, 0.0));

    for (const auto& note : notes)
    {
        double noteAbsStart = clipStart + note.startTime;
        double noteAbsEnd = noteAbsStart + note.duration;

        if (noteAbsEnd < clipStartTime || noteAbsStart > clipStartTime + clipDuration)
            continue;

        int startSample = static_cast<int>((noteAbsStart - clipStartTime) * sampleRate);
        int endSample = static_cast<int>((noteAbsEnd - clipStartTime) * sampleRate);

        startSample = juce::jmax(0, startSample);
        endSample = juce::jmin(numSamples, endSample);

        if (startSample < numSamples)
        {
            juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, note.noteNumber, note.velocity);
            noteOn.setTimeStamp(static_cast<double>(startSample) / sampleRate);
            midiBuffer.addEvent(noteOn, startSample);
        }

        if (endSample < numSamples)
        {
            juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, note.noteNumber, 0.0f);
            noteOff.setTimeStamp(static_cast<double>(endSample) / sampleRate);
            midiBuffer.addEvent(noteOff, endSample);
        }
    }
}

juce::MidiMessageSequence MidiClip::toMidiSequence() const
{
    juce::MidiMessageSequence seq;
    double bpm = getBpm();

    for (const auto& note : notes)
    {
        double startSeconds = note.startTime;
        double endSeconds = note.getEndTime();

        juce::MidiMessage noteOn(0x90, note.noteNumber,
                                  static_cast<int>(note.velocity * 127.0f), 0.0);
        noteOn.setTimeStamp(startSeconds);
        seq.addEvent(noteOn);

        juce::MidiMessage noteOff(0x80, note.noteNumber, 0, 0.0);
        noteOff.setTimeStamp(endSeconds);
        seq.addEvent(noteOff);
    }

    seq.updateMatchedPairs();
    return seq;
}

void MidiClip::fromMidiSequence(const juce::MidiMessageSequence& seq, double bpm)
{
    notes.clear();
    setBpm(bpm);

    for (int i = 0; i < seq.getNumEvents(); ++i)
    {
        auto* event = seq.getEventPointer(i);
        if (event == nullptr) continue;

        const auto& msg = event->message;

        if (msg.isNoteOn())
        {
            double startTime = msg.getTimeStamp();
            int noteNum = msg.getNoteNumber();
            float vel = static_cast<float>(msg.getVelocity()) / 127.0f;

            double endTime = startTime + 0.25;
            for (int j = 0; j < seq.getNumEvents(); ++j)
            {
                auto* offEvent = seq.getEventPointer(j);
                if (offEvent != nullptr && offEvent->message.isNoteOff() &&
                    offEvent->message.getNoteNumber() == noteNum &&
                    offEvent->message.getTimeStamp() > startTime)
                {
                    endTime = offEvent->message.getTimeStamp();
                    break;
                }
            }

            addNote(noteNum, startTime, endTime - startTime, vel);
        }
    }
}

juce::ValueTree MidiClip::getState() const
{
    return state.createCopy();
}

void MidiClip::setState(const juce::ValueTree& tree)
{
    if (tree.hasType(clipId))
    {
        state = tree.createCopy();
        syncStateToNotes();
    }
}

void MidiClip::syncNotesToState()
{
    auto notesTree = state.getOrCreateChildWithName(notesId, nullptr);
    notesTree.removeAllChildren(nullptr);

    for (const auto& note : notes)
    {
        juce::ValueTree noteTree("Note");
        noteTree.setProperty("noteNumber", note.noteNumber, nullptr);
        noteTree.setProperty("startTime", note.startTime, nullptr);
        noteTree.setProperty("duration", note.duration, nullptr);
        noteTree.setProperty("velocity", note.velocity, nullptr);
        notesTree.addChild(noteTree, -1, nullptr);
    }
}

void MidiClip::syncStateToNotes()
{
    notes.clear();
    auto notesTree = state.getChildWithName(notesId);
    if (!notesTree.isValid()) return;

    for (int i = 0; i < notesTree.getNumChildren(); ++i)
    {
        auto noteTree = notesTree.getChild(i);
        MidiNote note;
        note.noteNumber = static_cast<int>(noteTree.getProperty("noteNumber", 60));
        note.startTime = static_cast<double>(noteTree.getProperty("startTime", 0.0));
        note.duration = static_cast<double>(noteTree.getProperty("duration", 0.25));
        note.velocity = static_cast<float>(noteTree.getProperty("velocity", 0.8));
        notes.push_back(note);
    }
}

} // namespace harmonic_engine
