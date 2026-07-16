#include "harmonic_engine/GUI/EffectRack.h"

namespace harmonic_engine
{

EffectRack::EffectRack()
{
    addAndMakeVisible(addButton);
    addButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a90d9));
    addButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc0c0d0));
    addButton.onClick = [this] { showAddEffectMenu(); };

    addAndMakeVisible(viewport);
    viewport.setScrollOnDragEnabled(true);
}

EffectRack::~EffectRack() = default;

void EffectRack::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a2e));

    if (effectChain == nullptr || effectChain->getNumEffects() == 0)
    {
        g.setColour(juce::Colour(0xffc0c0d0).withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions(14.0f)));
        g.drawFittedText("No effects. Click 'Add Effect' to begin.",
                         getLocalBounds().reduced(10), juce::Justification::centred, 1);
    }
}

void EffectRack::resized()
{
    auto bounds = getLocalBounds();
    auto buttonArea = bounds.removeFromBottom(30);
    addButton.setBounds(buttonArea.reduced(4));

    viewport.setBounds(bounds);

    int totalHeight = 8;
    for (int i = 0; i < static_cast<int>(slotWidgetsList.size()); ++i)
    {
        auto* effect = effectChain->getEffect(i);
        totalHeight += calculateSlotHeight(effect) + 4;
    }
    totalHeight = juce::jmax(totalHeight, bounds.getHeight());

    auto* content = viewport.getViewedComponent();
    if (content == nullptr)
        return;

    content->setSize(bounds.getWidth(), totalHeight);

    int y = 4;
    for (int i = 0; i < static_cast<int>(slotWidgetsList.size()); ++i)
    {
        auto* effect = effectChain->getEffect(i);
        int slotH = calculateSlotHeight(effect);
        auto slotBounds = juce::Rectangle<int>(4, y, bounds.getWidth() - 8, slotH);

        auto& sw = slotWidgetsList[i];
        if (sw->nameLabel != nullptr)
            sw->nameLabel->setBounds(slotBounds.getX() + 4, slotBounds.getY() + 2,
                                     slotBounds.getWidth() - 100, 18);

        if (sw->bypassButton != nullptr)
            sw->bypassButton->setBounds(slotBounds.getRight() - 96, slotBounds.getY() + 2,
                                        56, 18);

        if (sw->removeButton != nullptr)
            sw->removeButton->setBounds(slotBounds.getRight() - 36, slotBounds.getY() + 2,
                                        32, 18);

        auto paramArea = slotBounds.withTrimmedTop(22).reduced(4, 2);
        int numParams = static_cast<int>(sw->sliders.size());
        if (numParams > 0)
        {
            int paramWidth = paramArea.getWidth() / numParams;
            for (int p = 0; p < numParams; ++p)
            {
                auto pBounds = paramArea.withX(paramArea.getX() + p * paramWidth)
                                        .withWidth(paramWidth);
                if (sw->sliderLabels[p] != nullptr)
                    sw->sliderLabels[p]->setBounds(pBounds.getX(), pBounds.getY(),
                                                   pBounds.getWidth(), 14);
                if (sw->sliders[p] != nullptr)
                    sw->sliders[p]->setBounds(pBounds.getX(), pBounds.getY() + 14,
                                              pBounds.getWidth(), pBounds.getHeight() - 14);
            }
        }

        y += slotH + 4;
    }
}

void EffectRack::setEffectChain(EffectChain* chain)
{
    effectChain = chain;
    rebuildSlots();
}

int EffectRack::calculateSlotHeight(EffectBase* effect) const
{
    if (effect == nullptr)
        return SlotHeight;

    int numParams = 0;
    if (dynamic_cast<ParametricEQ*>(effect))
        numParams = ParametricEQ::NumBands * 2;
    else if (dynamic_cast<DynamicsCompressor*>(effect))
        numParams = 4;
    else if (dynamic_cast<ReverbEffect*>(effect))
        numParams = 4;
    else if (dynamic_cast<DelayEffect*>(effect))
        numParams = 4;

    return HeaderHeight + 14 + numParams * SliderHeight / 2 + 8;
}

