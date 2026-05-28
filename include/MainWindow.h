#pragma once
#include <QMainWindow>
#include <QLabel>
#include "ClickableLabel.h"
#include <QScrollArea>
#include <QSlider>
#include <QComboBox>
#include <QSpinBox>
#include <QDockWidget>
#include <QListWidget>
#include <QStatusBar>
#include <opencv2/opencv.hpp>
#include "ImageProcessor.h"
#include "HistogramWidget.h"
#include "UndoManager.h"
#include "PanoramaProcessor.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    void openImage();
    void saveImage();
    void saveImageAs();
    void undo();
    void redo();
    void applyThreshold();
    void applyHistogramEq();
    void applyMorphology();
    void applyCanny();
    void applyGeometric();
    void openPanoramaDialog();
    void applyCartoon();
    void applyPencilSketch();
    void applyBilateral();
    void applyORBDetection();
    void applyGammaCorrection();
    void applyVignetteGrain();
    void activateFloodFill(); 
    void updateDisplay();          
    void onImageClicked(QPoint pos);
    void updateStatusBar(QPoint pos = {-1,-1});
    void zoomIn();
    void zoomOut();
    void fitToWindow();

private:
    void setupMenuBar();
    void setupToolBar();
    void setupCentralWidget();
    void setupDockPanels();
    void pushUndo(const std::string& desc); 
    void setCurrentImage(const cv::Mat& img, const std::string& undoDesc);
    cv::Mat getPreviewImage() const; 
    cv::Mat       m_original;    
    cv::Mat       m_current;     
    std::string   m_filePath;    
    UndoManager   m_undo;
    PanoramaProcessor m_pano;
    QLabel*       m_imageLabel   = nullptr;
    QScrollArea*  m_scrollArea   = nullptr;
    double        m_zoomFactor   = 1.0;
    QDockWidget*  m_paramDock    = nullptr;
    QComboBox*    m_threshMode   = nullptr;
    QSlider*      m_threshSlider = nullptr;
    QComboBox*    m_morphOp      = nullptr;
    QSlider*      m_morphKernel  = nullptr;
    QSlider*      m_cannyLow     = nullptr;
    QSlider*      m_cannyHigh    = nullptr;
    QSlider*      m_gammaSlider  = nullptr;
    HistogramWidget* m_histogram = nullptr;
    bool m_floodFillMode = false; 
    QLabel* m_modeLabel  = nullptr;
};
