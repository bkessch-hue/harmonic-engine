#include "harmonic_engine/GUI/TransportBar.h"

namespace harmonic_engine
{

TransportBar::TransportBar(Transport& t)
    : transport(t)
{
    setupButton(rewindBtn,   "Rewind (,)");
    setupButton(ffwdBtn,     "Fast Forward (.)");
    setupButton(stopBtn,     "Stop (0)");
    setupButton(playBtn,     "Play (Space)");
    setupButton(recordBtn,      "Record (R)");
    setupButton(loopRecordBtn,  "Loop Record (Shift+R)");
    setupButton(loopBtn,        "Loop (L)");
    setupButton(metronomeBtn,   "Metronome (M)");

    recordBtn.setColour(juce::TextButton::buttonColourId, Tokens::Colours::record());
    loopRecordBtn.setColour(juce::TextButton::buttonColourId, Tokens::Colours::bgOverlay());
    loopBtn.setColour(juce::TextButton::buttonColourId, Tokens::Colours::bgOverlay());
    metronomeBtn.setColour(juce::TextButton::buttonColourId, Tokens::Colours::bgOverlay());

    // Tempo
    tempoLabel.setText("BPM", juce::dontSendNotification);
    tempoLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    tempoLabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    addAndMakeVisible(tempoLabel);

    tempoSlider.setRange(20.0, 300.0, 0.1);
    tempoSlider.setValue(120.0, juce::dontSendNotification);
    tempoSlider.setTextValueSuffix("");
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, Tokens::kButtonHeight);
    tempoSlider.onValueChange = [this]() {
        transport.setTempo(tempoSlider.getValue());
        if (onTempoChanged) onTempoChanged(tempoSlider.getValue());
    };
    tempoSlider.setTooltip("Adjust tempo in BPM");
    addAndMakeVisible(tempoSlider);

    // Time Signature
    timeSigLabel.setText("4/4", juce::dontSendNotification);
    timeSigLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeMedium))));
    timeSigLabel.setColour(juce::Label::textColourId, Tokens::Colours::textPrimary());
    timeSigLabel.setJustificationType(juce::Justification::centred);
    timeSigLabel.setBorderSize(juce::BorderSize<int>(2));
    addAndMakeVisible(timeSigLabel);

    // Position display
    positionLabel.setText("1 : 1 : 0", juce::dontSendNotification);
    positionLabel.setFont(juce::Font(juce::FontOptions(20.0f)));
    positionLabel.setColour(juce::Label::textColourId, Tokens::Colours::textPrimary());
    positionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(positionLabel);

    // Zoom
    zoomLabel.setText("Zoom", juce::dontSendNotification);
    zoomLabel.setFont(juce::Font(juce::FontOptions(static_cast<float>(Tokens::kFontSizeSmall))));
    zoomLabel.setColour(juce::Label::textColourId, Tokens::Colours::textSecondary());
    addAndMakeVisible(zoomLabel);

    zoomSlider.setRange(10.0, 500.0, 1.0);
    zoomSlider.setValue(100.0, juce::dontSendNotification);
    zoomSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    zoomSlider.onValueChange = [this]() {
        if (onZoomChanged) onZoomChanged(zoomSlider.getValue());
    };
    zoomSlider.setTooltip("Zoom timeline");
    addAndMakeVisible(zoomSlider);
}

TransportBar::~TransportBar()
{
}

void TransportBar::setupButton(juce::TextButton& btn, const juce::String& tip)
{
    btn.setTooltip(tip);
    btn.addListener(this);
    addAndMakeVisible(btn);
}

void TransportBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(Tokens::Colours::bgDark());
    g.fillRect(bounds);

    g.setColour(Tokens::Colours::borderDefault());
    g.drawHorizontalLine(0, 0.0f, bounds.getWidth());
    g.drawHorizontalLine(getHeight() - 1, 0.0f, bounds.getWidth());

    // Draw section separators
    float sepAlpha = 0.3f;
    g.setColour(Tokens::Colours::borderSubtle().withAlpha(sepAlpha));
    int sepX = 440;
    g.drawVerticalLine(sepX, 0.0f, static_cast<float>(getHeight()));
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(Tokens::kSpace8, Tokens::kSpace4);

    const int btnH = bounds.getHeight() - Tokens::kSpace4;
    const int smBtnW = 36;
    const int mdBtnW = 52;
    const int gap = Tokens::kSpace2;
    int x = bounds.getX();
    int y = bounds.getY() + Tokens::kSpace2;

    // ── Transport buttons ───────────────────────────────────
    auto placeBtn = [&](juce::TextButton& btn, int w)
    {
        btn.setBounds(x, y, w, btnH);
        x += w + gap;
    };

    placeBtn(rewindBtn,   smBtnW);
    placeBtn(ffwdBtn,     smBtnW);
    placeBtn(stopBtn,     mdBtnW);
    placeBtn(playBtn,     mdBtnW);
    placeBtn(recordBtn,      mdBtnW);
    placeBtn(loopRecordBtn,  mdBtnW);
    placeBtn(loopBtn,        mdBtnW);
    placeBtn(metronomeBtn,   mdBtnW);

    x += Tokens::kSpace12;

    // ── Tempo ───────────────────────────────────────────────
    tempoLabel.setBounds(x, y, 30, btnH);
    x += 32;
    tempoSlider.setBounds(x, y, 90, btnH);
    x += 94;

    // ── Time Signature ──────────────────────────────────────
    timeSigLabel.setBounds(x, y, 40, btnH);
    x += 48;

    // ── Position display (fills remaining centre space) ─────
    int rightEdge = bounds.getRight() - Tokens::kSpace8;
    int zoomAreaW = 130;
    int posW = rightEdge - x - zoomAreaW - Tokens::kSpace16;
    if (posW > 80)
        positionLabel.setBounds(x, y, posW, btnH);

    // ── Zoom (right-aligned) ────────────────────────────────
    int zx = rightEdge - zoomAreaW;
    zoomLabel.setBounds(zx, y, 32, btnH);
    zx += 34;
    zoomSlider.setBounds(zx, y, zoomAreaW - 34, btnH);
}

