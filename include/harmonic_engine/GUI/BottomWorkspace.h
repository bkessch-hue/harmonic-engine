// BottomWorkspace.h — Resizable tabbed bottom panel containing:
// Mixer, Piano Roll, Audio Editor, and Automation tabs.

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/GUI/DesignTokens.h"

namespace harmonic_engine
{

class BottomWorkspace : public juce::Component,
                        public juce::Button::Listener
{
public:
    BottomWorkspace();
    ~BottomWorkspace() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;

    void setActiveTab(int index);
    int getActiveTab() const;

    // Set the component displayed in each tab.
    // Ownership is NOT transferred — caller retains ownership.
    void setMixerComponent(juce::Component* comp);
    void setPianoRollComponent(juce::Component* comp);
    void setAudioEditorComponent(juce::Component* comp);
    void setAutomationComponent(juce::Component* comp);
    void setStepSequencerComponent(juce::Component* comp);

    std::function<void(int)> onTabChanged;

private:
    struct Tab
    {
        juce::String name;
        juce::TextButton* button = nullptr;
        juce::Component* content = nullptr;
    };

    std::vector<Tab> tabs;
    int activeTab = 0;

    void updateVisibility();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomWorkspace)
};

} // namespace harmonic_engine
