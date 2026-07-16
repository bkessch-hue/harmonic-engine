#include "harmonic_engine/GUI/PianoRoll.h"

namespace harmonic_engine
{

PianoRoll::PianoRoll(Transport& t)
    : transport(t)
{
    setOpaque(true);
    startTimerHz(30);
}

PianoRoll::~PianoRoll()
{
    stopTimer();
}

// ──────────────────────────────────────────────────────────────
// Coordinate helpers
// ──────────────────────────────────────────────────────────────

int PianoRoll::getGridWidth() const
{
    return getWidth() - keyboardWidth;
}

int PianoRoll::getGridHeight() const
{
    return (highestNote - lowestNote) * noteHeight;
}

int PianoRoll::yToNoteNumber(int y) const
{
    int noteFromTop = (highestNote - 1) - static_cast<int>((y + scrollY) / noteHeight);
    return juce::jlimit(lowestNote, highestNote - 1, noteFromTop);
}

int PianoRoll::noteNumberToY(int noteNum) const
{
    int noteFromTop = (highestNote - 1) - noteNum;
    return static_cast<int>(noteFromTop * noteHeight - scrollY);
}

double PianoRoll::xToTime(int x) const
{
    return static_cast<double>(x + scrollX) / pixelsPerSecond;
}

int PianoRoll::timeToX(double time) const
{
    return static_cast<int>(time * pixelsPerSecond - scrollX);
}

double PianoRoll::snapTime(double time) const
{
    if (!snapEnabled || snapDivision <= 0.0)
        return time;

    double tempo = transport.getTempo();
    double beatDuration = 60.0 / tempo;
    double snapSeconds = snapDivision * beatDuration;
    return std::round(time / snapSeconds) * snapSeconds;
}

bool PianoRoll::isBlackKey(int noteNum) const
{
    int pitchClass = noteNum % 12;
    return pitchClass == 1 || pitchClass == 3 || pitchClass == 5 ||
           pitchClass == 6 || pitchClass == 8 || pitchClass == 10;
}

juce::String PianoRoll::getNoteName(int noteNum) const
{
    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F",
                                        "F#", "G", "G#", "A", "A#", "B" };
    int octave = (noteNum / 12) - 1;
    int pitchClass = noteNum % 12;
    return juce::String(noteNames[pitchClass]) + juce::String(octave);
}

juce::Colour PianoRoll::getNoteColour(int noteNum) const
{
    float hue = static_cast<float>(noteNum % 12) / 12.0f;
    return juce::Colour::fromHSV(hue, 0.7f, 0.6f, 1.0f);
}

int PianoRoll::getNoteAtPosition(int x, int y) const
{
    if (currentClip == nullptr)
        return -1;

    for (int i = currentClip->getNumNotes() - 1; i >= 0; --i)
    {
        auto note = currentClip->getNote(i);
        int noteY = noteNumberToY(note.noteNumber);
        int noteX = timeToX(note.startTime + currentClip->getTimelineStart()) + keyboardWidth;
        int noteW = static_cast<int>(note.duration * pixelsPerSecond);

        auto noteBounds = juce::Rectangle<int>(noteX, noteY, noteW, noteHeight);
        if (noteBounds.contains(x, y))
            return i;
    }
    return -1;
}

bool PianoRoll::isOnResizeEdge(int x, int /*y*/, int noteIndex) const
{
    if (currentClip == nullptr || noteIndex < 0)
        return false;

    auto note = currentClip->getNote(noteIndex);
    int noteX = timeToX(note.startTime + currentClip->getTimelineStart()) + keyboardWidth;
    int noteW = static_cast<int>(note.duration * pixelsPerSecond);
    int edgeThreshold = juce::jmin(6, noteW / 3);

    return x >= (noteX + noteW - edgeThreshold);
}

// ──────────────────────────────────────────────────────────────
// Drawing
// ──────────────────────────────────────────────────────────────

