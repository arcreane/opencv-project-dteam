#include "HistogramWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>

HistogramWidget::HistogramWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(256, 120);
    setMaximumHeight(150);
    m_histR.resize(256, 0.f);
    m_histG.resize(256, 0.f);
    m_histB.resize(256, 0.f);
}
void HistogramWidget::setImage(const cv::Mat& image) {
    if (image.empty()) return;
    computeHistograms(image);
    update();  // demande un repaint Qt
}
void HistogramWidget::computeHistograms(const cv::Mat& image) {
    m_isGray = (image.channels() == 1);
    int histSize = 256;
    float range[]  = {0, 256};
    const float* histRange = {range};
    if (m_isGray) {
        cv::Mat histMat;
        cv::calcHist(&image, 1, nullptr, cv::Mat(), histMat, 1, &histSize, &histRange);
        cv::normalize(histMat, histMat, 0, 1, cv::NORM_MINMAX);
        m_histR.resize(256);
        for (int i = 0; i < 256; ++i)
            m_histR[i] = histMat.at<float>(i);
    } else {
        std::vector<cv::Mat> planes;
        cv::split(image, planes);
        auto calcNorm = [&](const cv::Mat& plane, std::vector<float>& out) {
            cv::Mat h;
            cv::calcHist(&plane, 1, nullptr, cv::Mat(), h, 1, &histSize, &histRange);
            cv::normalize(h, h, 0, 1, cv::NORM_MINMAX);
            out.resize(256);
            for (int i = 0; i < 256; ++i)
                out[i] = h.at<float>(i);
        };
        calcNorm(planes[2], m_histR);  
        calcNorm(planes[1], m_histG);
        calcNorm(planes[0], m_histB);
    }
}
void HistogramWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    int w = width();
    int h = height() - 2;
    auto drawChannel = [&](const std::vector<float>& hist, QColor color) {
        if (hist.empty()) return;
        color.setAlpha(160);
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        QPainterPath path;
        path.moveTo(0, h);
        for (int i = 0; i < 256; ++i) {
            double x  = static_cast<double>(i) / 255.0 * w;
            double yv = h - hist[i] * h;
            if (i == 0) path.moveTo(x, yv);
            else        path.lineTo(x, yv);
        }
        path.lineTo(w, h);
        path.closeSubpath();
        p.drawPath(path);
    };
    if (m_isGray) {
        drawChannel(m_histR, QColor(200, 200, 200));
    } else {
        drawChannel(m_histB, QColor(0,   80, 255));
        drawChannel(m_histG, QColor(0,  200,  80));
        drawChannel(m_histR, QColor(255, 60,  60));
    }
    p.setPen(QColor(60, 60, 60));
    for (int i = 1; i < 4; ++i)
        p.drawLine(w * i / 4, 0, w * i / 4, h);
}