void EffectRack::rebuildSlots()
{
    slotWidgetsList.clear();

    if (effectChain == nullptr)
    {
        viewport.setViewedComponent(nullptr);
        resized();
        repaint();
        return;
    }

    auto* content = new juce::Component();
    viewport.setViewedComponent(content, true);

    int numEffects = effectChain->getNumEffects();

    for (int i = 0; i < numEffects; ++i)
    {
        auto* effect = effectChain->getEffect(i);
        if (effect == nullptr)
            continue;

        auto sw = std::make_unique<SlotWidgets>();

        sw->nameLabel = std::make_unique<juce::Label>("", effect->getName());
        sw->nameLabel->setColour(juce::Label::textColourId, juce::Colour(0xffc0c0d0));
        sw->nameLabel->setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        content->addAndMakeVisible(sw->nameLabel.get());

        sw->bypassButton = std::make_unique<juce::ToggleButton>("Bypass");
        sw->bypassButton->setColour(juce::ToggleButton::textColourId, juce::Colour(0xffc0c0d0));
        sw->bypassButton->setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff4a90d9));
        sw->bypassButton->setToggleState(effect->isBypassed(), juce::dontSendNotification);
        int idx = i;
        sw->bypassButton->onClick = [this, idx]
        {
            if (effectChain == nullptr) return;
            auto* fx = effectChain->getEffect(idx);
            if (fx != nullptr)
                fx->setBypassed(slotWidgetsList[idx]->bypassButton->getToggleState());
            viewport.getViewedComponent()->repaint();
        };
        content->addAndMakeVisible(sw->bypassButton.get());

        sw->removeButton = std::make_unique<juce::TextButton>("X");
        sw->removeButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe94560).withAlpha(0.6f));
        sw->removeButton->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc0c0d0));
        sw->removeButton->onClick = [this, idx] { removeEffectAt(idx); };
        content->addAndMakeVisible(sw->removeButton.get());

        auto createSlider = [&](const juce::String& name, float min, float max,
                                float interval, float value)
        {
            auto label = std::make_unique<juce::Label>("", name);
            label->setColour(juce::Label::textColourId, juce::Colour(0xffc0c0d0).withAlpha(0.7f));
            label->setFont(juce::Font(juce::FontOptions(10.0f)));

            auto slider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                          juce::Slider::NoTextBox);
            slider->setRange(min, max, interval);
            slider->setValue(value, juce::dontSendNotification);
            slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xff4a90d9));
            slider->setColour(juce::Slider::trackColourId, juce::Colour(0xff4a90d9).withAlpha(0.3f));
            slider->setColour(juce::Slider::backgroundColourId, juce::Colour(0xff222244));

            int si = static_cast<int>(sw->sliders.size());
            auto* swPtr = sw.get();
            slider->onValueChange = [this, idx, si, swPtr]()
            {
                if (effectChain == nullptr) return;
                auto* fx = effectChain->getEffect(idx);
                if (fx == nullptr) return;

                float val = static_cast<float>(swPtr->sliders[si]->getValue());

                if (auto* eq = dynamic_cast<ParametricEQ*>(fx))
                {
                    int band = si / 2;
                    if (si % 2 == 0)
                        eq->setBandFrequency(band, val);
                    else
                        eq->setBandGain(band, val);
                }
                else if (auto* comp = dynamic_cast<DynamicsCompressor*>(fx))
                {
                    switch (si)
                    {
                        case 0: comp->setThreshold(val); break;
                        case 1: comp->setRatio(val); break;
                        case 2: comp->setAttack(val); break;
                        case 3: comp->setRelease(val); break;
                        default: break;
                    }
                }
                else if (auto* rev = dynamic_cast<ReverbEffect*>(fx))
                {
                    switch (si)
                    {
                        case 0: rev->setRoomSize(val); break;
                        case 1: rev->setDamping(val); break;
                        case 2: rev->setWetLevel(val); break;
                        case 3: rev->setDryLevel(val); break;
                        default: break;
                    }
                }
                else if (auto* del = dynamic_cast<DelayEffect*>(fx))
                {
                    switch (si)
                    {
                        case 0: del->setDelayTime(val); break;
                        case 1: del->setFeedback(val); break;
                        case 2: del->setMix(val); break;
                        case 3: del->setLfoRate(val); break;
                        default: break;
                    }
                }
            };

            sw->sliderLabels.push_back(std::move(label));
            sw->sliders.push_back(std::move(slider));
        };

        if (auto* eq = dynamic_cast<ParametricEQ*>(effect))
        {
            for (int b = 0; b < ParametricEQ::NumBands; ++b)
            {
                createSlider("B" + juce::String(b + 1) + " Freq",
                             ParametricEQ::MinFrequency, ParametricEQ::MaxFrequency,
                             1.0f, eq->getBandFrequency(b));
                createSlider("B" + juce::String(b + 1) + " Gain",
                             ParametricEQ::MinGain, ParametricEQ::MaxGain,
                             0.1f, eq->getBandGain(b));
            }
        }
        else if (auto* comp = dynamic_cast<DynamicsCompressor*>(effect))
        {
            createSlider("Thresh", DynamicsCompressor::MinThreshold,
                         DynamicsCompressor::MaxThreshold, 0.1f, comp->getThreshold());
            createSlider("Ratio", DynamicsCompressor::MinRatio,
                         DynamicsCompressor::MaxRatio, 0.1f, comp->getRatio());
            createSlider("Atk", DynamicsCompressor::MinAttack,
                         DynamicsCompressor::MaxAttack, 0.1f, comp->getAttack());
            createSlider("Rel", DynamicsCompressor::MinRelease,
                         DynamicsCompressor::MaxRelease, 1.0f, comp->getRelease());
        }
        else if (auto* rev = dynamic_cast<ReverbEffect*>(effect))
        {
            createSlider("Room", ReverbEffect::MinRoomSize,
                         ReverbEffect::MaxRoomSize, 0.01f, rev->getRoomSize());
            createSlider("Damp", ReverbEffect::MinLevel,
                         ReverbEffect::MaxLevel, 0.01f, rev->getDamping());
            createSlider("Wet", ReverbEffect::MinLevel,
                         ReverbEffect::MaxLevel, 0.01f, rev->getWetLevel());
            createSlider("Dry", ReverbEffect::MinLevel,
                         ReverbEffect::MaxLevel, 0.01f, rev->getDryLevel());
        }
        else if (auto* del = dynamic_cast<DelayEffect*>(effect))
        {
            createSlider("Time", DelayEffect::MinDelayTime,
                         DelayEffect::MaxDelayTime, 0.001f, del->getDelayTime());
            createSlider("Fb", DelayEffect::MinFeedback,
                         DelayEffect::MaxFeedback, 0.01f, del->getFeedback());
            createSlider("Mix", DelayEffect::MinMix,
                         DelayEffect::MaxMix, 0.01f, del->getMix());
            createSlider("LFO", DelayEffect::MinLfoRate,
                         DelayEffect::MaxLfoRate, 0.1f, del->getLfoRate());
        }

        for (auto& sl : sw->sliders)
            content->addAndMakeVisible(sl.get());
        for (auto& lb : sw->sliderLabels)
            content->addAndMakeVisible(lb.get());

        slotWidgetsList.push_back(std::move(sw));
    }

    resized();
    viewport.repaint();
}

void EffectRack::showAddEffectMenu()
{
    juce::PopupMenu menu;

    menu.addItem(1, "Equalizer");
    menu.addItem(2, "Compressor");
    menu.addItem(3, "Reverb");
    menu.addItem(4, "Delay");

    menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
    {
        if (result == 0)
            return;

        EffectType type;
        switch (result)
        {
            case 1: type = EffectType::Equalizer; break;
            case 2: type = EffectType::Compressor; break;
            case 3: type = EffectType::Reverb; break;
            case 4: type = EffectType::Delay; break;
            default: return;
        }

        addEffectOfType(type);
    });
}

void EffectRack::addEffectOfType(EffectType type)
{
    if (effectChain == nullptr)
        return;

    effectChain->addEffect(type);
    rebuildSlots();
}

void EffectRack::removeEffectAt(int index)
{
    if (effectChain == nullptr)
        return;

    effectChain->removeEffect(index);
    rebuildSlots();
}

} // namespace harmonic_engine
