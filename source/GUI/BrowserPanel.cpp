#include "harmonic_engine/GUI/BrowserPanel.h"

namespace harmonic_engine
{

// ============================================================================
// BrowserContent — the scrollable content inside the viewport
// ============================================================================

class BrowserPanel::BrowserContent : public juce::Component
{
public:
    BrowserContent(BrowserPanel& owner) : parent(owner) {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.fillAll(Tokens::Colours::bgDark());

        int y = Tokens::kSpace4;

        for (int i = 0; i < static_cast<int>(parent.categories.size()); ++i)
        {
            auto& cat = parent.categories[i];

            // Category header
            auto headerBounds = juce::Rectangle<int>(
                Tokens::kSpace4, y, getWidth() - Tokens::kSpace8, Tokens::kSpace24);

            // Header background
            g.setColour(cat.expanded ? Tokens::Colours::bgRaised() : Tokens::Colours::bgBase());
            g.fillRoundedRectangle(headerBounds.toFloat(), Tokens::kCornerRadiusSmall);

            // Expand/collapse arrow
            g.setColour(Tokens::Colours::textSecondary());
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            juce::String arrow = cat.expanded ? "\xe2\x96\xbc" : "\xe2\x96\xb6"; // triangle
            g.drawText(arrow,
                       headerBounds.getX() + Tokens::kSpace4, headerBounds.getY(),
                       16, headerBounds.getHeight(),
                       juce::Justification::centredLeft);

            // Category name
            g.setColour(Tokens::Colours::textPrimary());
            g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeNormal))));
            g.drawText(cat.name,
                       headerBounds.getX() + 22, headerBounds.getY(),
                       headerBounds.getWidth() - 26, headerBounds.getHeight(),
                       juce::Justification::centredLeft);

            y += headerBounds.getHeight() + Tokens::kSpace2;

            // Items (if expanded)
            if (cat.expanded)
            {
                for (int j = 0; j < static_cast<int>(cat.items.size()); ++j)
                {
                    auto itemBounds = juce::Rectangle<int>(
                        Tokens::kSpace16, y,
                        getWidth() - Tokens::kSpace24, Tokens::kButtonHeight);

                    g.setColour(Tokens::Colours::textSecondary());
                    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
                    g.drawText(cat.items[j], itemBounds,
                               juce::Justification::centredLeft, true);

                    y += itemBounds.getHeight() + Tokens::kSpace1;
                }
                y += Tokens::kSpace4;
            }
            else
            {
                y += Tokens::kSpace2;
            }
        }

        totalHeight = y + Tokens::kSpace8;
    }

    int getTotalHeight() const { return totalHeight; }

    void mouseDown(const juce::MouseEvent& event) override
    {
        int clickY = event.getPosition().y;
        int y = Tokens::kSpace4;

        for (int i = 0; i < static_cast<int>(parent.categories.size()); ++i)
        {
            auto& cat = parent.categories[i];
            int headerBottom = y + Tokens::kSpace24;

            if (clickY >= y && clickY < headerBottom)
            {
                cat.expanded = !cat.expanded;
                repaint();
                resized();
                return;
            }

            y = headerBottom + Tokens::kSpace2;

            if (cat.expanded)
            {
                for (int j = 0; j < static_cast<int>(cat.items.size()); ++j)
                {
                    int itemBottom = y + Tokens::kButtonHeight;
                    if (clickY >= y && clickY < itemBottom)
                    {
                        if (parent.onItemSelected)
                            parent.onItemSelected(cat.items[j]);
                        return;
                    }
                    y = itemBottom + Tokens::kSpace1;
                }
                y += Tokens::kSpace4;
            }
            else
            {
                y += Tokens::kSpace2;
            }
        }
    }

private:
    BrowserPanel& parent;
    int totalHeight = 200;
};

// ============================================================================
// BrowserPanel
// ============================================================================

BrowserPanel::BrowserPanel()
{
    initCategories();

    content = std::make_unique<BrowserContent>(*this);
    viewport.setViewedComponent(content.get(), false);
    viewport.setScrollBarThickness(6);
    addAndMakeVisible(viewport);
}

BrowserPanel::~BrowserPanel() = default;

void BrowserPanel::initCategories()
{
    categories.resize(6);

    categories[0].name = "Projects";
    categories[0].expanded = true;
    categories[0].items = { "Recent Projects", "Templates", "Recovery" };

    categories[1].name = "Instruments";
    categories[1].expanded = false;
    categories[1].items = { "Sine", "Saw", "Square", "Triangle", "FM Synth",
                            "Electric Piano", "Organ", "Pluck" };

    categories[2].name = "Samples";
    categories[2].expanded = false;
    categories[2].items = { "Drums", "Percussion", "Vocals", "FX",
                            "Synths", "Keys", "Guitars" };

    categories[3].name = "Loops";
    categories[3].expanded = false;
    categories[3].items = { "808 Loops", "Drum Loops", "Melodic Loops",
                            "Bass Loops", "Vocal Loops" };

    categories[4].name = "Audio Effects";
    categories[4].expanded = false;
    categories[4].items = { "Equalizer", "Compressor", "Reverb", "Delay",
                            "Chorus", "Flanger", "Phaser" };

    categories[5].name = "Presets";
    categories[5].expanded = false;
    categories[5].items = { "Mixing", "Mastering", "Sound Design", "Creative" };
}

void BrowserPanel::paint(juce::Graphics& g)
{
    g.fillAll(Tokens::Colours::bgDark());

    // Draw title bar
    auto titleBounds = getLocalBounds().removeFromTop(Tokens::kSpace24);
    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(titleBounds);
    g.setColour(Tokens::Colours::textPrimary());
    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeMedium))));
    g.drawText("Browser", titleBounds.reduced(Tokens::kSpace8, 0),
               juce::Justification::centredLeft);

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(Tokens::kSpace24, 0.0f, static_cast<float>(getWidth()));
}

void BrowserPanel::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(Tokens::kSpace24); // title bar
    viewport.setBounds(bounds);

    if (content != nullptr)
    {
        int totalH = content->getTotalHeight();
        content->setSize(getWidth(), juce::jmax(totalH, getHeight()));
    }
}

void BrowserPanel::setCategoryExpanded(int index, bool expanded)
{
    if (index >= 0 && index < static_cast<int>(categories.size()))
    {
        categories[index].expanded = expanded;
        if (content != nullptr) content->repaint();
    }
}

bool BrowserPanel::isCategoryExpanded(int index) const
{
    if (index >= 0 && index < static_cast<int>(categories.size()))
        return categories[index].expanded;
    return false;
}

} // namespace harmonic_engine
