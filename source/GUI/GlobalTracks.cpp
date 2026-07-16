#include "harmonic_engine/GUI/GlobalTracks.h"
#include "harmonic_engine/GUI/DesignTokens.h"

namespace harmonic_engine
{

GlobalTracks::GlobalTracks(Transport& t)
    : transport(t)
{
    tempoEvents.push_back({ 0.0, 120.0 });
    timeSignatures.push_back({ 0.0, 4, 4 });
    markers.push_back({ "Start", 0.0, Tokens::Colours::play() });

    startTimerHz(30);
}

GlobalTracks::~GlobalTracks()
{
    stopTimer();
}

void GlobalTracks::setPixelsPerSecond(double pps)
{
    pixelsPerSecond = pps;
    repaint();
}

void GlobalTracks::setViewStartTime(double seconds)
{
    viewStartTime = seconds;
    repaint();
}

int GlobalTracks::timeToX(double time) const
{
    return labelWidth + static_cast<int>((time - viewStartTime) * pixelsPerSecond);
}

double GlobalTracks::xToTime(int x) const
{
    return viewStartTime + static_cast<double>(x - labelWidth) / pixelsPerSecond;
}

void GlobalTracks::addMarker(const juce::String& name, double timeSeconds, juce::Colour colour)
{
    markers.push_back({ name, timeSeconds, colour });
    if (onMarkerAdded) onMarkerAdded();
    repaint();
}

void GlobalTracks::removeMarker(int index)
{
    if (index >= 0 && index < static_cast<int>(markers.size()))
    {
        markers.erase(markers.begin() + index);
        repaint();
    }
}

int GlobalTracks::getNumMarkers() const { return static_cast<int>(markers.size()); }
Marker GlobalTracks::getMarker(int index) const { return markers[index]; }

void GlobalTracks::addTempoEvent(double timeSeconds, double bpm)
{
    tempoEvents.push_back({ timeSeconds, bpm });
    if (onTempoChanged) onTempoChanged();
    repaint();
}

void GlobalTracks::removeTempoEvent(int index)
{
    if (index >= 0 && index < static_cast<int>(tempoEvents.size()))
    {
        tempoEvents.erase(tempoEvents.begin() + index);
        if (onTempoChanged) onTempoChanged();
        repaint();
    }
}

int GlobalTracks::getNumTempoEvents() const { return static_cast<int>(tempoEvents.size()); }
TempoEvent GlobalTracks::getTempoEvent(int index) const { return tempoEvents[index]; }

void GlobalTracks::addTimeSignature(double timeSeconds, int num, int den)
{
    timeSignatures.push_back({ timeSeconds, num, den });
    if (onTimeSignatureChanged) onTimeSignatureChanged();
    repaint();
}

void GlobalTracks::removeTimeSignature(int index)
{
    if (index >= 0 && index < static_cast<int>(timeSignatures.size()))
    {
        timeSignatures.erase(timeSignatures.begin() + index);
        if (onTimeSignatureChanged) onTimeSignatureChanged();
        repaint();
    }
}

int GlobalTracks::getNumTimeSignatures() const { return static_cast<int>(timeSignatures.size()); }
TimeSignature GlobalTracks::getTimeSignature(int index) const { return timeSignatures[index]; }

void GlobalTracks::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(Tokens::Colours::bgDarkest());

    int y = 0;

    auto labelArea = juce::Rectangle<int>(0, y, labelWidth, markerTrackHeight);

    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(labelArea);
    g.setColour(Tokens::Colours::textSecondary());
    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    g.drawText("Markers", labelArea.reduced(4), juce::Justification::centredLeft);

    g.setColour(Tokens::Colours::bgDark());
    g.fillRect(labelWidth, y, getWidth() - labelWidth, markerTrackHeight);

    for (int i = 0; i < static_cast<int>(markers.size()); ++i)
    {
        int mx = timeToX(markers[i].timeSeconds);
        if (mx < labelWidth || mx > getWidth()) continue;

        auto flagColor = markers[i].colour;

        juce::Path flag;
        flag.addRoundedRectangle(static_cast<float>(mx - 2), static_cast<float>(y + 2),
                                 4.0f, static_cast<float>(markerTrackHeight - 4), 2.0f);
        g.setColour(flagColor);
        g.fillPath(flag);

        g.setColour(Tokens::Colours::textPrimary());
        g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
        g.drawText(markers[i].name,
                   mx + 6, y + 2, 100, markerTrackHeight - 4,
                   juce::Justification::centredLeft);
    }

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(markerTrackHeight - 1, 0.0f, static_cast<float>(getWidth()));

    y += markerTrackHeight;

    auto tempoLabelArea = juce::Rectangle<int>(0, y, labelWidth, tempoTrackHeight);
    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(tempoLabelArea);
    g.setColour(Tokens::Colours::textSecondary());
    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    g.drawText("Tempo", tempoLabelArea.reduced(4), juce::Justification::centredLeft);

    g.setColour(Tokens::Colours::bgDark());
    g.fillRect(labelWidth, y, getWidth() - labelWidth, tempoTrackHeight);