void TransportBar::buttonClicked(juce::Button* button)
{
    if (button == &rewindBtn && onRewind)        onRewind();
    else if (button == &ffwdBtn && onFastForward) onFastForward();
    else if (button == &stopBtn && onStop)        onStop();
    else if (button == &playBtn && onPlay)        onPlay();
    else if (button == &recordBtn && onRecord)          onRecord();
    else if (button == &loopRecordBtn && onToggleLoopRecord) onToggleLoopRecord();
    else if (button == &loopBtn && onToggleLoop)        onToggleLoop();
    else if (button == &metronomeBtn && onToggleMetronome) onToggleMetronome();
}

void TransportBar::updateDisplay()
{
    updatePositionDisplay();

    // Sync button active states
    playBtn.setColour(juce::TextButton::buttonColourId,
        transport.isPlaying() ? Tokens::Colours::play() : Tokens::Colours::bgOverlay());
    recordBtn.setColour(juce::TextButton::buttonColourId,
        transport.isRecording() ? Tokens::Colours::record() : Tokens::Colours::bgOverlay());
    loopRecordBtn.setColour(juce::TextButton::buttonColourId,
        loopRecordBtn.getToggleState() ? Tokens::Colours::record().withRotatedHue(0.1f) : Tokens::Colours::bgOverlay());
    loopBtn.setColour(juce::TextButton::buttonColourId,
        loopBtn.getToggleState() ? Tokens::Colours::loop() : Tokens::Colours::bgOverlay());

    // Sync tempo display
    double currentTempo = transport.getTempo();
    if (std::abs(tempoSlider.getValue() - currentTempo) > 0.05)
        tempoSlider.setValue(currentTempo, juce::dontSendNotification);
}

void TransportBar::updatePositionDisplay()
{
    const double seconds = transport.getPositionInSeconds();
    const double tempo = transport.getTempo();
    const double beatDuration = 60.0 / tempo;

    // Calculate bars:beats:ticks
    int totalBeats = static_cast<int>(seconds / beatDuration);
    int ticks = static_cast<int>(((seconds / beatDuration) - totalBeats) * 960.0);
    int beatsPerBar = 4;
    int bar = (totalBeats / beatsPerBar) + 1;
    int beat = (totalBeats % beatsPerBar) + 1;

    juce::String posString = juce::String(bar) + " : " +
                             juce::String(beat) + " : " +
                             juce::String(ticks);

    positionLabel.setText(posString, juce::dontSendNotification);
}

void TransportBar::setPlaying(bool playing)
{
    playBtn.setColour(juce::TextButton::buttonColourId,
        playing ? Tokens::Colours::play() : Tokens::Colours::bgOverlay());
}

void TransportBar::setRecording(bool recording)
{
    recordBtn.setColour(juce::TextButton::buttonColourId,
        recording ? Tokens::Colours::record() : Tokens::Colours::bgOverlay());
}

void TransportBar::setLoopRecording(bool loopRecording)
{
    loopRecordBtn.setToggleState(loopRecording, juce::dontSendNotification);
    loopRecordBtn.setColour(juce::TextButton::buttonColourId,
        loopRecording ? Tokens::Colours::record().withRotatedHue(0.1f) : Tokens::Colours::bgOverlay());
}

void TransportBar::setLooping(bool looping)
{
    loopBtn.setToggleState(looping, juce::dontSendNotification);
    loopBtn.setColour(juce::TextButton::buttonColourId,
        looping ? Tokens::Colours::loop() : Tokens::Colours::bgOverlay());
}

void TransportBar::setMetronomeOn(bool on)
{
    metronomeBtn.setColour(juce::TextButton::buttonColourId,
        on ? Tokens::Colours::accent() : Tokens::Colours::bgOverlay());
}

void TransportBar::setZoomRange(double minPPS, double maxPPS)
{
    zoomSlider.setRange(minPPS, maxPPS, 1.0);
}

void TransportBar::setZoomValue(double pps)
{
    zoomSlider.setValue(pps, juce::dontSendNotification);
}

double TransportBar::getZoomValue() const
{
    return zoomSlider.getValue();
}

} // namespace harmonic_engine
