#include "harmonic_engine/MainWindow.h"
#include "harmonic_engine/GUI/LookAndFeel.h"

namespace harmonic_engine
{

MainWindow::MainWindow(juce::String name)
    : DocumentWindow(name,
                     juce::Colour(0xff1a1a2e),
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);

    editor = std::make_unique<MainEditor>(audioEngine);
    setContentOwned(editor.release(), true);

    centreWithSize(1200, 800);
    setVisible(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

Engine& MainWindow::getEngine()
{
    return audioEngine;
}

} // namespace harmonic_engine