void PianoRoll::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colour(0xff0d1117));

    auto keyboardBounds = bounds.removeFromLeft(keyboardWidth);
    drawPianoKeyboard(g, keyboardBounds);
    drawGrid(g, bounds);
    drawNotes(g, bounds);
    drawPlayhead(g, bounds);
}

void PianoRoll::resized()
{
}

void PianoRoll::drawPianoKeyboard(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(juce::Colour(0xff16213e));
    g.fillRect(bounds);

    for (int noteNum = lowestNote; noteNum < highestNote; ++noteNum)
    {
        int y = noteNumberToY(noteNum);
        bool black = isBlackKey(noteNum);
        bool hovered = (noteNum == hoveredKeyNote);

        juce::Colour keyColour = black
            ? juce::Colour(0xff1a1a2e)
            : juce::Colour(0xff3a3a5e);

        if (hovered)
            keyColour = keyColour.brighter(0.25f);

        g.setColour(keyColour);
        g.fillRect(bounds.getX(), y, bounds.getWidth(), noteHeight);

        g.setColour(juce::Colour(0xff0d1117));
        g.drawHorizontalLine(y, static_cast<float>(bounds.getX()),
                             static_cast<float>(bounds.getRight()));

        bool showC = (noteNum % 12 == 0);
        bool showOther = !black && !showC;

        if (showC || showOther)
        {
            g.setColour(juce::Colour(0xff888888));
            g.setFont(juce::Font(juce::FontOptions(9.0f)));
            g.drawText(getNoteName(noteNum),
                       bounds.getX() + 4, y + 1, bounds.getWidth() - 8, noteHeight - 2,
                       juce::Justification::centredLeft);
        }
    }

    g.setColour(juce::Colour(0xff0d1117));
    g.drawVerticalLine(bounds.getRight() - 1, static_cast<float>(bounds.getY()),
                       static_cast<float>(bounds.getBottom()));
}

void PianoRoll::drawGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    double tempo = transport.getTempo();
    double pixelsPerBeat = pixelsPerSecond * 60.0 / tempo;
    double beatDuration = 60.0 / tempo;
    double snapSeconds = snapDivision * beatDuration;

    int firstBeat = static_cast<int>(scrollX / pixelsPerBeat) - 1;
    int lastBeat = firstBeat + static_cast<int>(getGridWidth() / pixelsPerBeat) + 3;

    for (int beat = firstBeat; beat <= lastBeat; ++beat)
    {
        int x = static_cast<int>(beat * pixelsPerBeat - scrollX) + keyboardWidth;
        bool isBar = (beat % 4 == 0);

        g.setColour(isBar ? juce::Colour(0xff333355) : juce::Colour(0xff1e2d4a));
        g.drawVerticalLine(x, static_cast<float>(bounds.getY()),
                           static_cast<float>(bounds.getBottom()));
    }

    int snapPixels = static_cast<int>(snapSeconds * pixelsPerSecond);
    if (snapPixels > 4)
    {
        int firstSnap = static_cast<int>(scrollX / snapPixels);
        int lastSnap = firstSnap + static_cast<int>(getGridWidth() / snapPixels) + 2;

        for (int s = firstSnap; s <= lastSnap; ++s)
        {
            double snapTimeVal = s * snapSeconds;
            double beatPos = snapTimeVal / beatDuration;
            bool isBarSnap = (std::abs(beatPos - std::round(beatPos / 4.0) * 4.0) < 0.001);
            bool isBeatSnap = (std::abs(beatPos - std::round(beatPos)) < 0.001);

            if (!isBarSnap && !isBeatSnap)
            {
                int x = static_cast<int>(snapTimeVal * pixelsPerSecond - scrollX) + keyboardWidth;
                g.setColour(juce::Colour(0xff141e30));
                g.drawVerticalLine(x, static_cast<float>(bounds.getY()),
                                   static_cast<float>(bounds.getBottom()));
            }
        }
    }

    for (int noteNum = lowestNote; noteNum <= highestNote; ++noteNum)
    {
        int y = noteNumberToY(noteNum);
        bool cNote = (noteNum % 12 == 0);

        g.setColour(cNote ? juce::Colour(0xff1e2d4a) : juce::Colour(0xff111922));
        g.drawHorizontalLine(y, static_cast<float>(keyboardWidth),
                             static_cast<float>(bounds.getRight()));
    }
}

