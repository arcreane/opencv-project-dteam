#include "PanoramaProcessor.h"

PanoramaProcessor::PanoramaProcessor() {
    m_stitcher = cv::Stitcher::create(m_mode);
}
void PanoramaProcessor::setMode(cv::Stitcher::Mode mode) {
    m_mode    = mode;
    m_stitcher = cv::Stitcher::create(m_mode);
}
bool PanoramaProcessor::stitch(const std::vector<cv::Mat>& images, cv::Mat& result) {
    if (images.size() < 2) {
        m_lastError = "Au moins 2 images sont nécessaires pour créer un panorama.";
        return false;
    }
    cv::Stitcher::Status status = m_stitcher->stitch(images, result);
    switch (status) {
        case cv::Stitcher::OK:
            m_lastError = "";
            return true;
        case cv::Stitcher::ERR_NEED_MORE_IMGS:
            m_lastError = "Pas assez d'images ou pas assez de chevauchement.";
            break;
        case cv::Stitcher::ERR_HOMOGRAPHY_EST_FAIL:
            m_lastError = "Estimation de l'homographie échouée — trop peu de points communs.";
            break;
        case cv::Stitcher::ERR_CAMERA_PARAMS_ADJUST_FAIL:
            m_lastError = "Ajustement des paramètres caméra échoué.";
            break;
        default:
            m_lastError = "Erreur de stitching inconnue (code " + std::to_string(status) + ").";
    }
    return false;
}
