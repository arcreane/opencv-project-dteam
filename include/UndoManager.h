#pragma once
#include <opencv2/opencv.hpp>
#include <deque>
#include <string>

class UndoManager {
public:
    explicit UndoManager(int maxStates = 20);
    void push(const cv::Mat& state, const std::string& description = "");
    bool undo(cv::Mat& outState);
    bool redo(cv::Mat& outState)
    bool canUndo() const;
    bool canRedo() const;
    void clear();
    std::string lastDescription() const;

private:
    struct Entry {
        cv::Mat image;
        std::string description;
    };
    std::deque<Entry> m_undoStack;
    std::deque<Entry> m_redoStack;
    int m_maxStates;
};