void PianoRoll::drawNotes(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (currentClip == nullptr)
        return;

    double clipStart = currentClip->getTimelineStart();

    for (int i = 0; i < currentClip->getNumNotes(); ++i)
    {
        auto note = currentClip->getNote(i);
        int noteY = noteNumberToY(note.noteNumber);
        int noteX = timeToX(note.startTime + clipStart) + keyboardWidth;
        int noteW = static_cast<int>(note.duration * pixelsPerSecond);
        noteW = juce::jmax(noteW, 4);

        if (noteX + noteW < keyboardWidth || noteX > bounds.getRight())
            continue;
        if (noteY + noteHeight < bounds.getY() || noteY > bounds.getBottom())
            continue;

        bool selected = selectedNotes.contains(i);
        juce::Colour baseColour = getNoteColour(note.noteNumber);

        if (selected)
            baseColour = baseColour.brighter(0.3f);

        g.setColour(baseColour);
        auto noteBounds = juce::Rectangle<int>(noteX, noteY, noteW, noteHeight - 1);
        g.fillRoundedRectangle(noteBounds.toFloat(), 2.0f);

        if (noteW > 20)
        {
            g.setColour(juce::Colour(0xff000000).withAlpha(0.3f));
            g.setFont(juce::Font(juce::FontOptions(9.0f)));
            g.drawText(getNoteName(note.noteNumber),
                       noteBounds.reduced(3, 0),
                       juce::Justification::centredLeft);
        }

        if (selected)
        {
            g.setColour(juce::Colour(0xffffffff));
            g.drawRoundedRectangle(noteBounds.toFloat(), 2.0f, 1.5f);
        }
        else
        {
            g.setColour(baseColour.brighter(0.15f));
            g.drawRoundedRectangle(noteBounds.toFloat(), 2.0f, 0.5f);
        }
    }
}

void PianoRoll::drawPlayhead(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    double playheadTime = transport.getPositionInSeconds();
    int playheadX = timeToX(playheadTime) + keyboardWidth;

    if (playheadX >= keyboardWidth && playheadX <= bounds.getRight())
    {
        g.setColour(juce::Colour(0xffff3333));
        g.drawVerticalLine(playheadX, static_cast<float>(bounds.getY()),
                           static_cast<float>(bounds.getBottom()));
    }
}

// ──────────────────────────────────────────────────────────────
// Mouse interaction
// ──────────────────────────────────────────────────────────────

