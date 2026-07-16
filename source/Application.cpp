#include "harmonic_engine/Application.h"
#include "harmonic_engine/MainWindow.h"

namespace harmonic_engine
{

HarmonicEngineApplication::HarmonicEngineApplication()
{
}

HarmonicEngineApplication& HarmonicEngineApplication::getApp()
{
    return *dynamic_cast<HarmonicEngineApplication*>(JUCEApplication::getInstance());
}

const juce::String HarmonicEngineApplication::getApplicationName()
{
    return "Harmonic Engine";
}

const juce::String HarmonicEngineApplication::getApplicationVersion()
{
    return "0.1.0";
}

bool HarmonicEngineApplication::moreThanOneInstanceAllowed()
{
    return true;
}

void HarmonicEngineApplication::initialise(const juce::String& commandLine)
{
    ignoreUnused(commandLine);

    juce::Desktop::getInstance().setScreenSaverEnabled(false);

    new MainWindow(getApplicationName());
}

void HarmonicEngineApplication::shutdown()
{
}

void HarmonicEngineApplication::systemRequestedQuit()
{
    quit();
}

void HarmonicEngineApplication::anotherInstanceStarted(const juce::String& commandLine)
{
    ignoreUnused(commandLine);
}

} // namespace harmonic_engine

juce::JUCEApplicationBase* createJuceApplication()
{
    return new harmonic_engine::HarmonicEngineApplication();
}
