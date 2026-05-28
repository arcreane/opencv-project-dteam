#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <vector>
#include <string>

class PanoramaProcessor {
public:
    PanoramaProcessor();
    bool stitch(const std::vector<cv::Mat>& images, cv::Mat& result);
    std::string lastError() const { return m_lastError; }
    void setMode(cv::Stitcher::Mode mode);

private:
    cv::Ptr<cv::Stitcher> m_stitcher;
    std::string m_lastError;
    cv::Stitcher::Mode m_mode = cv::Stitcher::PANORAMA;
};
