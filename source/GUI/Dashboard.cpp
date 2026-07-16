#include "harmonic_engine/GUI/Dashboard.h"

namespace harmonic_engine
{

// ============================================================================
// CollapsibleResizer
// ============================================================================

CollapsibleResizer::CollapsibleResizer(Direction dir, std::function<void(float)> onResize)
    : direction(dir), onResizeCallback(std::move(onResize))
{
    if (direction == Direction::Horizontal)
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    else
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}

void CollapsibleResizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Draw the resizer strip
    g.setColour(Tokens::Colours::borderDefault());
    if (direction == Direction::Horizontal)
    {
        float cx = bounds.getCentreX();
        g.fillRect(cx - 0.5f, bounds.getY(), 1.0f, bounds.getHeight());
    }
    else
    {
        float cy = bounds.getCentreY();
        g.fillRect(bounds.getX(), cy - 0.5f, bounds.getWidth(), 1.0f);
    }

    // Draw drag handle dots
    g.setColour(Tokens::Colours::textDisabled());
    if (direction == Direction::Horizontal)
    {
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        for (int i = -1; i <= 1; ++i)
            g.fillEllipse(cx - 1.5f, cy + i * 6.0f - 1.5f, 3.0f, 3.0f);
    }
    else
    {
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        for (int i = -1; i <= 1; ++i)
            g.fillEllipse(cx + i * 6.0f - 1.5f, cy - 1.5f, 3.0f, 3.0f);
    }
}

void CollapsibleResizer::mouseDown(const juce::MouseEvent& e)
{
    if (direction == Direction::Horizontal)
        dragStartPos = static_cast<float>(e.getMouseDownX());
    else
        dragStartPos = static_cast<float>(e.getMouseDownY());
}

void CollapsibleResizer::mouseDrag(const juce::MouseEvent& e)
{
    float delta;
    if (direction == Direction::Horizontal)
        delta = static_cast<float>(e.getPosition().getX()) - dragStartPos;
    else
        delta = static_cast<float>(e.getPosition().getY()) - dragStartPos;

    if (onResizeCallback)
        onResizeCallback(delta);
}

void CollapsibleResizer::mouseDoubleClick(const juce::MouseEvent&)
{
    collapsed = !collapsed;
}

void CollapsibleResizer::setCollapsed(bool c) { collapsed = c; }
bool CollapsibleResizer::isCollapsed() const { return collapsed; }

// ============================================================================
// Dashboard — Placeholder panel (used for browser/inspector until real ones exist)
// ============================================================================

class Dashboard::PlaceholderPanel : public juce::Component
{
public:
    explicit PlaceholderPanel(const juce::String& label) : nameLabel(label) {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(Tokens::Colours::bgDark());
        g.fillRect(bounds);
        g.setColour(Tokens::Colours::borderDefault());
        g.drawRect(bounds, Tokens::kBorderThickness);
        g.setColour(Tokens::Colours::textSecondary());
        g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeMedium))));
        g.drawText(nameLabel, bounds, juce::Justification::centred);
    }

private:
    juce::String nameLabel;
};

// ============================================================================
// Dashboard
// ============================================================================

Dashboard::Dashboard(Engine& engine)
    : audioEngine(engine)
{
    browserPlaceholder = std::make_unique<PlaceholderPanel>("Browser");
    inspectorPlaceholder = std::make_unique<PlaceholderPanel>("Inspector");

    addAndMakeVisible(browserPlaceholder.get());
    addAndMakeVisible(inspectorPlaceholder.get());

    // Resizers are created on demand when panels are visible.
    // For now, create all three.
    browserResizer = new CollapsibleResizer(CollapsibleResizer::Direction::Horizontal,
        [this](float delta) {
            browserWidth = juce::jmax(0, juce::jmin(browserWidth + static_cast<int>(delta), 400));
            layoutPanels();
        });
    addAndMakeVisible(browserResizer);

    inspectorResizer = new CollapsibleResizer(CollapsibleResizer::Direction::Horizontal,
        [this](float delta) {
            inspectorWidth = juce::jmax(0, juce::jmin(inspectorWidth - static_cast<int>(delta), 500));
            layoutPanels();
        });
    addAndMakeVisible(inspectorResizer);

    bottomResizer = new CollapsibleResizer(CollapsibleResizer::Direction::Vertical,
        [this](float delta) {
            bottomHeight = juce::jmax(0, juce::jmin(bottomHeight - static_cast<int>(delta), 600));
            layoutPanels();
        });
    addAndMakeVisible(bottomResizer);
}

Dashboard::~Dashboard() = default;

void Dashboard::paint(juce::Graphics& g)
{
    g.fillAll(Tokens::Colours::bgDarkest());
}

void Dashboard::resized()
{
    layoutPanels();
}

void Dashboard::layoutPanels()
{
    auto bounds = getLocalBounds();

    // ── Browser (left) ─────────────────────────────────────
    if (browserVisible && browserWidth > 0)
    {
        browserPlaceholder->setBounds(bounds.removeFromLeft(browserWidth));
        bounds.removeFromLeft(Tokens::kSpace2); // gap
    }

    // ── Inspector (right) ──────────────────────────────────
    if (inspectorVisible && inspectorWidth > 0)
    {
        bounds.removeFromRight(Tokens::kSpace2);
        inspectorPlaceholder->setBounds(bounds.removeFromRight(inspectorWidth));
    }

    // ── Bottom workspace ───────────────────────────────────
    if (bottomVisible && bottomHeight > 0)
    {
        bounds.removeFromBottom(Tokens::kSpace2);
        // Bottom panel gets the full remaining width
        // (will be replaced by BottomWorkspace in Stage 7)
    }

    // ── Resizer strips ─────────────────────────────────────
    const int resizerThickness = 6;

    if (browserVisible)
    {
        browserResizer->setVisible(true);
        browserResizer->setBounds(
            browserPlaceholder->getRight(), browserPlaceholder->getY(),
            resizerThickness, browserPlaceholder->getHeight());
    }
    else
    {
        browserResizer->setVisible(false);
    }

    if (inspectorVisible)
    {
        inspectorResizer->setVisible(true);
        inspectorResizer->setBounds(
            inspectorPlaceholder->getX() - resizerThickness, inspectorPlaceholder->getY(),
            resizerThickness, inspectorPlaceholder->getHeight());
    }
    else
    {
        inspectorResizer->setVisible(false);
    }
}

// ── Panel visibility ────────────────────────────────────────
void Dashboard::setBrowserVisible(bool v)  { browserVisible = v;  layoutPanels(); }
void Dashboard::setInspectorVisible(bool v) { inspectorVisible = v; layoutPanels(); }
void Dashboard::setBottomVisible(bool v)   { bottomVisible = v;   layoutPanels(); }
bool Dashboard::isBrowserVisible() const   { return browserVisible; }
bool Dashboard::isInspectorVisible() const { return inspectorVisible; }
bool Dashboard::isBottomVisible() const    { return bottomVisible; }

} // namespace harmonic_engine
