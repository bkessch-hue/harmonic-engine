#include "harmonic_engine/State/UndoManager.h"

namespace harmonic_engine
{

UndoManager::UndoManager()
    : juceUndoManager(100, 2000)
{
}

UndoManager::~UndoManager()
{
}

void UndoManager::perform(juce::UndoableAction* action, const juce::String& description)
{
    juceUndoManager.perform(action, description);
}

bool UndoManager::canUndo() const
{
    return juceUndoManager.canUndo();
}

bool UndoManager::canRedo() const
{
    return juceUndoManager.canRedo();
}

void UndoManager::undo()
{
    juceUndoManager.undo();
}

void UndoManager::redo()
{
    juceUndoManager.redo();
}

void UndoManager::clearHistory()
{
    juceUndoManager.clearUndoHistory();
}

int UndoManager::getNumActionsInHistory() const
{
    return juceUndoManager.getNumActionsInCurrentTransaction();
}

} // namespace harmonic_engine
