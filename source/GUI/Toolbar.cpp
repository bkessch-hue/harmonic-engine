#include "harmonic_engine/GUI/Toolbar.h"

namespace harmonic_engine
{

ToolbarButton::ToolbarButton(const juce::String& tip, juce::Colour active)
    : juce::Button(tip),
      activeColour(active)
{
    setTooltip(tip);
    setClickingTogglesState(false);
}

ToolbarButton::~ToolbarButton()
{
}

void ToolbarButton::paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    juce::Colour bgColour(0xff1e2d4a);
    if (activeState)
        bgColour = activeColour;
    else if (isButtonDown)
        bgColour = juce::Colour(0xff2a4a7a);
    else if (isMouseOver)
        bgColour = juce::Colour(0xff243a5c);

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour(0xff404060));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    juce::Colour textColour = activeState ? juce::Colour(0xffffffff) : juce::Colour(0xffc0c0d0);
    g.setColour(textColour);
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.drawText(getButtonText(), bounds, juce::Justification::centred, false);
}

void ToolbarButton::setActive(bool active)
{
    activeState = active;
    repaint();
}

bool ToolbarButton::isActive() const
{
    return activeState;
}

InteractiveToolbar::InteractiveToolbar()
    : playBtn("Play (Space)", juce::Colour(0xff2ecc71)),
      pauseBtn("Pause (P)", juce::Colour(0xfff39c12)),
      stopBtn("Stop (.)", juce::Colour(0xffe74c3c)),
      recordBtn("Record (R)", juce::Colour(0xffff3333)),
      skipStartBtn("Skip to Start (|<)", juce::Colour(0xff3498db)),
      skipEndBtn("Skip to End (>|)", juce::Colour(0xff3498db)),
      metronomeBtn("Metronome (M)", juce::Colour(0xff9b59b6)),
      countInBtn("Count-In (C)", juce::Colour(0xff9b59b6)),
      inputMonBtn("Input Monitoring (I)", juce::Colour(0xff1abc9c)),
      overdubBtn("Overdub / Replace (O)", juce::Colour(0xffe67e22)),
      armBtn("Arm Track (A)", juce::Colour(0xffff6666)),
      addAudioBtn("Add Audio Track (Cmd+T)", juce::Colour(0xff4a90d9)),
      addMidiBtn("Add MIDI Track (Cmd+Shift+T)", juce::Colour(0xff4a90d9)),
      importBtn("Import Audio (Cmd+I)", juce::Colour(0xff27ae60)),
      exportBtn("Export Audio (Cmd+E)", juce::Colour(0xff27ae60)),
      undoBtn("Undo (Cmd+Z)", juce::Colour(0xff555577)),
      redoBtn("Redo (Cmd+Shift+Z)", juce::Colour(0xff555577))
{
    allButtons = {
        &playBtn, &pauseBtn, &stopBtn, &recordBtn, &skipStartBtn, &skipEndBtn,
        nullptr,
        &metronomeBtn, &countInBtn, &inputMonBtn, &overdubBtn, &armBtn,
        nullptr,
        &addAudioBtn, &addMidiBtn, &importBtn, &exportBtn, &undoBtn, &redoBtn
    };

    for (auto* btn : allButtons)
    {
        if (btn != nullptr)
        {
            btn->addListener(this);
            addAndMakeVisible(*btn);
        }
    }

    startTimerHz(10);
}

InteractiveToolbar::~InteractiveToolbar()
{
    stopTimer();
    for (auto* btn : allButtons)
    {
        if (btn != nullptr)
            btn->removeListener(this);
    }
}

void InteractiveToolbar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff121a2e));
    g.fillRect(bounds);

    g.setColour(juce::Colour(0xff2a2a4a));
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
}

void InteractiveToolbar::resized()
{
    positionButtons();
}

void InteractiveToolbar::positionButtons()
{
    const int btnHeight = getHeight() - 8;
    const int btnWidth = 70;
    const int gap = 3;
    const int sepWidth = 10;
    int x = 8;
    int y = 4;

    auto placeBtn = [&](ToolbarButton& btn)
    {
        btn.setBounds(x, y, btnWidth, btnHeight);
        x += btnWidth + gap;
    };

    auto placeSep = [&]()
    {
        x += sepWidth;
    };

    placeBtn(playBtn);
    placeBtn(pauseBtn);
    placeBtn(stopBtn);
    placeBtn(recordBtn);
    placeBtn(skipStartBtn);
    placeBtn(skipEndBtn);
    placeSep();
    placeBtn(metronomeBtn);
    placeBtn(countInBtn);
    placeBtn(inputMonBtn);
    placeBtn(overdubBtn);
    placeBtn(armBtn);
    placeSep();
    placeBtn(addAudioBtn);
    placeBtn(addMidiBtn);
    placeBtn(importBtn);
    placeBtn(exportBtn);
    placeBtn(undoBtn);
    placeBtn(redoBtn);
}

void InteractiveToolbar::buttonClicked(juce::Button* button)
{
    if (button == &playBtn && onPlay) onPlay();
    else if (button == &pauseBtn && onPause) onPause();
    else if (button == &stopBtn && onStop) onStop();
    else if (button == &recordBtn && onRecord) onRecord();
    else if (button == &skipStartBtn && onSkipToStart) onSkipToStart();
    else if (button == &skipEndBtn && onSkipToEnd) onSkipToEnd();
    else if (button == &metronomeBtn && onToggleMetronome) onToggleMetronome();
    else if (button == &countInBtn && onToggleCountIn) onToggleCountIn();
    else if (button == &inputMonBtn && onToggleInputMonitoring) onToggleInputMonitoring();
    else if (button == &overdubBtn) { if (overdubBtn.isActive() && onRecordReplace) onRecordReplace(); else if (onRecordOverdub) onRecordOverdub(); }
    else if (button == &armBtn && onArmTrack) onArmTrack();
    else if (button == &addAudioBtn && onAddAudioTrack) onAddAudioTrack();
    else if (button == &addMidiBtn && onAddMidiTrack) onAddMidiTrack();
    else if (button == &importBtn && onImportAudio) onImportAudio();
    else if (button == &exportBtn && onExportAudio) onExportAudio();
    else if (button == &undoBtn && onUndo) onUndo();
    else if (button == &redoBtn && onRedo) onRedo();
}

void InteractiveToolbar::timerCallback()
{
}

void InteractiveToolbar::setPlaying(bool playing)
{
    playBtn.setActive(playing);
}

void InteractiveToolbar::setRecording(bool recording)
{
    recordBtn.setActive(recording);
}

void InteractiveToolbar::setMetronomeOn(bool on)
{
    metronomeBtn.setActive(on);
}

void InteractiveToolbar::setCountInOn(bool on)
{
    countInBtn.setActive(on);
}

void InteractiveToolbar::setInputMonitorOn(bool on)
{
    inputMonBtn.setActive(on);
}

void InteractiveToolbar::setOverdubMode(bool overdub)
{
    overdubBtn.setActive(overdub);
}

} // namespace harmonic_engine