void PianoRoll::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
    {
        int noteIdx = getNoteAtPosition(event.x, event.y);
        if (noteIdx >= 0 && currentClip != nullptr)
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Delete Note");
            menu.addSectionHeader("Velocity");
            menu.addItem(2, "Velocity: 1.0 (ff)");
            menu.addItem(3, "Velocity: 0.8 (mf)");
            menu.addItem(4, "Velocity: 0.5 (mp)");
            menu.addItem(5, "Velocity: 0.2 (pp)");

            menu.showMenuAsync(juce::PopupMenu::Options(), [this, noteIdx](int result) {
                if (currentClip == nullptr) return;
                if (result == 1)
                {
                    currentClip->removeNote(noteIdx);
selectedNotes.removeAllInstancesOf(noteIdx);
                    if (onNotesChanged) onNotesChanged();
                    repaint();
                }
                else if (result >= 2 && result <= 5 && noteIdx < currentClip->getNumNotes())
                {
                    float vel = (result == 2) ? 1.0f : (result == 3) ? 0.8f :
                                (result == 4) ? 0.5f : 0.2f;
                    auto note = currentClip->getNote(noteIdx);
                    note.velocity = vel;
                    currentClip->setNote(noteIdx, note);
                    if (onNotesChanged) onNotesChanged();
                    repaint();
                }
            });
        }
        return;
    }

    if (event.mods.isCommandDown() && event.x > keyboardWidth)
    {
        double clickTime = snapTime(xToTime(event.x - keyboardWidth));
        int clickNote = yToNoteNumber(event.y);
        currentClip->addNote(clickNote, clickTime, 60.0 / transport.getTempo(), 0.8f);
        selectedNotes.clear();
        selectedNotes.add(currentClip->getNumNotes() - 1);
        if (onNotesChanged) onNotesChanged();
        repaint();
        return;
    }

    int noteIdx = getNoteAtPosition(event.x, event.y);

    if (noteIdx >= 0)
    {
        if (event.mods.isShiftDown())
        {
            if (selectedNotes.contains(noteIdx))
                selectedNotes.removeAllInstancesOf(noteIdx);
            else
                selectedNotes.add(noteIdx);
        }
        else
        {
            if (!selectedNotes.contains(noteIdx))
            {
                selectedNotes.clear();
                selectedNotes.add(noteIdx);
            }
        }

        dragNoteIndex = noteIdx;
        auto note = currentClip->getNote(noteIdx);
        dragStartTime = xToTime(event.x - keyboardWidth);
        dragStartNote = note.noteNumber;
        dragOriginalStartTime = note.startTime;
        dragOriginalDuration = note.duration;
        dragStartBounds = juce::Rectangle<int>(
            timeToX(note.startTime + currentClip->getTimelineStart()) + keyboardWidth,
            noteNumberToY(note.noteNumber),
            static_cast<int>(note.duration * pixelsPerSecond),
            noteHeight);

        if (isOnResizeEdge(event.x, event.y, noteIdx))
            dragMode = DragMode::Resizing;
        else
            dragMode = DragMode::Moving;
    }
    else if (event.x > keyboardWidth)
    {
        dragMode = DragMode::Creating;
        double clickTime = snapTime(xToTime(event.x - keyboardWidth));
        int clickNote = yToNoteNumber(event.y);

        currentClip->addNote(clickNote, clickTime, 0.25 * 60.0 / transport.getTempo(), 0.8f);
        dragNoteIndex = currentClip->getNumNotes() - 1;

        selectedNotes.clear();
        selectedNotes.add(dragNoteIndex);
        if (onNotesChanged) onNotesChanged();
    }

    repaint();
}

void PianoRoll::mouseDrag(const juce::MouseEvent& event)
{
    if (currentClip == nullptr)
        return;

    if (dragMode == DragMode::Moving && dragNoteIndex >= 0 &&
        dragNoteIndex < currentClip->getNumNotes())
    {
        double timeDelta = xToTime(event.x - keyboardWidth) - dragStartTime;
        int noteDelta = yToNoteNumber(event.y) - dragStartNote;

        double newTime = snapTime(dragOriginalStartTime + timeDelta);
        newTime = juce::jmax(0.0, newTime);

        int newNote = juce::jlimit(lowestNote, highestNote - 1,
                                    dragStartNote + noteDelta);

        auto note = currentClip->getNote(dragNoteIndex);
        note.startTime = newTime;
        note.noteNumber = newNote;
        currentClip->setNote(dragNoteIndex, note);
        if (onNotesChanged) onNotesChanged();
        repaint();
    }
    else if (dragMode == DragMode::Resizing && dragNoteIndex >= 0 &&
             dragNoteIndex < currentClip->getNumNotes())
    {
        double currentTime = xToTime(event.x - keyboardWidth);
        double endTime = snapTime(currentTime);
        double newDuration = endTime - dragOriginalStartTime;
        newDuration = juce::jmax(60.0 / transport.getTempo() * 0.25, newDuration);

        auto note = currentClip->getNote(dragNoteIndex);
        note.duration = newDuration;
        currentClip->setNote(dragNoteIndex, note);
        if (onNotesChanged) onNotesChanged();
        repaint();
    }
    else if (dragMode == DragMode::Creating && dragNoteIndex >= 0 &&
             dragNoteIndex < currentClip->getNumNotes())
    {
        double currentTime = xToTime(event.x - keyboardWidth);
        double noteStart = dragOriginalStartTime;
        double endTime = snapTime(currentTime);

        double newDuration = std::abs(endTime - noteStart);
        newDuration = juce::jmax(60.0 / transport.getTempo() * 0.25, newDuration);

        auto note = currentClip->getNote(dragNoteIndex);
        note.duration = newDuration;
        currentClip->setNote(dragNoteIndex, note);
        if (onNotesChanged) onNotesChanged();
        repaint();
    }

    int keyNote = yToNoteNumber(event.y);
    if (keyNote != hoveredKeyNote)
    {
        hoveredKeyNote = keyNote;
        repaint();
    }
}

