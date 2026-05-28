#pragma once
#include <QWidget>
#include <opencv2/opencv.hpp>

class HistogramWidget : public QWidget {
    Q_OBJECT

public:
    explicit HistogramWidget(QWidget* parent = nullptr);
    void setImage(const cv::Mat& image);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void computeHistograms(const cv::Mat& image);
    std::vector<float> m_histR; 
    std::vector<float> m_histG; 
    std::vector<float> m_histB;  
    bool m_isGray = false;       
};