    if (tempoEvents.size() > 1)
    {
        juce::Path tempoLine;
        float lineY = static_cast<float>(y + tempoTrackHeight / 2);

        for (int i = 0; i < static_cast<int>(tempoEvents.size()); ++i)
        {
            int tx = timeToX(tempoEvents[i].timeSeconds);
            tx = juce::jmax(tx, labelWidth);

            float dotY = lineY - static_cast<float>(tempoEvents[i].bpm - 60.0) * 0.3f;

            if (i == 0)
                tempoLine.startNewSubPath(static_cast<float>(tx), dotY);
            else
                tempoLine.lineTo(static_cast<float>(tx), dotY);
        }

        g.setColour(Tokens::Colours::accent());
        g.strokePath(tempoLine, juce::PathStrokeType(2.0f));

        for (int i = 0; i < static_cast<int>(tempoEvents.size()); ++i)
        {
            int tx = timeToX(tempoEvents[i].timeSeconds);
            tx = juce::jmax(tx, labelWidth);
            float dotY = lineY - static_cast<float>(tempoEvents[i].bpm - 60.0) * 0.3f;

            g.setColour(Tokens::Colours::accent());
            g.fillEllipse(static_cast<float>(tx) - 4.0f, dotY - 4.0f, 8.0f, 8.0f);

            g.setColour(Tokens::Colours::textPrimary());
            g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
            g.drawText(juce::String(tempoEvents[i].bpm, 0) + " BPM",
                       static_cast<int>(tx) + 8, static_cast<int>(dotY) - 8, 60, 16,
                       juce::Justification::centredLeft);
        }
    }
    else if (!tempoEvents.empty())
    {
        g.setColour(Tokens::Colours::textPrimary());
        g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
        g.drawText(juce::String(tempoEvents[0].bpm, 0) + " BPM",
                   labelWidth + 8, y + 2, 80, tempoTrackHeight - 4,
                   juce::Justification::centredLeft);
    }

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(y + tempoTrackHeight - 1, 0.0f, static_cast<float>(getWidth()));
    y += tempoTrackHeight;

    auto tsLabelArea = juce::Rectangle<int>(0, y, labelWidth, timeSigTrackHeight);
    g.setColour(Tokens::Colours::bgBase());
    g.fillRect(tsLabelArea);
    g.setColour(Tokens::Colours::textSecondary());
    g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    g.drawText("Time Sig", tsLabelArea.reduced(4), juce::Justification::centredLeft);

    g.setColour(Tokens::Colours::bgDark());
    g.fillRect(labelWidth, y, getWidth() - labelWidth, timeSigTrackHeight);

    for (int i = 0; i < static_cast<int>(timeSignatures.size()); ++i)
    {
        int tx = timeToX(timeSignatures[i].timeSeconds);
        if (tx < labelWidth || tx > getWidth()) continue;

        g.setColour(Tokens::Colours::accent());
        g.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeMedium))));
        g.drawText(juce::String(timeSignatures[i].numerator) + "/" +
                   juce::String(timeSignatures[i].denominator),
                   tx + 4, y + 2, 60, timeSigTrackHeight - 4,
                   juce::Justification::centredLeft);

        g.setColour(Tokens::Colours::accent().withAlpha(0.5f));
        g.drawVerticalLine(tx, static_cast<float>(y),
                           static_cast<float>(y + timeSigTrackHeight));
    }

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(y + timeSigTrackHeight - 1, 0.0f, static_cast<float>(getWidth()));

    double playPos = transport.getPositionInSeconds();
    int playX = timeToX(playPos);
    if (playX >= labelWidth && playX <= getWidth())
    {
        g.setColour(Tokens::Colours::playhead());
        g.drawVerticalLine(playX, 0.0f, static_cast<float>(getHeight()));
    }
}

void GlobalTracks::resized()
{
}

void GlobalTracks::mouseDoubleClick(const juce::MouseEvent& event)
{
    int mx = event.getPosition().x;
    int my = event.getPosition().y;
    double clickTime = xToTime(mx);

    if (clickTime < 0.0) clickTime = 0.0;

    if (my < markerTrackHeight)
    {
        static int markerCounter = 1;
        juce::Colour colours[] = {
            Tokens::Colours::play(), Tokens::Colours::record(),
            Tokens::Colours::accent(), Tokens::Colours::meterMid(),
            Tokens::Colours::loop(), Tokens::Colours::solo()
        };
        addMarker("Marker " + juce::String(markerCounter++), clickTime,
                  colours[markerCounter % 6]);
    }
    else if (my < markerTrackHeight + tempoTrackHeight)
    {
        addTempoEvent(clickTime, 120.0);
    }
    else if (my < markerTrackHeight + tempoTrackHeight + timeSigTrackHeight)
    {
        addTimeSignature(clickTime, 4, 4);
    }
}

void GlobalTracks::mouseDown(const juce::MouseEvent& event)
{
    int mx = event.getPosition().x;
    int my = event.getPosition().y;

    if (event.mods.isPopupMenu() && my < markerTrackHeight)
    {
        double clickTime = xToTime(mx);
        for (int i = 0; i < static_cast<int>(markers.size()); ++i)
        {
            int markerX = timeToX(markers[i].timeSeconds);
            if (std::abs(mx - markerX) < 10)
            {
                removeMarker(i);
                return;
            }
        }
    }
}

void GlobalTracks::timerCallback()
{
    repaint();
}

} // namespace harmonic_engine