void PianoRoll::mouseUp(const juce::MouseEvent& /*event*/)
{
    dragMode = DragMode::None;
    dragNoteIndex = -1;
}

void PianoRoll::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (currentClip == nullptr || event.x <= keyboardWidth)
        return;

    double clickTime = snapTime(xToTime(event.x - keyboardWidth));
    int clickNote = yToNoteNumber(event.y);

    currentClip->addNote(clickNote, clickTime,
                         60.0 / transport.getTempo(), 0.8f);
    selectedNotes.clear();
    selectedNotes.add(currentClip->getNumNotes() - 1);
    if (onNotesChanged) onNotesChanged();
    repaint();
}

void PianoRoll::mouseWheelMove(const juce::MouseEvent& event,
                                const juce::MouseWheelDetails& wheel)
{
    if (event.mods.isAltDown())
    {
        double oldCenterTime = xToTime(getGridWidth() / 2);
        pixelsPerSecond = juce::jlimit(20.0, 800.0,
                                        pixelsPerSecond + wheel.deltaY * 15.0);
        repaint();
        (void)oldCenterTime;
    }
    else if (event.mods.isShiftDown())
    {
        scrollX += wheel.deltaY > 0 ? -40.0 : 40.0;
        scrollX = juce::jmax(0.0, scrollX);
        repaint();
    }
    else
    {
        scrollY += wheel.deltaY > 0 ? -30.0 : 30.0;
        scrollY = juce::jmax(0.0, scrollY);
        double maxScrollY = static_cast<double>((highestNote - lowestNote) * noteHeight
                                                - getGridHeight());
        scrollY = juce::jmin(scrollY, juce::jmax(0.0, maxScrollY));
        repaint();
    }
}

void PianoRoll::mouseMove(const juce::MouseEvent& event)
{
    int keyNote = yToNoteNumber(event.y);
    if (keyNote != hoveredKeyNote)
    {
        hoveredKeyNote = keyNote;
        repaint();
    }
}

// ──────────────────────────────────────────────────────────────
// Public setters
// ──────────────────────────────────────────────────────────────

void PianoRoll::setMidiClip(MidiClip* clip)
{
    currentClip = clip;
    selectedNotes.clear();
    dragNoteIndex = -1;
    hoveredNoteIndex = -1;
    dragMode = DragMode::None;
    scrollX = 0.0;
    scrollY = 0.0;
    repaint();
}

void PianoRoll::setPixelsPerSecond(double pps)
{
    pixelsPerSecond = juce::jlimit(20.0, 800.0, pps);
    repaint();
}

void PianoRoll::setSnapEnabled(bool snap)
{
    snapEnabled = snap;
}

void PianoRoll::setSnapDivision(double divisionBeats)
{
    snapDivision = juce::jmax(0.0625, divisionBeats);
}

// ──────────────────────────────────────────────────────────────
// Timer & ScrollBar
// ──────────────────────────────────────────────────────────────

void PianoRoll::timerCallback()
{
    if (transport.isPlaying())
        repaint();
}

void PianoRoll::scrollBarMoved(juce::ScrollBar* /*scrollBar*/, double newRangeStart)
{
    scrollX = newRangeStart;
    repaint();
}

} // namespace harmonic_engine
