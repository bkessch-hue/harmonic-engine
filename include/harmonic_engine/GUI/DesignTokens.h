// DesignTokens.h — Shared design constants for the Harmonic Engine DAW.
// Header-only. Include from any GUI file to access consistent spacing,
// color, typography, and control-size values.

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace harmonic_engine
{
namespace Tokens
{

// ── Spacing (8px grid) ──────────────────────────────────────
constexpr int kSpace1 = 1;   // Hairline
constexpr int kSpace2 = 2;   // Tight gap
constexpr int kSpace4 = 4;   // Default inner padding
constexpr int kSpace8 = 8;   // Standard gap between related items
constexpr int kSpace12 = 12; // Gap between grouped sections
constexpr int kSpace16 = 16; // Gap between major sections
constexpr int kSpace24 = 24; // Panel header height / large gap
constexpr int kSpace32 = 32; // Panel minimum dimension

// ── Control Sizes ───────────────────────────────────────────
constexpr int kMenuBarHeight      = 24;
constexpr int kTransportHeight    = 40;
constexpr int kStatusBarHeight    = 22;
constexpr int kBrowserDefaultWidth = 220;
constexpr int kInspectorDefaultWidth = 240;
constexpr int kBottomDefaultHeight = 200;
constexpr int kTrackHeaderWidth   = 160;
constexpr int kRulerHeight        = 28;
constexpr int kGlobalTracksHeight = 80;
constexpr int kButtonHeight       = 24;
constexpr int kSliderHeight       = 20;
constexpr int kComboHeight        = 22;
constexpr int kTabHeight          = 28;
constexpr int kChannelStripWidth  = 72;
constexpr int kMeterWidth         = 6;

// ── Typography ──────────────────────────────────────────────
constexpr float kFontSizeSmall  = 10.0f;
constexpr float kFontSizeNormal = 11.0f;
constexpr float kFontSizeMedium = 12.0f;
constexpr float kFontSizeLarge  = 14.0f;
constexpr float kFontSizeTitle  = 16.0f;

// ── Border / Radius ─────────────────────────────────────────
constexpr float kBorderThickness = 1.0f;
constexpr float kCornerRadiusSmall = 3.0f;
constexpr float kCornerRadiusMedium = 4.0f;
constexpr float kCornerRadiusLarge = 6.0f;

// ── Colors ──────────────────────────────────────────────────
// Base palette — dark navy with single restrained accent.
namespace Colours
{
    // Backgrounds (darkest to lightest)
    inline juce::Colour bgDarkest()     { return juce::Colour(0xff0d1117); }
    inline juce::Colour bgDark()        { return juce::Colour(0xff161b22); }
    inline juce::Colour bgBase()        { return juce::Colour(0xff1a1f2e); }
    inline juce::Colour bgRaised()      { return juce::Colour(0xff21262d); }
    inline juce::Colour bgOverlay()     { return juce::Colour(0xff2d333b); }
    inline juce::Colour bgHighlight()   { return juce::Colour(0xff363d48); }

    // Borders
    inline juce::Colour borderSubtle()  { return juce::Colour(0xff21262d); }
    inline juce::Colour borderDefault() { return juce::Colour(0xff30363d); }
    inline juce::Colour borderStrong()  { return juce::Colour(0xff484f58); }

    // Text
    inline juce::Colour textPrimary()   { return juce::Colour(0xffc9d1d9); }
    inline juce::Colour textSecondary() { return juce::Colour(0xff8b949e); }
    inline juce::Colour textDisabled()  { return juce::Colour(0xff484f58); }
    inline juce::Colour textInverse()   { return juce::Colour(0xff0d1117); }

    // Accent — single restrained blue-teal
    inline juce::Colour accent()        { return juce::Colour(0xff58a6ff); }
    inline juce::Colour accentDim()     { return juce::Colour(0xff1f6feb); }
    inline juce::Colour accentBright()  { return juce::Colour(0xff79c0ff); }

    // Functional
    inline juce::Colour record()        { return juce::Colour(0xffda3633); }
    inline juce::Colour play()          { return juce::Colour(0xff3fb950); }
    inline juce::Colour loop()          { return juce::Colour(0xffd29922); }
    inline juce::Colour mute()          { return juce::Colour(0xffd29922); }
    inline juce::Colour solo()          { return juce::Colour(0xff58a6ff); }
    inline juce::Colour arm()           { return juce::Colour(0xffda3633); }

    // Meters
    inline juce::Colour meterLow()      { return juce::Colour(0xff3fb950); }
    inline juce::Colour meterMid()      { return juce::Colour(0xffd29922); }
    inline juce::Colour meterPeak()     { return juce::Colour(0xffda3633); }

    // Playhead
    inline juce::Colour playhead()      { return juce::Colour(0xffda3633); }

    // Selection
    inline juce::Colour selection()     { return juce::Colour(0xff1f6feb).withAlpha(0.35f); }

    // Track lane alternating
    inline juce::Colour laneEven()      { return juce::Colour(0xff0d1117); }
    inline juce::Colour laneOdd()       { return juce::Colour(0xff10141a); }

    // Track colours (assigned to tracks in order)
    inline juce::Colour trackColour(int index)
    {
        const juce::Colour palette[] = {
            juce::Colour(0xff58a6ff), // blue
            juce::Colour(0xff3fb950), // green
            juce::Colour(0xffd29922), // amber
            juce::Colour(0xfff78166), // coral
            juce::Colour(0xffbc8cff), // purple
            juce::Colour(0xff79c0ff), // sky
            juce::Colour(0xff56d364), // emerald
            juce::Colour(0xffe3b341), // gold
        };
        return palette[index % 8];
    }
}

} // namespace Tokens
} // namespace harmonic_engine
