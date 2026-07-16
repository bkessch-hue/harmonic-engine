#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace harmonic_engine
{

class ToolbarButton : public juce::Button
{
public:
    ToolbarButton(const juce::String& toolTip, juce::Colour activeColour);
    ~ToolbarButton() override;

    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override;
    void setActive(bool active);
    bool isActive() const;

private:
    bool activeState = false;
    juce::Colour activeColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToolbarButton)
};

class InteractiveToolbar : public juce::Component,
                           public juce::Button::Listener,
                           public juce::Timer
{
public:
    InteractiveToolbar();
    ~InteractiveToolbar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

    void setPlaying(bool playing);
    void setRecording(bool recording);
    void setMetronomeOn(bool on);
    void setCountInOn(bool on);
    void setInputMonitorOn(bool on);
    void setOverdubMode(bool overdub);

    std::function<void()> onPlay;
    std::function<void()> onStop;
    std::function<void()> onPause;
    std::function<void()> onRecord;
    std::function<void()> onSkipToStart;
    std::function<void()> onSkipToEnd;
    std::function<void()> onToggleMetronome;
    std::function<void()> onToggleCountIn;
    std::function<void()> onToggleInputMonitoring;
    std::function<void()> onRecordOverdub;
    std::function<void()> onRecordReplace;
    std::function<void()> onArmTrack;
    std::function<void()> onAddAudioTrack;
    std::function<void()> onAddMidiTrack;
    std::function<void()> onImportAudio;
    std::function<void()> onExportAudio;
    std::function<void()> onUndo;
    std::function<void()> onRedo;

private:
    ToolbarButton playBtn;
    ToolbarButton pauseBtn;
    ToolbarButton stopBtn;
    ToolbarButton recordBtn;
    ToolbarButton skipStartBtn;
    ToolbarButton skipEndBtn;

    ToolbarButton metronomeBtn;
    ToolbarButton countInBtn;
    ToolbarButton inputMonBtn;
    ToolbarButton overdubBtn;
    ToolbarButton armBtn;

    ToolbarButton addAudioBtn;
    ToolbarButton addMidiBtn;
    ToolbarButton importBtn;
    ToolbarButton exportBtn;
    ToolbarButton undoBtn;
    ToolbarButton redoBtn;

    std::vector<ToolbarButton*> allButtons;

    void positionButtons();
    void setupButton(ToolbarButton& btn, const juce::String& tip, bool isActive,
                     std::function<void()> callback);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InteractiveToolbar)
};

} // namespace harmonic_engine
