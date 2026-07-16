#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace harmonic_engine
{

class UndoManager
{
public:
    UndoManager();
    ~UndoManager();

    void perform(juce::UndoableAction* action, const juce::String& description);

    bool canUndo() const;
    bool canRedo() const;

    void undo();
    void redo();

    void clearHistory();

    int getNumActionsInHistory() const;

private:
    juce::UndoManager juceUndoManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UndoManager)
};

} // namespace harmonic_engine
