#include "harmonic_engine/GUI/BottomWorkspace.h"

namespace harmonic_engine
{

BottomWorkspace::BottomWorkspace()
{
    // Create tab buttons
    const char* tabNames[] = { "Mixer", "Piano Roll", "Audio Editor", "Automation", "Drums" };
    const int numTabs = 5;

    for (int i = 0; i < numTabs; ++i)
    {
        Tab tab;
        tab.name = tabNames[i];
        tab.button = new juce::TextButton(tab.name);
        tab.button->setClickingTogglesState(false);
        tab.button->setColour(juce::TextButton::buttonColourId, Tokens::Colours::bgBase());
        tab.button->setColour(juce::TextButton::textColourOffId, Tokens::Colours::textSecondary());
        tab.button->addListener(this);
        addAndMakeVisible(tab.button);
        tabs.push_back(tab);
    }

    updateVisibility();
}

BottomWorkspace::~BottomWorkspace()
{
    for (auto& tab : tabs)
        delete tab.button;
}

void BottomWorkspace::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Tab bar background
    auto tabBarBounds = bounds.removeFromTop(Tokens::kTabHeight);
    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(tabBarBounds);

    // Active tab indicator
    if (activeTab >= 0 && activeTab < static_cast<int>(tabs.size()))
    {
        auto activeBounds = tabs[activeTab].button->getBounds();
        g.setColour(Tokens::Colours::accent());
        g.fillRect(activeBounds.getX(), activeBounds.getBottom() - 2,
                   activeBounds.getWidth(), 2);
    }

    // Content background
    g.setColour(Tokens::Colours::bgDarkest());
    g.fillRect(bounds);

    // Border
    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));
}

void BottomWorkspace::resized()
{
    auto bounds = getLocalBounds();

    // Tab bar
    auto tabBar = bounds.removeFromTop(Tokens::kTabHeight);
    const int tabW = 100;
    int x = Tokens::kSpace4;

    for (auto& tab : tabs)
    {
        tab.button->setBounds(x, tabBar.getY() + Tokens::kSpace2,
                              tabW, tabBar.getHeight() - Tokens::kSpace4);
        x += tabW + Tokens::kSpace2;
    }

    // Content area — fill with active component
    if (activeTab >= 0 && activeTab < static_cast<int>(tabs.size()))
    {
        auto* content = tabs[activeTab].content;
        if (content != nullptr)
            content->setBounds(bounds);
    }
}

void BottomWorkspace::setActiveTab(int index)
{
    if (index >= 0 && index < static_cast<int>(tabs.size()) && index != activeTab)
    {
        activeTab = index;
        updateVisibility();
        repaint();
        if (onTabChanged) onTabChanged(activeTab);
    }
}

int BottomWorkspace::getActiveTab() const
{
    return activeTab;
}

void BottomWorkspace::setMixerComponent(juce::Component* comp)
{
    if (tabs.size() > 0) tabs[0].content = comp;
    updateVisibility();
}

void BottomWorkspace::setPianoRollComponent(juce::Component* comp)
{
    if (tabs.size() > 1) tabs[1].content = comp;
    updateVisibility();
}

void BottomWorkspace::setAudioEditorComponent(juce::Component* comp)
{
    if (tabs.size() > 2) tabs[2].content = comp;
    updateVisibility();
}

void BottomWorkspace::setAutomationComponent(juce::Component* comp)
{
    if (tabs.size() > 3) tabs[3].content = comp;
    updateVisibility();
}

void BottomWorkspace::setStepSequencerComponent(juce::Component* comp)
{
    if (tabs.size() > 4) tabs[4].content = comp;
    updateVisibility();
}

void BottomWorkspace::buttonClicked(juce::Button* button)
{
    for (int i = 0; i < static_cast<int>(tabs.size()); ++i)
    {
        if (tabs[i].button == button)
        {
            setActiveTab(i);
            return;
        }
    }
}

void BottomWorkspace::updateVisibility()
{
    for (int i = 0; i < static_cast<int>(tabs.size()); ++i)
    {
        auto* content = tabs[i].content;
        if (content != nullptr)
        {
            bool isActive = (i == activeTab);
            content->setVisible(isActive);
        }

        // Update tab button appearance
        if (tabs[i].button != nullptr)
        {
            bool isActive = (i == activeTab);
            tabs[i].button->setColour(juce::TextButton::buttonColourId,
                isActive ? Tokens::Colours::bgRaised() : Tokens::Colours::bgBase());
            tabs[i].button->setColour(juce::TextButton::textColourOffId,
                isActive ? Tokens::Colours::textPrimary() : Tokens::Colours::textSecondary());
        }
    }
    repaint();
}

} // namespace harmonic_engine
