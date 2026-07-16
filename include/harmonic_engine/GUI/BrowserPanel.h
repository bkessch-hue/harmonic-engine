// BrowserPanel.h — Collapsible left panel containing browsable categories:
// Projects, Instruments, Samples, Loops, Audio Effects, Presets.
// Each category is a tree-view section that expands/collapses.

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/GUI/DesignTokens.h"

namespace harmonic_engine
{

class BrowserPanel : public juce::Component
{
public:
    BrowserPanel();
    ~BrowserPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Category visibility toggles
    void setCategoryExpanded(int index, bool expanded);
    bool isCategoryExpanded(int index) const;

    struct Category
    {
        juce::String name;
        bool expanded = false;
        std::vector<juce::String> items;
    };

    std::function<void(const juce::String&)> onItemSelected;

private:
    juce::Viewport viewport;
    class BrowserContent;
    std::unique_ptr<BrowserContent> content;

    std::vector<Category> categories;

    void initCategories();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserPanel)
};

} // namespace harmonic_engine
