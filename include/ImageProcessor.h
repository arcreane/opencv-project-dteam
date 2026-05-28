#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class ImageProcessor {
public:
    static cv::Mat threshold(const cv::Mat& src, int mode, int thresh = 128, int blockSize = 11);
    static cv::Mat equalizeHistogram(const cv::Mat& src, bool useCLAHE = false,
                                     double clipLimit = 2.0, int tileSize = 8);
    static cv::Mat morphology(const cv::Mat& src, int op, int kernelSize = 3,
                              int kernelShape = cv::MORPH_RECT);
    static cv::Mat canny(const cv::Mat& src, int low = 50, int high = 150, int aperture = 3);
    static cv::Mat geometricTransform(const cv::Mat& src, int mode,
                                      const std::vector<cv::Point2f>& srcPts,
                                      const std::vector<cv::Point2f>& dstPts);
    static cv::Mat cartoonEffect(const cv::Mat& src);
    static cv::Mat pencilSketch(const cv::Mat& src, float sigmaS = 60.f,
                                float sigmaR = 0.07f, float shade = 0.05f);
    static cv::Mat bilateralFilter(const cv::Mat& src, int d = 9,
                                   double sigmaColor = 75.0, double sigmaSpace = 75.0);
    static cv::Mat detectORB(const cv::Mat& src, int maxFeatures = 500);
    static cv::Mat gammaCorrection(const cv::Mat& src, double gamma = 1.0);
    static cv::Mat vignetteGrain(const cv::Mat& src, double strength = 0.5, int grainAmount = 10);
    static cv::Mat floodFillTool(const cv::Mat& src, cv::Point seed,
                                 cv::Scalar color, int tolerance = 20);
    static cv::Mat toDisplayFormat(const cv::Mat& src);
    static cv::Mat fitToSize(const cv::Mat& src, int maxW, int maxH);
};
