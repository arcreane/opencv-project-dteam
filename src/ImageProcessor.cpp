#include "ImageProcessor.h"
#include <opencv2/photo.hpp>  
#include <opencv2/features2d.hpp> 

cv::Mat ImageProcessor::threshold(const cv::Mat& src, int mode, int thresh, int blockSize) {
    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src.clone();
    cv::Mat result;
    switch (mode) {
        case 0:
            cv::threshold(gray, result, thresh, 255, cv::THRESH_BINARY);
            break;
        case 1: 
            cv::threshold(gray, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            break;
        case 2: 
            if (blockSize % 2 == 0) blockSize++;  // doit être impair
            cv::adaptiveThreshold(gray, result, 255,
                                  cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                  cv::THRESH_BINARY, blockSize, 2);
            break;
    }
    return result;
}
cv::Mat ImageProcessor::equalizeHistogram(const cv::Mat& src, bool useCLAHE,
                                           double clipLimit, int tileSize) {
    cv::Mat result;
    if (src.channels() == 1) {
        if (useCLAHE) {
            auto clahe = cv::createCLAHE(clipLimit, cv::Size(tileSize, tileSize));
            clahe->apply(src, result);
        } else {
            cv::equalizeHist(src, result);
        }
    } else {
        cv::Mat ycrcb;
        cv::cvtColor(src, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> channels;
        cv::split(ycrcb, channels);
        if (useCLAHE) {
            auto clahe = cv::createCLAHE(clipLimit, cv::Size(tileSize, tileSize));
            clahe->apply(channels[0], channels[0]);
        } else {
            cv::equalizeHist(channels[0], channels[0]);
        }
        cv::merge(channels, ycrcb);
        cv::cvtColor(ycrcb, result, cv::COLOR_YCrCb2BGR);
    }
    return result;
}
cv::Mat ImageProcessor::morphology(const cv::Mat& src, int op, int kernelSize, int kernelShape) {
    cv::Mat kernel = cv::getStructuringElement(
        kernelShape,
        cv::Size(kernelSize, kernelSize)
    );
    cv::Mat result;
    int morphOp = cv::MORPH_DILATE;
    switch (op) {
        case 0: morphOp = cv::MORPH_DILATE;   break; 
        case 1: morphOp = cv::MORPH_ERODE;    break; 
        case 2: morphOp = cv::MORPH_OPEN;     break; 
        case 3: morphOp = cv::MORPH_CLOSE;    break; 
        case 4: morphOp = cv::MORPH_GRADIENT; break; 
    }
    cv::morphologyEx(src, result, morphOp, kernel);
    return result;
}
cv::Mat ImageProcessor::canny(const cv::Mat& src, int low, int high, int aperture) {
    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src.clone();
    cv::Mat edges;
    cv::Canny(gray, edges, low, high, aperture);
    return edges;
}
cv::Mat ImageProcessor::geometricTransform(const cv::Mat& src, int mode,
                                            const std::vector<cv::Point2f>& srcPts,
                                            const std::vector<cv::Point2f>& dstPts) {
    cv::Mat result;
    if (mode == 0 && srcPts.size() >= 3 && dstPts.size() >= 3) {
        cv::Mat M = cv::getAffineTransform(
            std::vector<cv::Point2f>(srcPts.begin(), srcPts.begin()+3),
            std::vector<cv::Point2f>(dstPts.begin(), dstPts.begin()+3)
        );
        cv::warpAffine(src, result, M, src.size());
    } else if (mode == 1 && srcPts.size() >= 4 && dstPts.size() >= 4) {
        cv::Mat M = cv::getPerspectiveTransform(
            std::vector<cv::Point2f>(srcPts.begin(), srcPts.begin()+4),
            std::vector<cv::Point2f>(dstPts.begin(), dstPts.begin()+4)
        );
        cv::warpPerspective(src, result, M, src.size());
    } else {
        result = src.clone();
    }
    return result;
}
cv::Mat ImageProcessor::cartoonEffect(const cv::Mat& src) {
    cv::Mat color = src.clone();
    for (int i = 0; i < 6; ++i)
        cv::bilateralFilter(color.clone(), color, 9, 9, 7);
    cv::Mat gray, edges;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 7);   
    cv::adaptiveThreshold(gray, edges, 255,
                          cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY, 9, 2);
    cv::Mat edgesColor;
    cv::cvtColor(edges, edgesColor, cv::COLOR_GRAY2BGR);
    cv::Mat result;
    cv::bitwise_and(color, edgesColor, result);   
    return result;
}
cv::Mat ImageProcessor::pencilSketch(const cv::Mat& src, float sigmaS, float sigmaR, float shade) {
    cv::Mat gray_sketch, color_sketch;
    cv::pencilSketch(src, gray_sketch, color_sketch, sigmaS, sigmaR, shade);
    return gray_sketch; 
}
cv::Mat ImageProcessor::bilateralFilter(const cv::Mat& src, int d,
                                         double sigmaColor, double sigmaSpace) {
    cv::Mat result;
    cv::bilateralFilter(src, result, d, sigmaColor, sigmaSpace);
    return result;
}
cv::Mat ImageProcessor::detectORB(const cv::Mat& src, int maxFeatures) {
    cv::Ptr<cv::ORB> orb = cv::ORB::create(maxFeatures);
    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src.clone();
    std::vector<cv::KeyPoint> keypoints;
    orb->detect(gray, keypoints);
    cv::Mat result = src.clone();
    cv::drawKeypoints(result, keypoints, result,
                      cv::Scalar(0, 255, 0),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    return result;
}
cv::Mat ImageProcessor::gammaCorrection(const cv::Mat& src, double gamma) {
    // Construit une LUT (Look-Up Table) de 256 valeurs : f(i) = 255 × (i/255)^gamma
    // Appliquer une LUT est O(n) en taille d'image, très rapide.
    cv::Mat lut(1, 256, CV_8U);
    for (int i = 0; i < 256; ++i) {
        double normalized = i / 255.0;
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(
            std::pow(normalized, gamma) * 255.0
        );
    }
    cv::Mat result;
    cv::LUT(src, lut, result);
    return result;
}
cv::Mat ImageProcessor::vignetteGrain(const cv::Mat& src, double strength, int grainAmount) {
    cv::Mat result = src.clone();
    int rows = src.rows, cols = src.cols;
    cv::Mat kernelX = cv::getGaussianKernel(cols, cols * 0.45);
    cv::Mat kernelY = cv::getGaussianKernel(rows, rows * 0.45);
    cv::Mat kernel  = kernelY * kernelX.t(); 
    double minVal, maxVal;
    cv::minMaxLoc(kernel, &minVal, &maxVal);
    kernel = (kernel - minVal) / (maxVal - minVal);
    cv::Mat mask;
    kernel.convertTo(mask, CV_32F);
    mask = 1.0 - strength * (1.0 - mask);
    std::vector<cv::Mat> channels;
    result.convertTo(result, CV_32F);
    cv::split(result, channels);
    for (auto& ch : channels)
        cv::multiply(ch, mask, ch);
    cv::merge(channels, result);
    result.convertTo(result, CV_8U);
    if (grainAmount > 0) {
        cv::Mat noise(src.size(), src.type());
        cv::randn(noise, 0, grainAmount);
        result = result + noise;
    }
    return result;
}
cv::Mat ImageProcessor::floodFillTool(const cv::Mat& src, cv::Point seed,
                                       cv::Scalar color, int tolerance) {
    cv::Mat result = src.clone();
    cv::floodFill(result, seed, color,
                  nullptr,                            
                  cv::Scalar(tolerance, tolerance, tolerance),  
                  cv::Scalar(tolerance, tolerance, tolerance),  
                  cv::FLOODFILL_FIXED_RANGE);
    return result;
}
cv::Mat ImageProcessor::toDisplayFormat(const cv::Mat& src) {
    if (src.channels() == 3) {
        cv::Mat rgb;
        cv::cvtColor(src, rgb, cv::COLOR_BGR2RGB);
        return rgb;
    }
    return src;
}
cv::Mat ImageProcessor::fitToSize(const cv::Mat& src, int maxW, int maxH) {
    if (src.empty()) return src;
    double scaleW = static_cast<double>(maxW) / src.cols;
    double scaleH = static_cast<double>(maxH) / src.rows;
    double scale  = std::min({scaleW, scaleH, 1.0});  // ne jamais agrandir
    cv::Mat result;
    cv::resize(src, result, cv::Size(), scale, scale, cv::INTER_AREA);
    return result;
}
