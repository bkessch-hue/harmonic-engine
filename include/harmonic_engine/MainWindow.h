#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "harmonic_engine/AudioEngine/Engine.h"
#include "harmonic_engine/GUI/MainEditor.h"

namespace harmonic_engine
{

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name);
    ~MainWindow() override;

    void closeButtonPressed() override;

    Engine& getEngine();

private:
    Engine audioEngine;
    std::unique_ptr<MainEditor> editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace harmonic_engine
