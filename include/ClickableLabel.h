#pragma once
#include <QLabel>
#include <QMouseEvent>
#include <functional>

class ClickableLabel : public QLabel {
public:
    explicit ClickableLabel(QWidget* parent = nullptr) : QLabel(parent) {
        setMouseTracking(true);
    }
    std::function<void(QPoint)> onClickHandler;
    std::function<void(QPoint)> onMoveHandler;
protected:
    void mousePressEvent(QMouseEvent* e) override {
        if (onClickHandler) onClickHandler(e->pos());
        QLabel::mousePressEvent(e);
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (onMoveHandler) onMoveHandler(e->pos());
        QLabel::mouseMoveEvent(e);
    }
};
