#include "UndoManager.h"

UndoManager::UndoManager(int maxStates)
    : m_maxStates(maxStates)
{}
void UndoManager::push(const cv::Mat& state, const std::string& description) {
    m_redoStack.clear();
    Entry entry;
    entry.image       = state.clone();
    entry.description = description;
    m_undoStack.push_back(std::move(entry));
    while (static_cast<int>(m_undoStack.size()) > m_maxStates) {
        m_undoStack.pop_front();
    }
}
bool UndoManager::undo(cv::Mat& outState) {
    if (m_undoStack.empty()) return false;
    m_redoStack.push_back(m_undoStack.back());
    m_undoStack.pop_back();
    if (m_undoStack.empty()) {
        outState = m_redoStack.back().image.clone();
        m_undoStack.push_back(m_redoStack.back());
        m_redoStack.pop_back();
        return false;  
    }
    outState = m_undoStack.back().image.clone();
    return true;
}
bool UndoManager::redo(cv::Mat& outState) {
    if (m_redoStack.empty()) return false;
    Entry entry = m_redoStack.back();
    m_redoStack.pop_back();
    m_undoStack.push_back(entry);
    outState = entry.image.clone();
    return true;
}
bool UndoManager::canUndo() const {
    return m_undoStack.size() > 1;
}
bool UndoManager::canRedo() const {
    return !m_redoStack.empty();
}
void UndoManager::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}
std::string UndoManager::lastDescription() const {
    if (!m_undoStack.empty())
        return m_undoStack.back().description;
    return "";
}
