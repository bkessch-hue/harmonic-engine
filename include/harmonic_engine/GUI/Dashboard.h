// Dashboard.h — Root layout component for the DAW interface.
// Replaces MainEditor's ad-hoc resized() with a structured,
// responsive layout using collapsible panels.

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/GUI/DesignTokens.h"
#include "harmonic_engine/AudioEngine/Engine.h"

namespace harmonic_engine
{

// Forward declarations — Dashboard only holds references, not ownership.
class BrowserPanel;
class InspectorPanel;
class BottomWorkspace;
class StatusBar;

// CollapsibleResizer — a vertical or horizontal strip that the user
// can drag to resize adjacent panels. Double-click collapses the panel.
class CollapsibleResizer : public juce::Component
{
public:
    enum class Direction { Horizontal, Vertical };

    CollapsibleResizer(Direction dir, std::function<void(float)> onResize);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

private:
    Direction direction;
    bool collapsed = false;
    float dragStartPos = 0.0f;
    std::function<void(float)> onResizeCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CollapsibleResizer)
};


class Dashboard : public juce::Component
{
public:
    Dashboard(Engine& engine);
    ~Dashboard() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Panel visibility toggles
    void setBrowserVisible(bool visible);
    void setInspectorVisible(bool visible);
    void setBottomVisible(bool visible);
    bool isBrowserVisible() const;
    bool isInspectorVisible() const;
    bool isBottomVisible() const;

    // Access to sub-panels for callback wiring
    Engine& getEngine() { return audioEngine; }

private:
    Engine& audioEngine;

    // ── Panel geometry ──────────────────────────────────────
    int browserWidth  = Tokens::kBrowserDefaultWidth;
    int inspectorWidth = Tokens::kInspectorDefaultWidth;
    int bottomHeight  = Tokens::kBottomDefaultHeight;
    bool browserVisible  = true;
    bool inspectorVisible = true;
    bool bottomVisible   = true;

    // ── Resizer strips ──────────────────────────────────────
    // These are raw pointers; owned by juce::Component (addAndMakeVisible).
    CollapsibleResizer* browserResizer  = nullptr;
    CollapsibleResizer* inspectorResizer = nullptr;
    CollapsibleResizer* bottomResizer   = nullptr;

    // Placeholder panels (will be replaced by real ones in later stages)
    class PlaceholderPanel;
    std::unique_ptr<PlaceholderPanel> browserPlaceholder;
    std::unique_ptr<PlaceholderPanel> inspectorPlaceholder;

    void layoutPanels();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Dashboard)
};

} // namespace harmonic_engine
