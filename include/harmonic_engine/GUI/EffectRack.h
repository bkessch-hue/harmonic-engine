#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/EffectBase.h"
#include "harmonic_engine/AudioEngine/AudioEffects.h"
#include "harmonic_engine/GUI/LookAndFeel.h"

namespace harmonic_engine
{

class EffectRack : public juce::Component
{
public:
    static constexpr int SlotHeight = 80;
    static constexpr int HeaderHeight = 24;
    static constexpr int SliderHeight = 22;
    static constexpr int Padding = 6;

    EffectRack();
    ~EffectRack() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setEffectChain(EffectChain* chain);
    EffectChain* getEffectChain() const { return effectChain; }

private:
    void rebuildSlots();
    void showAddEffectMenu();
    void addEffectOfType(EffectType type);
    void removeEffectAt(int index);
    int calculateSlotHeight(EffectBase* effect) const;

    EffectChain* effectChain = nullptr;

    struct SlotWidgets
    {
        std::unique_ptr<juce::Label> nameLabel;
        std::unique_ptr<juce::ToggleButton> bypassButton;
        std::unique_ptr<juce::TextButton> removeButton;
        std::vector<std::unique_ptr<juce::Slider>> sliders;
        std::vector<std::unique_ptr<juce::Label>> sliderLabels;
    };

    std::vector<std::unique_ptr<SlotWidgets>> slotWidgetsList;

    juce::TextButton addButton { "Add Effect" };
    juce::Viewport viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectRack)
};

} // namespace harmonic_engine
